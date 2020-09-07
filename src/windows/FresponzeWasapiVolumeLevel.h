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
#include "FresponzeVolumeLevel.h"
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

class CWASAPIAudioSessionEvents : public IAudioSessionEvents
{
private:
	long Ref = 0;
	IAudioHardware* pParentHardware = nullptr;
	IAudioVolume* pParentAudioVolume = nullptr;
	IMMDevice* pParentDevice = nullptr;

public:
	CWASAPIAudioSessionEvents(IAudioVolume* pAudioVolume, IMMDevice* pNParentDevice, IAudioHardware* pAudioHardware)
	{
		AddRef();
		pParentHardware = pAudioHardware;
		pParentAudioVolume = pAudioVolume;
		pParentDevice = pNParentDevice;
	}

	virtual HRESULT STDMETHODCALLTYPE OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) override {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) override {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext) override {
		pParentAudioVolume->SetVirtualVolume(NewVolume);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext) override {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) override {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnStateChanged(AudioSessionState NewState) override {
		switch (NewState)
		{
		case AudioSessionStateInactive:
			TypeToLog("WASAPI: Audio session state changed: inactive");
			break;
		case AudioSessionStateActive:
			TypeToLog("WASAPI: Audio session state changed: active");
			break;
		case AudioSessionStateExpired:
			TypeToLog("WASAPI: Audio session state changed: expired");
			break;
		default:
			break;
		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override {
		if (!pParentHardware->Open(RenderType, 100.f)) return E_FAIL;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &, void **) override {
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE AddRef() override {
		return _InterlockedIncrement(&Ref);
	}

	ULONG STDMETHODCALLTYPE Release() override {
		ULONG ulRef = _InterlockedDecrement(&Ref);
		if (0 == ulRef) {
			delete this;
		}
		return ulRef;
	}
};

class CWASAPIAudioVolume : public IAudioVolume
{
private:
	IAudioHardware* pParentHardware = nullptr;
	IMMDevice* pParentDevice = nullptr;
	IAudioSessionControl* pAudioSessionControl = nullptr;
	IAudioSessionEvents* pAudioSessionEvents = nullptr;
	IAudioSessionManager* pAudioSessionManager = nullptr;
	ISimpleAudioVolume* pSimpleAudioVolume = nullptr;

public:
	CWASAPIAudioVolume(IMMDevice* pDevice, IAudioHardware* pAudioHardware)
	{
		pAudioHardware->Clone((void**)&pParentHardware);
		if (SUCCEEDED(pDevice->QueryInterface(__uuidof(IMMDevice), (void**)&pParentDevice))) {
			if (SUCCEEDED(pParentDevice->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, NULL, (void**)&pAudioSessionManager))) {
				if (SUCCEEDED(pAudioSessionManager->GetAudioSessionControl(nullptr, 0, &pAudioSessionControl))) {
					pAudioSessionEvents = new CWASAPIAudioSessionEvents(this, pDevice, pParentHardware);
					pAudioSessionControl->RegisterAudioSessionNotification(pAudioSessionEvents);
				}

				pAudioSessionManager->GetSimpleAudioVolume(nullptr, FALSE, &pSimpleAudioVolume);
			}
		}
	}

	~CWASAPIAudioVolume()
	{
		if (pAudioSessionControl) {
			pAudioSessionControl->UnregisterAudioSessionNotification(pAudioSessionEvents);
			pAudioSessionControl->AddRef();
			_RELEASE(pAudioSessionControl);
		}

		_RELEASE(pAudioSessionControl);
		_RELEASE(pAudioSessionEvents);
		_RELEASE(pSimpleAudioVolume);
		_RELEASE(pAudioSessionManager);
		_RELEASE(pParentDevice);
		_RELEASE(pParentHardware);
	}

	bool GetVolume(fr_f32& fVolume) override
	{
		if (!pSimpleAudioVolume) return false;
		return SUCCEEDED(pSimpleAudioVolume->GetMasterVolume(&fVolume));
	}

	bool SetVolume(fr_f32 fVolume) override
	{
		if (!pSimpleAudioVolume) return false;
		return SUCCEEDED(pSimpleAudioVolume->SetMasterVolume(fVolume, nullptr));
	}

	bool GetVirtualVolume(fr_f32& fVolume)
	{
		if (VirtualVolume < 0.0001f) {
			GetVolume(VirtualVolume);
		}

		fVolume = VirtualVolume;
		return true;
	}

	bool SetVirtualVolume(fr_f32 fVolume)
	{
		VirtualVolume = fVolume;
		return true;
	}

	bool IsMuted() override
	{
		BOOL bMute = FALSE;
		bool isPassed = SUCCEEDED(pSimpleAudioVolume->GetMute(&bMute));
		return isPassed ? !!bMute : false;
	}

	bool Mute(bool bState) override
	{
		return SUCCEEDED(pSimpleAudioVolume->SetMute(bState, nullptr));
	}
};
