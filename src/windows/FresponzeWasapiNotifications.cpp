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
#include "FresponzeWasapi.h"

const PROPERTYKEY FRPKEY_AudioEndpoint_PhysicalSpeakers = { { 0x1da5d803, 0xd492, 0x4edd, { 0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e } }, 3 };
const PROPERTYKEY FRPKEY_AudioEngine_DeviceFormat = { { 0xf19f064d, 0x82c, 0x4e27, { 0xbc, 0x73, 0x68, 0x82, 0xa1, 0xbb, 0x8e, 0x4c } }, 0 };
const PROPERTYKEY FRPKEY_AudioEngine_OEMFormat = { { 0xe4870e26, 0x3cc5, 0x4cd2, { 0xba, 0x46, 0xca, 0xa, 0x9a, 0x70, 0xed, 0x4} }, 3 };

HRESULT
STDMETHODCALLTYPE 
CWASAPIAudioNotification::OnDeviceStateChanged(
	LPCWSTR pwstrDeviceId, 
	DWORD dwNewState
)
{
	IMMDevice* pDevice = nullptr;
	if (dwNewState == DEVICE_STATE_DISABLED || dwNewState == DEVICE_STATE_UNPLUGGED || dwNewState == DEVICE_STATE_NOTPRESENT) {
		if (!pCallback->DeviceDisabled((void*)(pwstrDeviceId))) return E_FAIL;
		if (!pCallback->ListOfDevicesChanded()) return E_FAIL;
	}

	return S_OK;
}

HRESULT 
STDMETHODCALLTYPE 
CWASAPIAudioNotification::OnDeviceAdded(
	LPCWSTR pwstrDeviceId
)
{
	if (!pCallback->ListOfDevicesChanded()) return E_FAIL;
	return S_OK;
}

HRESULT 
STDMETHODCALLTYPE 
CWASAPIAudioNotification::OnDeviceRemoved(
	LPCWSTR pwstrDeviceId
)
{
	if (!pCallback->ListOfDevicesChanded()) return E_FAIL;
	return S_OK;
}

HRESULT 
STDMETHODCALLTYPE 
CWASAPIAudioNotification::OnDefaultDeviceChanged(
	EDataFlow flow,
	ERole role, 
	LPCWSTR pwstrDefaultDeviceId
)
{
	if (!pCallback->DefaultDeviceChanged(flow == eRender ? RenderType : flow == eCapture ? CaptureType : NoneType)) return E_FAIL;
	return S_OK;
}

HRESULT 
STDMETHODCALLTYPE 
CWASAPIAudioNotification::OnPropertyValueChanged(
	LPCWSTR pwstrDeviceId,
	const PROPERTYKEY key
)
{
	if (key.fmtid == FRPKEY_AudioEndpoint_PhysicalSpeakers.fmtid ||
		key.fmtid == FRPKEY_AudioEngine_DeviceFormat.fmtid ||
		key.fmtid == FRPKEY_AudioEngine_OEMFormat.fmtid) {
		if (!pCallback->ListOfDevicesChanded()) return E_FAIL;
	}

	return S_OK;
}
