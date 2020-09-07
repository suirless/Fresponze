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

class IAudioEndpoint : public IBaseInterface
{
protected:
	fr_f32* pTempBuffer = nullptr;
	void* pBackBuffer = nullptr;
	IAudioCallback* pAudioCallback = nullptr;
	IBaseEvent* pSyncEvent = nullptr;
	IBaseEvent* pStartEvent = nullptr;
	EndpointInformation EndpointInfo = {};

public:	
	virtual void GetDevicePointer(void*& pDevice) = 0;
	virtual void SetDeviceInfo(EndpointInformation& DeviceInfo) = 0;
	virtual void GetDeviceInfo(EndpointInformation& DeviceInfo) = 0;
	virtual void SetCallback(IAudioCallback* pCallback) = 0;
	virtual bool Open(fr_f32 Delay) = 0;
	virtual bool Close() = 0;
	virtual bool Start() = 0;
	virtual bool Stop() = 0;
};
