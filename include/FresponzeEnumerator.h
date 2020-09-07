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

class IAudioEnumerator : public IBaseInterface
{
protected:
	fr_i32 InputDevices = 0;
	fr_i32 OutputDevices = 0;
	EndpointInformation DefaultInputInfo = {};
	EndpointInformation DefaultOutputInfo = {};
	EndpointInformation* InputDevicesInfo = nullptr;
	EndpointInformation* OutputDevicesInfo = nullptr;

public:
	virtual bool EnumerateDevices() = 0;
	virtual bool GetDevicesCount(fr_i32 EndpointType, fr_i32& Count) = 0;

	virtual void GetInputDeviceList(EndpointInformation*& InputDevices) = 0;
	virtual void GetOutputDeviceList(EndpointInformation*& OutputDevices) = 0;

	virtual bool GetDefaultDevice(fr_i32 EndpointType, IAudioEndpoint*& pOutDevice) = 0;
	virtual bool GetDeviceById(fr_i32 EndpointType, fr_i32 DeviceId, IAudioEndpoint*& pOutDevice) = 0;
	virtual bool GetDeviceByUUID(fr_i32 EndpointType, char* DeviceUUID, IAudioEndpoint*& pOutDevice) = 0;
};
