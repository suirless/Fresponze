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
#include <xaudio2.h>
#include <mmdeviceapi.h>
#include <AudioClient.h>

#ifndef GUID_SECT
#define GUID_SECT
#endif

#define __FRDEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define FRDEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_CLSID(FRE_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define FRDEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_IID(FRE_IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)

#define maxmin(a, minimum, maximum)  min(max(a, minimum), maximum)

inline
bool 
GetDeviceInformation(IMMDevice* pDevicePtr, PcmFormat& fmt)
{
	FRDEFINE_IID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 00000003, 0000, 0010, 80, 00, 00, aa, 00, 38, 9b, 71);
	IAudioClient* pAudioClient = nullptr;
	WAVEFORMATEX* pWaveFormat = nullptr;
	PROPVARIANT value = { 0 };

	if (FAILED(pDevicePtr->Activate(__uuidof(pAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient))) return false;
	if (FAILED(pAudioClient->GetMixFormat(&pWaveFormat))) {
		_RELEASE(pAudioClient);
		return false;
	}

	/* Set local format struct */
	fmt.IsFloat = pWaveFormat->wFormatTag == 3;
	fmt.Bits = pWaveFormat->wBitsPerSample;
	fmt.Channels = pWaveFormat->nChannels;
	fmt.SampleRate = pWaveFormat->nSamplesPerSec;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE* pTmp = (WAVEFORMATEXTENSIBLE*)pWaveFormat;
		fmt.IsFloat = pTmp->SubFormat == FRE_IID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	}

	if (pWaveFormat) CoTaskMemFree(pWaveFormat);
	_RELEASE(pAudioClient);
	return true;
}

enum EXAudio2Versions : fr_i32
{
	eNoneVersion,
	eXAudio27,
	eXAudio28,
	eXAudio29
};

IXAudio2* InitializeXAudio2(fr_i32& Version);

class CXAudio2AudioEndpoint final : public IAudioEndpoint
{
private:
	IXAudio2* pXAudio2 = nullptr;
	IXAudio2MasteringVoice* pMasteringVoice = nullptr;

public:
	CXAudio2AudioEndpoint(IXAudio2* pTempXAudio2, IXAudio2MasteringVoice* pTempMasteringVoice, EndpointInformation& Info)
	{
		if (pTempXAudio2) {
			pXAudio2 = pTempXAudio2;
			pMasteringVoice = pTempMasteringVoice;
			pXAudio2->AddRef();
			SetDeviceInfo(Info);
			_InterlockedIncrement(&Counter);
		}
	}

	~CXAudio2AudioEndpoint()
	{
		Close();
		if (pMasteringVoice) pMasteringVoice->DestroyVoice();
		_RELEASE(pAudioCallback);	
		_RELEASE(pXAudio2);
	}

	void GetDevicePointer(void*& pDevice) override;
	void ThreadProc();
	void SetDeviceInfo(EndpointInformation& DeviceInfo) override;
	void GetDeviceInfo(EndpointInformation& DeviceInfo) override;
	void SetCallback(IAudioCallback* pCallback) override;
	bool Open(fr_f32 Delay) override;
	bool Close() override;
	bool Start() override;
	bool Stop() override;
};
