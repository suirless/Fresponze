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
#include "FresponzeEmitter.h"

namespace vraudio {
    class ResonanceAudioApi;
    typedef int SourceId;
}

enum CResonanceEmitterEnum
{
    eRVolumeParameter = 0,
    eREmmiterPosition,
    eREmitterRotation,
    eRListenerPosition,
    eRListenerRotation,
    eRHRTF,
    eRMaxDistance,
    eRMinDistance,
    eRPluginParametersCount,
    eRInvalidParameter = 0xFFFF	// Max value for configuration values
};

class CResonanceEmitter final : public IBaseEmitter
{
protected:
    vraudio::ResonanceAudioApi* pResonanceAudio = nullptr;
    vraudio::SourceId EmitterSourceId = 0;

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
    const char* EmitterName = "Resonance HRTF Emitter";
    const char* EmitterDescription = "Custom Resonance Audio Emitter with HRTF panning";
    const char* EmitterVendor = "Google";
    const char* EmitterConfigurationDescription[ePluginParametersCount] = {
            "Volume level of audio",
            "View angle"
    };

    /* Counting and support functions */
    void ProcessInternal(fr_f32** ppData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate);
    void FreeStuff();

public:
    CResonanceEmitter();
    ~CResonanceEmitter() override;

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

IBaseEmitter* GetResonanceEmitter();