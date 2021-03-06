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
#include <audiopolicy.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

class CWASAPIAudioEnpoint final : public IAudioEndpoint
{
private:
	fr_f32 DelayCustom = 0.f;
	HANDLE hThread = nullptr;
	IBaseEvent* pThreadEvent = nullptr;
	IMMDevice* pCurrentDevice = nullptr;
	IAudioClient* pAudioClient = nullptr;
	IAudioRenderClient* pRenderClient = nullptr;
	IAudioCaptureClient* pCaptureClient = nullptr;

	bool CreateWasapiThread();
	bool GetEndpointDeviceInfo();

	bool InitalizeEndpoint()
	{
		pSyncEvent = new CWinEvent;
		pThreadEvent = new CWinEvent;
		pStartEvent = new CWinEvent;
		return !!pThreadEvent || !!pSyncEvent;
	}

	bool InitializeClient(IMMDevice* pDev)
	{	
		if (pDev) return SUCCEEDED(pDev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient));
		return false;
	}

	bool InitializeToPlay(fr_f32 Delay);

public:
	CWASAPIAudioEnpoint(fr_i32 DeviceType, void* pMMDevice, EndpointInformation& Info)
	{
		AddRef();
		IMMDevice* pTempDevice = (IMMDevice*)pMMDevice;
		if (!pTempDevice) return;

		pTempDevice->QueryInterface(IID_PPV_ARGS(&pCurrentDevice));
		InitalizeEndpoint();
		memcpy(&EndpointInfo, &Info, sizeof(EndpointInformation));
		EndpointInfo.Type = DeviceType;
	}

	~CWASAPIAudioEnpoint()
	{
		Close();
		_RELEASE(pAudioCallback);
		_RELEASE(pCaptureClient);
		_RELEASE(pRenderClient);
		_RELEASE(pAudioClient);
		_RELEASE(pCurrentDevice);
		if (pSyncEvent) delete pSyncEvent;
		if (pThreadEvent) delete pThreadEvent;
	}

	void GetDevicePointer(void*& pDevice) override;
	void ThreadProc();
    void GetDeviceFormat(PcmFormat& pcmFormat) override { pcmFormat = EndpointInfo.EndpointFormat; }
	void SetDeviceInfo(EndpointInformation& DeviceInfo) override;
	void GetDeviceInfo(EndpointInformation& DeviceInfo) override;
	void SetCallback(IAudioCallback* pCallback) override;
	bool Open(fr_f32 Delay) override;
	bool Close() override;
	bool Start() override;
	bool Stop() override;
};
