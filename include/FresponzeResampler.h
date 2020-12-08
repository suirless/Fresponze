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
    virtual ~IBaseResampler() = default;
	virtual void Initialize(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) = 0;
	virtual void Destroy() = 0;
	virtual void Reset(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) = 0;
	virtual void Resample(fr_i32 frames, fr_f32** inputData, fr_f32** outputData) = 0;
	virtual void ResampleDouble(fr_i32 frames, fr_f64** inputData, fr_f64** outputData) = 0;
};



class CR8BrainResampler : public IBaseResampler
{
private:
	bool lin = false;
	fr_i32 bufLength = 0;
	fr_i32 inSRate = 0;
	fr_i32 outSRate = 0;
	fr_i32 channels = 0;
	fr_i32 DelayTime = 0;
	fr_f64* StaticFloatBuffer[8] = {};
	r8b::CDSPResampler* resampler[8] = {};

public:
	fr_f32 GetOutSampleRate() {
		return outSRate;
	}

	~CR8BrainResampler() override
	{
		Destroy();
	}

	void Initialize(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear) override
	{
		bufLength = MaxBufferIn;
		inSRate = InputSampleRate;
		outSRate = OutputSampleRate;
		channels = ChannelsCount;
		for (size_t i = 0; i < ChannelsCount; i++) {
			resampler[i] = new r8b::CDSPResampler(inSRate, outSRate, bufLength, 2.0, isLinear ? 136.45 : 109.56, isLinear ? r8b::fprLinearPhase : r8b::fprMinPhase);
			StaticFloatBuffer[i] = (fr_f64*)FastMemAlloc(bufLength * sizeof(fr_f64));
		}

		Flush();
	}

	fr_i32 GetDelayTime()
	{
		return DelayTime;
	}

	void Destroy()
	{
		size_t index = 0;
		for (auto& resampler_ptr : resampler) {
			if (resampler_ptr) {
				delete resampler_ptr;
				resampler_ptr = nullptr;
			}
		}

		for (auto& float_ptr : StaticFloatBuffer) {
			if (float_ptr) {
				FreeFastMemory(float_ptr);
				float_ptr = nullptr;
			}
		}
	}

	void Flush()
	{
		for (auto& elem : StaticFloatBuffer) {
			if (elem) memset(elem, 0, bufLength * sizeof(fr_f64));
		}

		for (auto& elem : resampler) {
			if (elem) elem->clear();
		}

		if (resampler[0]) {
			DelayTime = resampler[0]->getInLenBeforeOutStart();
		}
	}

	void Reset(fr_i32 MaxBufferIn, fr_i32 InputSampleRate, fr_i32 OutputSampleRate, fr_i32 ChannelsCount, bool isLinear)  override
	{
		if (MaxBufferIn != bufLength || inSRate != InputSampleRate || outSRate != OutputSampleRate || channels != ChannelsCount) {
			Destroy();
			Initialize(MaxBufferIn, InputSampleRate, OutputSampleRate, ChannelsCount, isLinear);
		}
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
		fr_i32 maxSize = std::max(frames, convertedFrames) * 2;

		fr_f64* floatBufTemp[8] = {};
		FloatToDouble(inputData, StaticFloatBuffer, channels, frames);
		for (size_t i = 0; i < channels; i++) {
			double* tempFirstPointer = StaticFloatBuffer[i];
			double*& tempSecondPointer = floatBufTemp[i];
			resampler[i]->process(tempFirstPointer, frames, tempSecondPointer);
			DoubleToFloatSingle(outputData[i], tempSecondPointer, convertedFrames);
		}
	}
};

inline
IBaseResampler*
GetCurrentResampler()
{
	return new CR8BrainResampler();
}