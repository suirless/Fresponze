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
#include "FresponzeWasapi.h"
#include "FresponzeEnumerator.h"

class CWASAPIAudioEnumerator final : public IAudioEnumerator
{
private:
	IMMDeviceEnumerator* pDeviceEnumerator = nullptr;
	IMMNotificationClient* pNotificationClient = nullptr;

	bool GetDeviceInfoByDevice(EndpointInformation* pEndpointStruct, IMMDevice* pDevice);
	bool OpenDeviceAndGetInfo(fr_i32 EndpointType, wchar_t* pUUID, fr_i32 DeviceId, EndpointInformation& EndpointStruct, IMMDevice*& pDevice);

public:
	CWASAPIAudioEnumerator();
	~CWASAPIAudioEnumerator();

	bool EnumerateDevices() override;
	bool GetDevicesCount(fr_i32 EndpointType, fr_i32& Count) override;

	void GetInputDeviceList(EndpointInformation*& InputDevices) override;
	void GetOutputDeviceList(EndpointInformation*& OutputDevices) override;

	bool GetDefaultDevice(fr_i32 EndpointType, IAudioEndpoint*& pOutDevice) override;
	bool GetDeviceById(fr_i32 EndpointType, fr_i32 DeviceId, IAudioEndpoint*& pOutDevice) override;
	bool GetDeviceByUUID(fr_i32 EndpointType, char* DeviceUUID, IAudioEndpoint*& pOutDevice) override;
};
