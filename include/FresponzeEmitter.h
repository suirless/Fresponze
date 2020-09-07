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
#include "FresponzeListener.h"

struct FilterParameter
{
	fr_f32 FilterResonance = 0.f;		// Must be bigger than 0 and lower than 1
	fr_f32 FilterFrequency = 0.f;		// Must be converted from natural frequency to relative
	fr_f32 FilterVolume = 0.f;			// Value of linear volume setting
};

enum AdvancedEmitterConfiguration
{
	eVolumeParameter,
	eAngleParameter,			// Angle from quart value
	ePluginParametersCount,
	eInvalidParameter = 0xFFFF	// Max value for configuration values
};

/* 
	This is example how to create custom emitter with your name of 
	company or custom controls 
*/
class CAdvancedEmitter : public IBaseEmitter
{
protected:
	/* Plugin process settings */
	fr_i32 EmittersState = 0;
	fr_f32 VolumeLevel = 1.f;
	fr_f32 Angle = 0;		
	PcmFormat ListenerFormat = {};

	/* Parameters and flags */
	fr_i32 EmitterEffectCategory = CategoryEffect;
	fr_i32 EmitterEffectType = SoundEffectType;
	fr_i32 EmitterConfigurationKnob[ePluginParametersCount] = { CircleKnob, LineKnob };

	/* Names and descriptions */
	const char* EmitterName = "Advanced Pan Emitter";
	const char* EmitterDescription = "Example for creating advanced emitter with custom settings and parameters";
	const char* EmitterVendor = "Fresponze";
	const char* EmitterConfigurationDescription[ePluginParametersCount] = {
		"Volume level of audio", 
		"View angle"
	};

	/* Counting and support functions */
	void ProcessInternal(fr_f32** ppData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate);
	void FreeStuff();

public:
	CAdvancedEmitter();
	~CAdvancedEmitter();

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
