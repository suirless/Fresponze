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
#include "FresponzeTypes.h"

/* Effect chain type and FFT settings */
enum EEffectType : fr_i32
{
	UnknownEffectType,		// No effect
	SoundEffectType,		// For single sound or input signal
	PreMixEffect,			// For pre-master state, for check audio engine picture 
	AfterMixEffect, 		// For master-channel
	FFTEffectType			// Special FFT-effect, in FFT chain. Do not recommended add more then 2 parallel FFT effects (bad perfomance issue)
};

/* Visualisation and knobs */
enum EKnobType : fr_i32
{
	NoKnobType,		// Text
	CircleKnob,		// Circle-like knob
	LineKnob,		// Flat knob
	CustomKnob		// Custom knob, just draw it yourself
};

/* FREFFECT VERSION 1.1 (DO NOT DELETE) */
class IBaseEffect : public IBaseInterface
{
public:
	virtual bool GetEffectCategory(fr_i32& EffectCategory) = 0;			// Use BasePluginCategory enum here
	virtual bool GetEffectType(fr_i32& EffectType) = 0;

	virtual bool GetPluginName(fr_string64& DescriptionString) = 0;
	virtual bool GetPluginVendor(fr_string64& DescriptionString) = 0;
	virtual bool GetPluginDescription(fr_string256& DescriptionString) = 0;

	virtual bool GetVariablesCount(fr_i32& CountOfVariables) = 0;

	/* VERSION 1.1 ADDITION BEGIN */
	virtual bool GetVariableDescription(fr_i32 VariableIndex, fr_string128& DescriptionString) = 0;
	/* VERSION 1.1 ADDITION END */

	virtual bool GetVariableKnob(fr_i32 VariableIndex, fr_i32& KnobType) = 0;
	virtual void SetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize) = 0;
	virtual void GetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize) = 0;

	virtual bool Process(fr_f32** ppData, fr_i32 Frames) = 0;

	/* VERSION 1.2 ADDITION BEGIN */
	virtual void SetFormat(PcmFormat* pFormat) = 0;
	virtual void GetFormat(PcmFormat* pFormat) = 0;
	/* VERSION 1.2 ADDITION END */

	/* Add functions to interface here */
}; 

struct EffectNodeStruct;
struct EffectNodeStruct
{
	EffectNodeStruct* pNext;
	EffectNodeStruct* pPrev;
	IBaseEffect* pEffect;  
	void* pReserved;
};
