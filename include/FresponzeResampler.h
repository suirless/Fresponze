/*********************************************************************
* Copyright (C) Anton Kovalev (vertver), 2019. All rights reserved.
* Fresponze - fast, simple and modern multimedia sound library
* Apache-2 License
**********************************************************************
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************/
#pragma once
#include "FresponzeTypes.h"
#include "CDSPResampler.h"

class IBaseResampler
{
public:
	virtual void Initialize(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) = 0;
	virtual void Destroy() = 0;
	virtual void Reset(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) = 0;
	virtual void Resample(fr_i32 frames, fr_f32** inputData, fr_f32** outputData) = 0;
	virtual void ResampleDouble(fr_i32 frames, fr_f64** inputData, fr_f64** outputData) = 0;
};

#ifdef USE_LIBSOXR_RESAMPLER

#endif

class CR8BrainResampler : public IBaseResampler
{
private:
	bool lin = false;
	fr_i32 bufLength = 0;
	fr_i32 inSRate = 0;
	fr_i32 outSRate = 0;
	fr_i32 channels = 0;
	C2DDoubleBuffer doubleBuffers[2] = {};
	r8b::CDSPResampler* resampler[MAX_CHANNELS] = {};

public:
	~CR8BrainResampler()
	{
		Destroy();
		doubleBuffers[0].Free();
		doubleBuffers[1].Free();
	}

	void Initialize(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) override
	{
		bufLength = MaxBufferIn;
		inSRate = InputSampleRate;
		outSRate = OutputSampleRate;
		channels = ChannelsCount;
		for (size_t i = 0; i < ChannelsCount; i++) {
			resampler[i] = new r8b::CDSPResampler(inSRate, outSRate, bufLength, 2.0, isLinear ? 136.45 : 109.56, isLinear ? r8b::fprLinearPhase : r8b::fprMinPhase);
		}	
	}

	void Destroy() override
	{
		size_t index = 0;
		for (size_t i = 0; i < MAX_CHANNELS; i++) {
			if (resampler[i]) delete resampler[i];
		}
	}

	void Reset(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear)  override
	{
		Destroy();
		Initialize(MaxBufferIn, InputSampleRate, OutputSampleRate, ChannelsCount, isLinear);
	}
	
	void ResampleDouble(fr_i32 frames, fr_f64** inputData, fr_f64** outputData)  override
	{
		if (frames > bufLength) Reset(frames, inSRate, outSRate, channels, lin);
		for (size_t i = 0; i < channels; i++) {
			resampler[i]->process(inputData[i], frames, outputData[i]);
		}
	}

	void Resample(fr_i32 frames, fr_f32** inputData, fr_f32** outputData)  override
	{
		fr_i32 convertedFrames = 0;
		CalculateFrames(frames, inSRate, outSRate, convertedFrames);
		if (frames > bufLength) Reset(frames, inSRate, outSRate, channels, lin);

		doubleBuffers[0].Resize(channels, frames);
		doubleBuffers[1].Resize(channels, frames);
		FloatToDouble(inputData, doubleBuffers[0].GetBuffers(), channels, frames);
		for (size_t i = 0; i < channels; i++) {
			resampler[i]->process(doubleBuffers[0][i], frames, doubleBuffers[1][i]);
		}

		DoubleToFloat(outputData, doubleBuffers[1].GetBuffers(), channels, convertedFrames);
	}
};

inline
IBaseResampler*
GetCurrentResampler()
{
	return new CR8BrainResampler();
}