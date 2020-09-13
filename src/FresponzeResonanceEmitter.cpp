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
#include "FresponzeResonanceEmitter.h"
#ifdef FRESPONZE_USE_RESONANCE_API
#include "resonance_audio_api.h"
#include "binaural_surround_renderer.h"

IBaseEmitter*
GetResonanceEmitter()
{
    return new CResonanceEmitter();
}

CResonanceEmitter::CResonanceEmitter()
{
    AddRef();
}

CResonanceEmitter::~CResonanceEmitter()
{
    IMediaListener* pTemp = ((IMediaListener*)pParentListener);
    _RELEASE(pTemp);
    FreeStuff();
}

void
CResonanceEmitter::FreeStuff()
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
CResonanceEmitter::AddEffect(IBaseEffect* pNewEffect)
{
    if (!pLastEffect) {
        pFirstEffect = new EffectNodeStruct;
        memset(pFirstEffect, 0, sizeof(EffectNodeStruct));
        pLastEffect = pFirstEffect;
        pNewEffect->Clone((void**)&pLastEffect->pEffect);
    } else {
        EffectNodeStruct* pTemp = new EffectNodeStruct;
        memset(pFirstEffect, 0, sizeof(EffectNodeStruct));
        pNewEffect->Clone((void**)&pTemp->pEffect);
        pLastEffect->pNext = pTemp;
        pTemp->pPrev = pLastEffect;
        pLastEffect = pTemp;
    }
}

void
CResonanceEmitter::DeleteEffect(IBaseEffect* pNewEffect)
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
CResonanceEmitter::SetFormat(PcmFormat* pFormat)
{
    ListenerFormat = *pFormat;
    EffectNodeStruct* pNEffect = pFirstEffect;
    BugAssert((pFormat->SampleRate && pFormat->Frames && pFormat->Channels), "Wrong input PCM Format");
    if (!(pFormat->SampleRate && pFormat->Frames && pFormat->Channels)) {
        return;
    }

    while (pNEffect) {
        pNEffect->pEffect->SetFormat(pFormat);
        pNEffect = pNEffect->pNext;
    }

    /*
        yeah, resonance audio doesn't provide additional API to destroy object, so we
        just delete pointer vie 'delete' operator
     */
    if (pResonanceAudio) {
        // fuck yeah, we also must free source before destroying resonance API header
        pResonanceAudio->DestroySource(EmitterSourceId);
        delete pResonanceAudio;
        pResonanceAudio = nullptr;
        EmitterSourceId = 0;
    }

    pResonanceAudio = vraudio::CreateResonanceAudioApi(pFormat->Channels, pFormat->Frames, pFormat->SampleRate);
    EmitterSourceId = pResonanceAudio->CreateAmbisonicSource(pFormat->Channels);
}

void
CResonanceEmitter::GetFormat(PcmFormat* pFormat)
{
    *pFormat = ListenerFormat;
}

/* Base emitter code (parent source, position, state) */
void
CResonanceEmitter::SetListener(void* pListener)
{
    IMediaListener* pTemp = (IMediaListener*)pListener;
    pTemp->Clone(&pParentListener);
}

void*
CResonanceEmitter::GetListener()
{
    return pParentListener;
}

void
CResonanceEmitter::SetState(fr_i32 state)
{
    EmittersState = state;
}

fr_i32
CResonanceEmitter::GetState()
{
    return EmittersState;
}

void
CResonanceEmitter::SetPosition(fr_i64 FPosition)
{
    FilePosition = FPosition;
}

fr_i64
CResonanceEmitter::GetPosition()
{
    return FilePosition;
}

/* Advanced emitter processing code */
bool
CResonanceEmitter::GetEffectCategory(fr_i32& EffectCategory)
{
    EffectCategory = EmitterEffectCategory;
    return true;
}

bool
CResonanceEmitter::GetEffectType(fr_i32& EffectType)
{
    EffectType = EmitterEffectType;
    return true;
}

bool
CResonanceEmitter::GetPluginName(fr_string64& DescriptionString)
{
    strcpy(DescriptionString, EmitterName);
    return true;
}

bool
CResonanceEmitter::GetPluginVendor(fr_string64& DescriptionString)
{
    strcpy(DescriptionString, EmitterVendor);
    return true;
}

bool
CResonanceEmitter::GetPluginDescription(fr_string256& DescriptionString)
{
    strcpy(DescriptionString, EmitterDescription);
    return true;
}

bool
CResonanceEmitter::GetVariablesCount(fr_i32& CountOfVariables)
{
    CountOfVariables = ePluginParametersCount;
    return true;
}

bool
CResonanceEmitter::GetVariableDescription(fr_i32 VariableIndex, fr_string128& DescriptionString)
{
    if (VariableIndex >= ePluginParametersCount) return false;
    strcpy(DescriptionString, EmitterConfigurationDescription[VariableIndex]);
    return true;
}

bool
CResonanceEmitter::GetVariableKnob(fr_i32 VariableIndex, fr_i32& KnobType)
{
    if (VariableIndex >= ePluginParametersCount) return false;
    KnobType = EmitterConfigurationKnob[VariableIndex];
    return true;
}

void
CResonanceEmitter::SetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize)
{
    if (!pData) return;
    if (Option >= ePluginParametersCount) return;

    switch (Option) {
        case eVolumeParameter:
        case eAngleParameter:
        default:
            break;
    }
}

void
CResonanceEmitter::GetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize)
{
    if (!pData) return;
    if (Option >= ePluginParametersCount) return;

    switch (Option) {
        case eVolumeParameter:
        case eAngleParameter:
        default:
            break;
    }
}

void
CResonanceEmitter::ProcessInternal(fr_f32** ppData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
    for (size_t i = 0; i < Channels; i++) {
        for (size_t o = 0; o < Frames; o++) {
            ppData[i][o] *= VolumeLevel;
        }
    }
}

bool
CResonanceEmitter::Process(fr_f32** ppData, fr_i32 Frames)
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
#else
IBaseEmitter*
GetResonanceEmitter()
{
    return nullptr;
}
#endif