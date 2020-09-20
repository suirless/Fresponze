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
#include "FresponzeWasapi.h"

#define __FRDEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#undef FRDEFINE_CLSID
#undef FRDEFINE_IID
#define FRDEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_CLSID(FRR_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define FRDEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_IID(FRR_IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)

FRDEFINE_IID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 00000003, 0000, 0010, 80, 00, 00, aa, 00, 38, 9b, 71);
FRDEFINE_IID(KSDATAFORMAT_SUBTYPE_PCM, 00000001, 0000, 0010, 80, 00, 00, aa, 00, 38, 9b, 71);

extern const PROPERTYKEY FRRPKEY_Device_FriendlyName = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 };

CWASAPIAudioEnumerator::CWASAPIAudioEnumerator()
{
	AddRef();
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
}

CWASAPIAudioEnumerator::~CWASAPIAudioEnumerator()
{
	if (InputDevicesInfo) FreeFastMemory(InputDevicesInfo);
	if (OutputDevicesInfo) FreeFastMemory(OutputDevicesInfo);
	_RELEASE(pDeviceEnumerator);
}

bool
CWASAPIAudioEnumerator::GetDeviceInfoByDevice(
	EndpointInformation* pEndpointStruct,
	IMMDevice* pDevice
)
{
	LPWSTR lpwDeviceId = nullptr;
	IAudioClient* pAudioClient = nullptr;
	IPropertyStore* pPropertyStore = nullptr;
	WAVEFORMATEX* pWaveFormat = nullptr;
	PROPVARIANT value = { 0 };

	if (!pDevice) return false;
	if (FAILED(pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore))) return false;

	/* Get friendly name of current device */
	PropVariantInit(&value);
	if (SUCCEEDED(pPropertyStore->GetValue(FRRPKEY_Device_FriendlyName, &value))) {
		if (value.vt == VT_LPWSTR) {
			/* Get size of new UTF-8 string */
			int StringSize = WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, -1, NULL, 0, NULL, NULL);
			char lpNewString[260] = {};

			/* Translate UTF-16 string to normalized UTF-8 string */
			if (StringSize && StringSize < 259) {
				if (WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, -1, lpNewString, 260, NULL, NULL)) {
					strcpy_s(pEndpointStruct->EndpointName, lpNewString);
				}
			} else {
				strcpy_s(pEndpointStruct->EndpointName, "Unknown Device Name");
			}
		}
	} else {
		strcpy_s(pEndpointStruct->EndpointName, "Unknown Device Name");
	}
	PropVariantClear(&value);

	/* Get UUID of current device */
	if (SUCCEEDED(pDevice->GetId(&lpwDeviceId))) {
		/* Get size of new UTF-8 string */
		int StringSize = WideCharToMultiByte(CP_UTF8, 0, lpwDeviceId, -1, NULL, 0, NULL, NULL);
		char lpNewString[260] = {};

		/* Translate UTF-16 string to normalized UTF-8 string */
		if (StringSize && StringSize < 259) {
			if (WideCharToMultiByte(CP_UTF8, 0, lpwDeviceId, -1, lpNewString, 260, NULL, NULL)) {
				strcpy_s(pEndpointStruct->EndpointUUID, lpNewString);
			} else {
				strcpy_s(pEndpointStruct->EndpointUUID, "Unknown Device UUID");
			}
		}
	} else {
		strcpy_s(pEndpointStruct->EndpointUUID, "Unknown Device UUID");
	}

	if (FAILED(pDevice->Activate(__uuidof(pAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient))) {
		_RELEASE(pPropertyStore);
		return false;
	}

	if (FAILED(pAudioClient->GetMixFormat(&pWaveFormat))) {
		_RELEASE(pAudioClient);
		_RELEASE(pPropertyStore);
		return false;
	}

	/* Set local format struct */
	UINT32 BufferFrames = 0;
	pEndpointStruct->EndpointFormat.IsFloat = (pWaveFormat->wFormatTag == 3);
	pEndpointStruct->EndpointFormat.Bits = pWaveFormat->wBitsPerSample;
	pEndpointStruct->EndpointFormat.Channels = pWaveFormat->nChannels;
	pEndpointStruct->EndpointFormat.SampleRate = pWaveFormat->nSamplesPerSec;
	if (pWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		WAVEFORMATEXTENSIBLE* pTmp = (WAVEFORMATEXTENSIBLE*)pWaveFormat;
		pEndpointStruct->EndpointFormat.IsFloat = pTmp->SubFormat == FRR_IID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	}
	
	_RELEASE(pAudioClient);
	_RELEASE(pPropertyStore);
	CoTaskMemFree(pWaveFormat);
	return true;
}

bool
CWASAPIAudioEnumerator::OpenDeviceAndGetInfo(
	fr_i32 EndpointType, 
	wchar_t* pUUID, 
	fr_i32 DeviceId,
	EndpointInformation& EndpointStruct,
	IMMDevice*& pDevice
)
{
	EDataFlow flow = eRender;
	IMMDeviceCollection* pDeviceCollection = nullptr;

	if (!pDeviceEnumerator) return false;
	if (pUUID) {
		if (SUCCEEDED(pDeviceEnumerator->GetDevice(pUUID, &pDevice))) {
			if (GetDeviceInfoByDevice(&EndpointStruct, pDevice)) return true;
			_RELEASE(pDevice);
			return false;
		}
	}

	if (DeviceId != -1) {
		switch (EndpointType) {
		case RenderType:
			flow = eRender;
			break;
		case CaptureType:
			flow = eCapture;
			break;
		default:
			break;
		}

		if (FAILED(pDeviceEnumerator->EnumAudioEndpoints(flow, eConsole, &pDeviceCollection))) return false;
		if (SUCCEEDED(pDeviceCollection->Item(DeviceId, &pDevice))) {
			if (!GetDeviceInfoByDevice(&EndpointStruct, pDevice)) {
				_RELEASE(pDevice);
				_RELEASE(pDeviceCollection);
				return false;
			}

			_RELEASE(pDeviceCollection);
			return true;
		}
	}

	if (FAILED(pDeviceEnumerator->GetDefaultAudioEndpoint(flow, eConsole, &pDevice))) return false;
	if (!GetDeviceInfoByDevice(&EndpointStruct, pDevice)) {
		_RELEASE(pDevice);
		return false;
	}

	return true;

}

bool
CWASAPIAudioEnumerator::EnumerateDevices()
{
	UINT CountOfDevices = 0;
	HRESULT hr = 0;
	IMMDeviceCollection* pInputDeviceCollection = nullptr;
	IMMDeviceCollection* pOutputDeviceCollection = nullptr;

	if (!pDeviceEnumerator) return false;
	hr = pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pInputDeviceCollection);
	pDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pOutputDeviceCollection);
	if (!pOutputDeviceCollection && !pInputDeviceCollection) return false;

	if (pInputDeviceCollection) {
		if (SUCCEEDED(pInputDeviceCollection->GetCount(&CountOfDevices))) {	
			if (CountOfDevices > 0) {
				if (InputDevices != CountOfDevices) {
					InputDevices = CountOfDevices;
					if (InputDevicesInfo) FreeFastMemory(InputDevicesInfo);
					InputDevicesInfo = (EndpointInformation*)FastMemAlloc(sizeof(EndpointInformation) * (InputDevices + 1));
				}

				for (size_t i = 0; i < InputDevices; i++) {
					IMMDevice* pTempDevice = nullptr;
					if (FAILED(pInputDeviceCollection->Item((UINT)i, &pTempDevice))) continue;
					if (!GetDeviceInfoByDevice(&InputDevicesInfo[i], pTempDevice)) {
						_RELEASE(pTempDevice);
						_RELEASE(pInputDeviceCollection);
						_RELEASE(pOutputDeviceCollection);
						if (InputDevicesInfo) FreeFastMemory(InputDevicesInfo);
						if (OutputDevicesInfo) FreeFastMemory(OutputDevicesInfo);
						InputDevicesInfo = nullptr;
						OutputDevicesInfo = nullptr;
						return false;
					}

					_RELEASE(pTempDevice);
				}
			}
		}
	}

	if (pOutputDeviceCollection) {
		if (SUCCEEDED(pOutputDeviceCollection->GetCount(&CountOfDevices))) {
			if (CountOfDevices > 0) {
				if (OutputDevices != CountOfDevices) {
					OutputDevices = CountOfDevices;
					if (OutputDevicesInfo) FreeFastMemory(OutputDevicesInfo);
					OutputDevicesInfo = (EndpointInformation*)FastMemAlloc(sizeof(EndpointInformation) * (OutputDevices + 1));
				}

				for (size_t i = 0; i < OutputDevices; i++) {
					IMMDevice* pTempDevice = nullptr;
					if (FAILED(pOutputDeviceCollection->Item((UINT)i, &pTempDevice))) continue;
					if (!GetDeviceInfoByDevice(&OutputDevicesInfo[i], pTempDevice)) {
						_RELEASE(pTempDevice);
						_RELEASE(pInputDeviceCollection);
						_RELEASE(pOutputDeviceCollection);
						if (InputDevicesInfo) FreeFastMemory(InputDevicesInfo);
						if (OutputDevicesInfo) FreeFastMemory(OutputDevicesInfo);
						InputDevicesInfo = nullptr;
						OutputDevicesInfo = nullptr;
						return false;
					}

					_RELEASE(pTempDevice);
				}
			}
		}
	}

	_RELEASE(pInputDeviceCollection);
	_RELEASE(pOutputDeviceCollection);
	return true;
}

bool
CWASAPIAudioEnumerator::GetDevicesCount(fr_i32 EndpointType, fr_i32& Count)
{
	UINT CountOfDevices = 0;
	IMMDeviceCollection* pInputDeviceCollection = nullptr;
	IMMDeviceCollection* pOutputDeviceCollection = nullptr;
	if (!pDeviceEnumerator) return false;

	switch (EndpointType) {
		case RenderType:
			if (FAILED(pDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pOutputDeviceCollection))) return false;
			if (FAILED(pOutputDeviceCollection->GetCount(&CountOfDevices))) return false;
			_RELEASE(pOutputDeviceCollection);
			break;
		case CaptureType:
			if (FAILED(pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pInputDeviceCollection))) return false;
			if (FAILED(pInputDeviceCollection->GetCount(&CountOfDevices))) return false;
			_RELEASE(pInputDeviceCollection);
			break;
	default:
		break;
	}

	Count = CountOfDevices;
	return true;
}

void 
CWASAPIAudioEnumerator::GetInputDeviceList(EndpointInformation*& InputDevices)
{
	InputDevices = InputDevicesInfo;
}

void 
CWASAPIAudioEnumerator::GetOutputDeviceList(EndpointInformation*& OutputDevices)
{
	OutputDevices = OutputDevicesInfo;
}

bool
CWASAPIAudioEnumerator::GetDefaultDevice(fr_i32 EndpointType, IAudioEndpoint*& pOutDevice)
{
	IMMDevice* pTempDevice = nullptr;
	EndpointInformation endpointInfo = {};
	if (!pDeviceEnumerator) return false;

	if (!OpenDeviceAndGetInfo(EndpointType, nullptr, -1, endpointInfo, pTempDevice)) return false;
	pOutDevice = new CWASAPIAudioEnpoint(EndpointType, (void*)pTempDevice, endpointInfo);

	_RELEASE(pTempDevice);
	return true;
}

bool
CWASAPIAudioEnumerator::GetDeviceById(fr_i32 EndpointType, fr_i32 DeviceId, IAudioEndpoint*& pOutDevice)
{
	IMMDevice* pTempDevice = nullptr;
	EndpointInformation endpointInfo = {};
	if (!pDeviceEnumerator) return false;

	if (!OpenDeviceAndGetInfo(EndpointType, nullptr, DeviceId, endpointInfo, pTempDevice)) return false;
	pOutDevice = new CWASAPIAudioEnpoint(EndpointType, (void*)pTempDevice, endpointInfo);

	_RELEASE(pTempDevice);
	return true;
}

bool
CWASAPIAudioEnumerator::GetDeviceByUUID(fr_i32 EndpointType, char* DeviceUUID, IAudioEndpoint*& pOutDevice)
{
	fr_wstring512 szUUID = {};
	IMMDevice* pTempDevice = nullptr;
	EndpointInformation endpointInfo = {};
	if (!pDeviceEnumerator) return false;

	/* Get size of new UTF-8 string */
	int StringSize = MultiByteToWideChar(CP_UTF8, 0, DeviceUUID, -1, nullptr, 0);
	char lpNewString[260] = {};

	/* Translate UTF-16 string to normalized UTF-8 string */
	if (StringSize && StringSize < 511) {
		if (MultiByteToWideChar(CP_UTF8, 0, DeviceUUID, -1, szUUID, 512)) {
			if (!OpenDeviceAndGetInfo(EndpointType, szUUID, -1, endpointInfo, pTempDevice)) return false;
		} else {
			if (!OpenDeviceAndGetInfo(EndpointType, nullptr, -1, endpointInfo, pTempDevice)) return false;
		}
	} else {
		if (!OpenDeviceAndGetInfo(EndpointType, nullptr, -1, endpointInfo, pTempDevice)) return false;
	}

	pOutDevice = new CWASAPIAudioEnpoint(EndpointType, (void*)pTempDevice, endpointInfo);

	_RELEASE(pTempDevice);
	return true;
}
