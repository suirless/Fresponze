/*********************************************************************
* Copyright (C) Anton Kovalev (vertver), 2019-2020. All rights reserved.
* Copyright (C) Suirless, 2020. All rights reserved.
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
#include "FresponzeEffect.h"
#include "FresponzeListener.h"

class IAudioMixer : public IBaseInterface
{
public:
	virtual bool SetMixFormat(PcmFormat& NewFormat) = 0;
	virtual bool GetMixFormat(PcmFormat& ThisFormat) = 0;

	/*
		Function for internal use
	*/
	virtual bool Record(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
	virtual bool Update(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
	virtual bool Render(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
	virtual bool Flush() = 0;
};

class CMixerAudioCallback final : public IAudioCallback
{
protected:
	IAudioMixer* pAudioMixer = nullptr;

public:
	CMixerAudioCallback(IAudioMixer* pParentMixer)
	{
		AddRef();
		pAudioMixer = pParentMixer;
	}

	fr_err FlushCallback() override
	{
		return pAudioMixer->Flush() ? 0 : -1;
	}

	fr_err FormatCallback(PcmFormat* fmtToSwitch) override
	{
		if (!fmtToSwitch) return -1;
		return pAudioMixer->SetMixFormat(*fmtToSwitch) ? 0 : -2;
	}

	fr_err EndpointCallback(fr_f32* pData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate, fr_i32 CurrentEndpointType) override
	{
		if (!pAudioMixer) return -1;

		if (CurrentEndpointType == RenderType) {
			return (pAudioMixer->Update(pData, Frames, Channels, SampleRate) ? 0 : -1);
		}
		else if (CurrentEndpointType == CaptureType) {
			return (pAudioMixer->Record(pData, Frames, Channels, SampleRate) ? 0 : -1);
		}

		return -2;
	}

	fr_err RenderCallback(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
	{
		if (!pAudioMixer) return -1;
		return (pAudioMixer->Render(Frames, Channels, SampleRate) ? 0 : -1);
	}
};

class IAdvancedMixer : public IAudioMixer
{
protected:
	fr_i32 BufferPosition = 0;
	fr_i32 CurrentBuffer = 0;
	SoundState InputState = NoneState;
	EffectNodeStruct* pInputFirstEffect = nullptr;
	EffectNodeStruct* pMasterFirstEffect = nullptr;
	IAudioCallback* pAudioCallback = nullptr;
	C2DFloatBuffer mixBuffer = {};
	C2DFloatBuffer tempBuffer = {};
	CFloatBuffer OutputBuffer = {};
	CRingFloatBuffer RingBuffer = {};
	PcmFormat MixFormat = {};
	PcmFormat InputFormat = {};

public:
	virtual void SetBufferSamples(fr_i32 SamplesIn) = 0;

	virtual bool SetMixFormat(PcmFormat& NewFormat) = 0;
	virtual bool GetMixFormat(PcmFormat& ThisFormat) = 0;

	virtual bool Record(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
	virtual bool Update(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
	virtual bool Render(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;

	virtual bool AddEmitterToListener(ListenersNode* pListener, IBaseEmitter* pEmmiter) = 0;
	virtual bool DeleteEmitterFromListener(ListenersNode* pListener, IBaseEmitter* pEmmiter) = 0;

	virtual bool CreateListener(void* pListenerOpenLink /* local or internet link */, ListenersNode*& pNewListener, PcmFormat ListFormat = {}) = 0;
	virtual bool DeleteListener(ListenersNode* pListNode) = 0;

	virtual bool CreateEmitter(IBaseEmitter*& pEmitterToCreate, fr_i32 Type) = 0;
};