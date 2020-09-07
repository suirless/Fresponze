/*****************************************************************
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
#include "FresponzeWasapiNotification.h"
#include "FresponzeWasapiVolumeLevel.h"
#include "FresponzeMixer.h"

class CWASAPIAudioNotificationCallback final : public IAudioNotificationCallback
{
private:
	wchar_t* pInputStringUUID = nullptr;
	wchar_t* pOutputStringUUID = nullptr;
	IAudioHardware* pParentHardware = nullptr;

public:
	CWASAPIAudioNotificationCallback(IAudioHardware* pHardwareNew)
	{
		pHardwareNew->Clone((void**)&pParentHardware);
	}

	~CWASAPIAudioNotificationCallback()
	{
		_RELEASE(pParentHardware);
	}

	bool SetCurrentDevice(fr_i32 DeviceType, bool IsDefaultDevice, void* pDevicePointer) override
	{
		wchar_t** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputStringUUID : DeviceType == CaptureType ? &pInputStringUUID : nullptr);
		IMMDevice* pTempDevice = (IMMDevice*)pDevicePointer;

		if (ppThisEndpoint) {
			if (*ppThisEndpoint) {
				CoTaskMemFree(*ppThisEndpoint);
				*ppThisEndpoint = nullptr;
			}
		}

		if (DeviceType == RenderType) {
			bDefaultOutputDevice = IsDefaultDevice;
		} else if (DeviceType == CaptureType) {
			bDefaultInputDevice = IsDefaultDevice;
		}

		if (FAILED(pTempDevice->GetId(ppThisEndpoint))) return false;	  
		return true;
	}

	bool DeviceDisabled(void* pDeviceDisabled) override
	{
		wchar_t* pThisEndpoint = (wchar_t*)pDeviceDisabled;
		if (!pParentHardware->Enumerate()) return false;
		if (!pOutputStringUUID) {
			if (!pParentHardware->Restart(RenderType, 100.f)) return false;
			return true;
		}

		if (!wcscmp(pThisEndpoint, pOutputStringUUID)) {
			if (!pParentHardware->Restart(RenderType, 100.f)) return false;
			return true;
		}

		if (!pInputStringUUID) {
			if (!pParentHardware->Restart(CaptureType, 100.f)) return false;
			return true;
		}

		if (!wcscmp(pThisEndpoint, pInputStringUUID)) {
			if (!pParentHardware->Restart(CaptureType, 100.f)) return false;
			return true;
		}

		return true;
	}

	bool DefaultDeviceChanged(fr_i32 DeviceType) override
	{
		if (DeviceType == RenderType) {
			if (bDefaultOutputDevice) {
				if (!pParentHardware->Restart(RenderType, -1)) return false;
			}
		} else if (DeviceType == CaptureType) {
			if (bDefaultOutputDevice) {
				if (bDefaultInputDevice) if (!pParentHardware->Restart(CaptureType, -1)) return false;
			}
		}

		return true;
	}

	bool ListOfDevicesChanded() override
	{
		return pParentHardware->Enumerate();
	}
};

class CWASAPIAudioHardware final : public IAudioHardware
{
private:
	bool isLocalCallback = false;
	IMMNotificationClient* pNotifyClient = nullptr;

	void FreeAndRestoreVolumeShit(void* pPointer)
	{
		if (pAudioVolume) _RELEASE(pAudioVolume);
		pAudioVolume = new CWASAPIAudioVolume((IMMDevice*)pPointer, this);
	}

public:
	CWASAPIAudioHardware(IAudioCallback* pParentAudioCallback)
	{
		AddRef();
		pAudioCallback = pParentAudioCallback;
		pAudioEnumerator = new CWASAPIAudioEnumerator();
		pNotificationCallback = new CWASAPIAudioNotificationCallback(this);
		pNotifyClient = new CWASAPIAudioNotification(pNotificationCallback);
	}

	~CWASAPIAudioHardware()
	{
		_RELEASE(pInputEndpoint);
		_RELEASE(pOutputEndpoint);
		_RELEASE(pNotifyClient);
		_RELEASE(pNotificationCallback)
		_RELEASE(pAudioEnumerator);
		_RELEASE(pAudioVolume);
	}

	bool Enumerate() override
	{
		return pAudioEnumerator->EnumerateDevices();
	}

	void GetDevicesList(EndpointInformation*& InputList, EndpointInformation*& OutputList) override
	{
		Enumerate();
		pAudioEnumerator->GetInputDeviceList(InputList);
		pAudioEnumerator->GetOutputDeviceList(OutputList);
	}

	bool GetDevicesCount(fr_i32 EndpointType, fr_i32& Count)  override
	{
		return pAudioEnumerator->GetDevicesCount(EndpointType, Count);
	}

	bool Open(fr_i32 DeviceType, fr_f32 DelayTime) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		if (!pAudioEnumerator->GetDefaultDevice(DeviceType, pThisEndpoint)) return false;
		if (!pThisEndpoint->Open(DelayTime)) {
			_RELEASE(pThisEndpoint);
			return false;
		}

		if (DeviceType == RenderType) {
			void* pPointer = nullptr;
			pThisEndpoint->GetDevicePointer(pPointer);
			FreeAndRestoreVolumeShit(pPointer);
		}

		void* rawPtr = nullptr;
		pThisEndpoint->GetDevicePointer(rawPtr);
		pNotificationCallback->SetCurrentDevice(DeviceType, true, rawPtr);
		pThisEndpoint->SetCallback(pAudioCallback);
		return true;
	}

	bool Open(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		if (!pAudioEnumerator->GetDeviceByUUID(DeviceType, pUUID, pThisEndpoint)) return false;
		if (!pThisEndpoint->Open(DelayTime)) {
			_RELEASE(pThisEndpoint);
			return false;
		}

		if (DeviceType == RenderType) {
			void* pPointer = nullptr;
			pThisEndpoint->GetDevicePointer(pPointer);
			FreeAndRestoreVolumeShit(pPointer);
		}

		void* rawPtr = nullptr;
		pThisEndpoint->GetDevicePointer(rawPtr);
		pNotificationCallback->SetCurrentDevice(DeviceType, false, rawPtr);
		pThisEndpoint->SetCallback(pAudioCallback);
		return true;
	}

	bool Open(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		if (!pAudioEnumerator->GetDeviceById(DeviceType, DeviceId, pThisEndpoint)) return false;
		if (!pThisEndpoint->Open(DelayTime)) {
			_RELEASE(pThisEndpoint);
			return false;
		}

		if (DeviceType == RenderType) {
			void* pPointer = nullptr;
			pThisEndpoint->GetDevicePointer(pPointer);
			FreeAndRestoreVolumeShit(pPointer);
		}

		void* rawPtr = nullptr;
		pThisEndpoint->GetDevicePointer(rawPtr);
		pNotificationCallback->SetCurrentDevice(DeviceType, DeviceId == -1 ? true : false, rawPtr);
		pThisEndpoint->SetCallback(pAudioCallback);
		return true;
	}

	bool Restart(fr_i32 DeviceType, fr_f32 DelayTime) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		if (pThisEndpoint) pThisEndpoint->Close();
		return Open(DeviceType, DelayTime);
	}

	bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, char* pUUID) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		pThisEndpoint->Close();
		return Open(DeviceType, DelayTime, pUUID);
	}

	bool Restart(fr_i32 DeviceType, fr_f32 DelayTime, fr_i32 DeviceId) override
	{
		IAudioEndpoint** ppThisEndpoint = ((DeviceType == RenderType) ? &pOutputEndpoint : DeviceType == CaptureType ? &pInputEndpoint : nullptr);
		if (!ppThisEndpoint) return false;
		IAudioEndpoint*& pThisEndpoint = *ppThisEndpoint;

		pThisEndpoint->Close();
		return Open(DeviceType, DelayTime, DeviceId);
	}

	bool Start() override
	{
		if (pInputEndpoint) if (!pInputEndpoint->Start()) return false;
		if (pOutputEndpoint) if (!pOutputEndpoint->Start()) return false;
		return true;
	}

	bool Stop() override
	{
		if (pInputEndpoint) if (!pInputEndpoint->Stop()) return false;
		if (pOutputEndpoint) if (!pOutputEndpoint->Stop()) return false;
		return true;
	}

	void SetVolume(fr_f32 VolumeLevel) override
	{
		if (pAudioVolume) pAudioVolume->SetVolume(VolumeLevel);
	}

	void GetVolume(fr_f32& VolumeLevel) override
	{
		if (pAudioVolume) pAudioVolume->GetVirtualVolume(VolumeLevel);
	}

	void GetEndpointInfo(fr_i32 DeviceType, EndpointInformation& endpointInfo) override
	{
		switch (DeviceType)
		{
		case CaptureType:
			if (pInputEndpoint) pInputEndpoint->GetDeviceInfo(endpointInfo);
			break;
		case RenderType:
		default:
			if (pOutputEndpoint) pOutputEndpoint->GetDeviceInfo(endpointInfo);
			break;
		}
	}

	void GetRawPtr(fr_i32 DeviceType, void*& OutPtr) override
	{
		switch (DeviceType)
		{
		case CaptureType:
			if (pInputEndpoint) pInputEndpoint->GetDevicePointer(OutPtr);
			break;
		case RenderType:
		default:
			if (pOutputEndpoint) pOutputEndpoint->GetDevicePointer(OutPtr);
			break;
		}
	}

	bool Close() override
	{
		if (pInputEndpoint) if (!pInputEndpoint->Close()) return false;
		if (pOutputEndpoint) if (!pOutputEndpoint->Close()) return false;
		return true;
	}

};
