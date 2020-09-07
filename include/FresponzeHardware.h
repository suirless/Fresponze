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
#include "FresponzeEndpoint.h"
#include "FresponzeEnumerator.h"
#include "FresponzeVolumeLevel.h"

class IAudioNotificationCallback : public IBaseInterface
{
protected:
	bool bDefaultInputDevice = false;
	bool bDefaultOutputDevice = false;

public:
	virtual bool SetCurrentDevice(fr_i32 DeviceType, bool IsDefaultDevice, void* pDevicePointer) = 0;
	virtual bool DeviceDisabled(void* pDeviceDisabled) = 0;
	virtual bool DefaultDeviceChanged(fr_i32 DeviceType) = 0;
	virtual bool ListOfDevicesChanded() = 0;
};

class IAudioHardware : public IBaseInterface
{
protected:
	IAudioVolume* pAudioVolume = nullptr;
	IAudioEnumerator* pAudioEnumerator = nullptr;
	IAudioEndpoint* pInputEndpoint = nullptr;
	IAudioEndpoint* pOutputEndpoint = nullptr;
	IAudioCallback* pAudioCallback = nullptr;
	IAudioNotificationCallback* pNotificationCallback = nullptr;

public:
	virtual bool Enumerate() = 0;
	virtual void GetDevicesList(EndpointInformation*& InputList, EndpointInformation*& OutputList) = 0;
	virtual bool GetDevicesCount(fr_i32 EndpointType, fr_i32& Count) = 0;

	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime) = 0;
	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) = 0;
	virtual bool Open(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) = 0;

	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime) = 0;
	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) = 0;
	virtual bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) = 0;

	virtual bool Start() = 0;
	virtual bool Stop() = 0;

	virtual void SetVolume(fr_f32 VolumeLevel) = 0;
	virtual void GetVolume(fr_f32& VolumeLevel) = 0;

	virtual void GetEndpointInfo(fr_i32 DeviceType, EndpointInformation& endpointInfo) = 0;
	virtual void GetRawPtr(fr_i32 DeviceType, void*& OutPtr) = 0;

	virtual bool Close() = 0;
};
