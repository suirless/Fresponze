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
#include "FresponzeHardware.h"
#include "FresponzeMixer.h"

class IFresponze : public IBaseInterface
{
protected:
	IAudioCallback* pAudioCallback = nullptr;
	IAudioHardware* pAudioHardware = nullptr;
	IAudioMixer* pAudioMixer = nullptr;

public:
	virtual bool GetRemoteInterface(void** ppOutInterface) = 0;

	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime) = 0;
	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) = 0;
	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) = 0;

	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime) = 0;
	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) = 0;
	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) = 0;

	virtual bool Start() = 0;
	virtual bool Stop() = 0;

	virtual bool Close() = 0;
};
