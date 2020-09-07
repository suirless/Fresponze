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
#include "FresponzeEmitter.h"

IBaseEmitter*
GetAdvancedEmitter()
{
	return new CAdvancedEmitter;
}

CAdvancedEmitter::CAdvancedEmitter()
{
	AddRef();
}

CAdvancedEmitter::~CAdvancedEmitter()
{
	IMediaListener* pTemp = ((IMediaListener*)pParentListener);
	_RELEASE(pTemp);
	FreeStuff();
}

void 
CAdvancedEmitter::FreeStuff()
{
	EffectNodeStruct* pNode = pFirstEffect;
	EffectNodeStruct* pThisNode = nullptr;
	while (pNode) {
		pThisNode = pNode->pNext;
		_RELEASE(pNode->pEffect);
		delete pNode;
		pNode = pThisNode;
	}
}

void
CAdvancedEmitter::AddEffect(IBaseEffect* pNewEffect)
{
	if (!pLastEffect) {
		pFirstEffect = new EffectNodeStruct;
		memset(pFirstEffect, 0, sizeof(EffectNodeStruct));
		pLastEffect = pFirstEffect;
		pNewEffect->Clone((void**)&pLastEffect->pEffect);
	}
	else {
		EffectNodeStruct* pTemp = new EffectNodeStruct;
		memset(pFirstEffect, 0, sizeof(EffectNodeStruct));
		pNewEffect->Clone((void**)&pTemp->pEffect);
		pLastEffect->pNext = pTemp;
		pTemp->pPrev = pLastEffect;
		pLastEffect = pTemp;
	}
}

void
CAdvancedEmitter::DeleteEffect(IBaseEffect* pNewEffect)
{
	EffectNodeStruct* pNode = pFirstEffect;
	while (pNode) {
		if (pNode->pEffect == pNewEffect) {
			pNode->pPrev->pNext = pNode->pNext;
			pNode->pNext->pPrev = pNode->pPrev;
			_RELEASE(pNode->pEffect);
			delete pNode;
			return;
		}
	}
}

void 
CAdvancedEmitter::SetFormat(PcmFormat* pFormat)
{
	ListenerFormat = *pFormat;
	EffectNodeStruct* pNEffect = pFirstEffect;
	while (pNEffect) {
		pNEffect->pEffect->SetFormat(pFormat);
		pNEffect = pNEffect->pNext;
	}
}

void
CAdvancedEmitter::GetFormat(PcmFormat* pFormat)
{
	*pFormat = ListenerFormat;
}

/* Base emitter code (parent source, position, state) */
void   
CAdvancedEmitter::SetListener(void* pListener)
{
	IMediaListener* pTemp = (IMediaListener*)pListener;
	pTemp->Clone(&pParentListener);
}

void*
CAdvancedEmitter::GetListener()
{
	return pParentListener;
}

void   
CAdvancedEmitter::SetState(fr_i32 state)
{
	EmittersState = state;
}

fr_i32
CAdvancedEmitter::GetState()
{
	return EmittersState;
}

void   
CAdvancedEmitter::SetPosition(fr_i64 FPosition)
{
	FilePosition = FPosition;
}

fr_i64 
CAdvancedEmitter::GetPosition()
{
	return FilePosition;
}

/* Advanced emitter processing code */
bool 
CAdvancedEmitter::GetEffectCategory(fr_i32& EffectCategory)
{
	EffectCategory = EmitterEffectCategory;
	return true;
}

bool 
CAdvancedEmitter::GetEffectType(fr_i32& EffectType)
{
	EffectType = EmitterEffectType;
	return true;
}

bool 
CAdvancedEmitter::GetPluginName(fr_string64& DescriptionString) 
{
	strcpy(DescriptionString, EmitterName);
	return true;
}

bool 
CAdvancedEmitter::GetPluginVendor(fr_string64& DescriptionString) 
{
	strcpy(DescriptionString, EmitterVendor);
	return true;
}

bool 
CAdvancedEmitter::GetPluginDescription(fr_string256& DescriptionString)
{
	strcpy(DescriptionString, EmitterDescription);
	return true;
}

bool 
CAdvancedEmitter::GetVariablesCount(fr_i32& CountOfVariables) 
{
	CountOfVariables = ePluginParametersCount;
	return true;
}

bool 
CAdvancedEmitter::GetVariableDescription(fr_i32 VariableIndex, fr_string128& DescriptionString)
{
	if (VariableIndex >= ePluginParametersCount) return false;
	strcpy(DescriptionString, EmitterConfigurationDescription[VariableIndex]);
	return true;
}

bool 
CAdvancedEmitter::GetVariableKnob(fr_i32 VariableIndex, fr_i32& KnobType) 
{
	if (VariableIndex >= ePluginParametersCount) return false;
	KnobType = EmitterConfigurationKnob[VariableIndex];
	return true;
}

void 
CAdvancedEmitter::SetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize)
{
	if (!pData) return;
	if (Option >= ePluginParametersCount) return;
	if (DataSize != sizeof(fr_f32)) return;
	fr_f32 ValueToApply = *pData;

	switch (Option)
	{
	case eVolumeParameter:		VolumeLevel = ValueToApply; break;
	case eAngleParameter:		Angle = ValueToApply; break;
	default:
		break;
	}
}

void 
CAdvancedEmitter::GetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize)
{
	if (!pData) return;
	if (Option >= ePluginParametersCount) return;
	if (DataSize != sizeof(fr_f32)) return;
	fr_f32& ValueToApply = *pData;

	switch (Option)
	{
	case eVolumeParameter:		ValueToApply = VolumeLevel; break;
	case eAngleParameter:		ValueToApply = Angle; break;
	default:
		break;
	}
}

void
CAdvancedEmitter::ProcessInternal(fr_f32** ppData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	/* Apple angle to signal */
	if (Channels >= 2) {
		fr_f32 leftcoeff = cosf(Angle) - sinf(Angle);
		fr_f32 rightcoeff = cosf(Angle) + sinf(Angle);
		for (size_t o = 0; o < Frames; o++) {
			ppData[0][o] *= leftcoeff;
			ppData[1][o] *= rightcoeff;
		}
	}

	for (size_t i = 0; i < Channels; i++) {
		for (size_t o = 0; o < Frames; o++) {
			ppData[i][o] *= VolumeLevel;
		}
	}
}

bool 
CAdvancedEmitter::Process(fr_f32** ppData, fr_i32 Frames) 
{
	if (!pParentListener) return false;
	fr_i32 BaseEmitterPosition = 0;
	fr_i32 BaseListenerPosition = 0;
	fr_i32 FramesReaded = 0;
	IMediaListener* ThisListener = (IMediaListener*)pParentListener;

	if (EmittersState == eStopState || EmittersState == ePauseState) return false;

	/* Get current position of listener and emitter to reset old state */
	BaseEmitterPosition = (fr_i32)GetPosition();
	BaseListenerPosition = (fr_i32)ThisListener->GetPosition();

	fr_i64 FullFileSize = ThisListener->GetFullFrames();
	if (BaseEmitterPosition > FullFileSize) {
		BaseEmitterPosition = 0;
	}

	/* Set emitter position to listener and read data */
	fr_i64 SettedListenerPosition = ThisListener->SetPosition((fr_i64)BaseEmitterPosition);
	FramesReaded = ThisListener->Process(ppData, Frames);
	fr_i64 CurrentListPosition = ThisListener->GetPosition();

	if (FramesReaded < Frames) {
		BaseEmitterPosition = 0;
		ThisListener->SetPosition((fr_i64)BaseEmitterPosition);

		/* We don't want replay audio if we set this flag */
		if (EmittersState == ePlayState) {
			EmittersState = eStopState;
		} else {
			fr_f32* pTempData[MAX_CHANNELS] = {};
			for (size_t i = 0; i < ListenerFormat.Channels; i++) {
				pTempData[i] = &ppData[i][FramesReaded];
			}

			FramesReaded += ThisListener->Process(pTempData, Frames - FramesReaded);
			if (FramesReaded != Frames) {
				TypeToLogFormated("Wrong formated buffer: expected %u, but value is %u", Frames, FramesReaded);
			}
			BaseEmitterPosition = ThisListener->GetPosition();
		}
	} else {
		BaseEmitterPosition += FramesReaded;
	}

	/* Process by emitter effect */
	ProcessInternal(ppData, Frames, ListenerFormat.Channels, ListenerFormat.SampleRate);
	EffectNodeStruct* pEffectToProcess = pFirstEffect;
	while (pEffectToProcess) {
		pEffectToProcess->pEffect->Process(ppData, Frames);
		pEffectToProcess = pEffectToProcess->pNext;
	}

	SetPosition(BaseEmitterPosition);
	return true;
}

