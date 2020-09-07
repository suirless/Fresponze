/*********************************************************************
* Copyright (C) Anton Kovalev (vertver), 2020. All rights reserved.
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
#include "FresponzeEmitter.h"
#ifdef STEAM_AUDIO_BUILD
#include "phonon.h"

enum ESteamAudioOptions
{
	eSVolumeParameter,
	eXAxis,
	eYAxis,
	eZAxis,
	eCountOfParameters
};

/* 
	Master emitter created for post and pre mixer processing.
	Ex. Integrating wWise into Fresponze Audio Engine and
	set output via custom WASAPI (or other) output.
*/
class CSteamAudioEmitter : public IBaseEmitter
{
private:
	fr_f32 VolumeLevel = 0.f;
	fr_i32 EmittersState = 0;
	fr_i32 CurrentSourceId = 0;
	PcmFormat outputFormat = {};
	IPLVector3 vectorOfAngle = { 0.5f, 0.f, 0.5f};
	PcmFormat ListenerFormat = {};
	IPLhandle BinauralRender = {};
	IPLhandle BinauralEffect = {};
	IPLAudioFormat EmitterFormat = {};
	IPLRenderingSettings EmitterSettings = {};
	IPLHrtfParams hrtfParams = {};
	IPLAudioBuffer InputBuffer = {};
	IPLAudioBuffer OutputBuffer = {};

	void Reset();
	void Create();
	void FreeStuff();

public:
	CSteamAudioEmitter();
	~CSteamAudioEmitter();

	void AddEffect(IBaseEffect* pNewEffect) override;
	void DeleteEffect(IBaseEffect* pNewEffect) override;

	void SetFormat(PcmFormat* pFormat) override;
	void GetFormat(PcmFormat* pFormat) override;

	void SetListener(void* pListener) override;
	void SetState(fr_i32 state) override;
	void SetPosition(fr_i64 FPosition) override;

	void* GetListener() override;
	fr_i32 GetState() override;
	fr_i64 GetPosition() override;

	bool GetEffectCategory(fr_i32& EffectCategory) override;
	bool GetEffectType(fr_i32& EffectType) override;

	bool GetPluginName(fr_string64& DescriptionString) override;
	bool GetPluginVendor(fr_string64& DescriptionString) override;
	bool GetPluginDescription(fr_string256& DescriptionString) override;

	bool GetVariablesCount(fr_i32& CountOfVariables) override;
	bool GetVariableDescription(fr_i32 VariableIndex, fr_string128& DescriptionString) override;
	bool GetVariableKnob(fr_i32 VariableIndex, fr_i32& KnobType) override;
	void SetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize) override;
	void GetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize) override;

	bool Process(fr_f32** ppData, fr_i32 Frames) override;
};
#endif

IBaseEmitter* GetSteamEmitter();