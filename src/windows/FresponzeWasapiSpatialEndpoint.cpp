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
#ifdef WASAPI_USE_SPATIAL_AUDIO
#include <process.h>
#include <avrt.h>

#ifndef GUID_SECT
#define GUID_SECT
#endif

#define __FRDEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const GUID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_IID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const IID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define __FRDEFINE_CLSID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const CLSID n GUID_SECT = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
#define FRDEFINE_CLSID(className, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_CLSID(FR_CLSID_##className, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)
#define FRDEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    __FRDEFINE_IID(FR_IID_##interfaceName, 0x##l, 0x##w1, 0x##w2, 0x##b1, 0x##b2, 0x##b3, 0x##b4, 0x##b5, 0x##b6, 0x##b7, 0x##b8)

FRDEFINE_IID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 00000003, 0000, 0010, 80, 00, 00, aa, 00, 38, 9b, 71);
FRDEFINE_IID(KSDATAFORMAT_SUBTYPE_PCM, 00000001, 0000, 0010, 80, 00, 00, aa, 00, 38, 9b, 71);
#define maxmin(a, minimum, maximum)  min(max(a, minimum), maximum)

PROPERTYKEY FFRPKEY_Device_FriendlyName = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 };

const AudioObjectType ChannelMask_Mono = AudioObjectType_FrontCenter;
const AudioObjectType ChannelMask_Stereo = (AudioObjectType)(AudioObjectType_FrontLeft | AudioObjectType_FrontRight);
const AudioObjectType ChannelMask_2_1 = (AudioObjectType)(ChannelMask_Stereo | AudioObjectType_LowFrequency);
const AudioObjectType ChannelMask_Quad = (AudioObjectType)(AudioObjectType_FrontLeft | AudioObjectType_FrontRight | AudioObjectType_BackLeft | AudioObjectType_BackRight);
const AudioObjectType ChannelMask_4_1 = (AudioObjectType)(ChannelMask_Quad | AudioObjectType_LowFrequency);
const AudioObjectType ChannelMask_5_1 = (AudioObjectType)(AudioObjectType_FrontLeft | AudioObjectType_FrontRight | AudioObjectType_FrontCenter | AudioObjectType_LowFrequency | AudioObjectType_SideLeft | AudioObjectType_SideRight);
const AudioObjectType ChannelMask_7_1 = (AudioObjectType)(ChannelMask_5_1 | AudioObjectType_BackLeft | AudioObjectType_BackRight);

const UINT32 MaxStaticObjectCount_7_1_4 = 12;
const AudioObjectType ChannelMask_7_1_4 = (AudioObjectType)(ChannelMask_7_1 | AudioObjectType_TopFrontLeft | AudioObjectType_TopFrontRight | AudioObjectType_TopBackLeft | AudioObjectType_TopBackRight);

const UINT32 MaxStaticObjectCount_7_1_4_4 = 16;
const AudioObjectType ChannelMask_7_1_4_4 = (AudioObjectType)(ChannelMask_7_1_4 | AudioObjectType_BottomFrontLeft | AudioObjectType_BottomFrontRight | AudioObjectType_BottomBackLeft | AudioObjectType_BottomBackRight);

const UINT32 MaxStaticObjectCount_8_1_4_4 = 17;
const AudioObjectType ChannelMask_8_1_4_4 = (AudioObjectType)(ChannelMask_7_1_4_4 | AudioObjectType_BackCenter);

inline
DWORD
GetSleepTime(
	DWORD Frames,
	DWORD SampleRate
)
{
	float fRet = 0.f;
	if (!SampleRate) return 0;

	fRet = ((((float)Frames / (float)SampleRate) * 1000.f) / 4.f);
	return (DWORD)fRet;
}

inline
void
CopyDataToBuffer(
	float* FileData,
	void* OutData,
	size_t FramesCount,
	bool IsFloat,
	int Bits,
	int Channels
)
{
	size_t sizeToRead = FramesCount * Channels;

	if (IsFloat) {
		memcpy(OutData, FileData, sizeToRead * sizeof(fr_f32));
	}
	else {
		short* pShortData = (short*)FileData;
		switch (Bits) {
		case 16:
			for (size_t i = 0; i < sizeToRead; i++) {
				pShortData[i] = maxmin(((short)(FileData[i] * 32768.0f)), -32768, 32767);
			}
			break;
		default:
			break;
		}
	}
}

inline
void
CopyDataFromBuffer(
	void* FileData,
	float* OutData,
	size_t FramesCount,
	bool IsFloat,
	int Bits,
	int Channels
)
{
	size_t sizeToRead = FramesCount * Channels;

	if (IsFloat) {
		memcpy(OutData, FileData, sizeToRead * sizeof(fr_f32));
	}
	else {
		short* pShortData = (short*)FileData;
		switch (Bits) {
		case 16:
			for (size_t i = 0; i < sizeToRead; i++) {
				OutData[i] = (float)pShortData[i] < 0 ? pShortData[i] / 32768.0f : pShortData[i] / 32767.0f;
			}
			break;
		default:
			break;
		}
	}
}

void
CWASAPISpatialAudioEnpoint::GetDevicePointer(
	void*& pDevice
)
{
	pDevice = pCurrentDevice;
}

void
WINAPIV
WASAPISpatialThreadProc(
	void* pData
)
{
	CoInitialize(nullptr);
	CWASAPISpatialAudioEnpoint* pThis = (CWASAPISpatialAudioEnpoint*)pData;
	pThis->ThreadProc();
	CoUninitialize();
}

void
CWASAPISpatialAudioEnpoint::ThreadProc()
{
	if (!InitializeToPlay(DelayCustom)) return;
	PcmFormat fmtToPush = {};
	bool isFloat = EndpointInfo.EndpointFormat.IsFloat;
	fr_err errCode = 0;
	UINT32 CurrentFrames = 0;
	UINT32 SampleRate = EndpointInfo.EndpointFormat.SampleRate;
	UINT32 Bits = EndpointInfo.EndpointFormat.Bits;
	UINT32 FramesInBuffer = EndpointInfo.EndpointFormat.Frames;
	UINT32 CurrentChannels = EndpointInfo.EndpointFormat.Channels;
	DWORD dwTask = 0;
	DWORD dwFlags = 0;
	DWORD dwFlushTime = GetSleepTime(FramesInBuffer, (DWORD)SampleRate);
	HRESULT hr = 0;
	HANDLE hMMCSS = nullptr;

	switch (EndpointInfo.Type) {
	case ProxyType:
	case RenderType: {
		hMMCSS = AvSetMmThreadCharacteristicsA("Pro Audio", &dwTask);
		if (IsInvalidHandle(hMMCSS)) { TypeToLog("WASAPI: AvSetMmThreadCharacteristicsA() failed"); return; }
	}
	break;
	case CaptureType: {
		hMMCSS = AvSetMmThreadCharacteristicsA("Capture", &dwTask);
		if (IsInvalidHandle(hMMCSS)) { TypeToLog("WASAPI: AvSetMmThreadCharacteristicsA() failed"); return; }
	}
	break;
	default: {
		hMMCSS = AvSetMmThreadCharacteristicsA("Audio", &dwTask);
		if (IsInvalidHandle(hMMCSS)) { TypeToLog("WASAPI: AvSetMmThreadCharacteristicsA() failed"); return; }
	}
	break;
	}

	if (!AvSetMmThreadPriority(hMMCSS, AVRT_PRIORITY_CRITICAL)) {
		TypeToLog("WASAPI: AvSetMmThreadPriority() failed");
		goto EndOfThread;
	}

	pSyncEvent->Raise();
	pStartEvent->Wait();
	pThreadEvent->Reset();
	if (pAudioCallback) {
		pAudioCallback->FlushCallback();
		pAudioCallback->FormatCallback(&fmtToPush);
	}

	pObjectAudioStream->Start();
	while (!pThreadEvent->Wait(dwFlushTime)) {
		if (WaitForSingleObject(hCompleteEvent, 100) != WAIT_OBJECT_0) {
			hr = pObjectAudioStream->Reset();
			if (FAILED(hr)) {
				goto EndOfThread;
			}
		}

		if (!pAudioCallback) continue;

		UINT32 availableDynamicObjectCount = 0;
		UINT32 frameCount = 0;
		hr = pObjectAudioStream->BeginUpdatingAudioObjects(&availableDynamicObjectCount, &frameCount);

		BYTE* buffer = nullptr;
		UINT32 bufferLength = 0;

		if (pStaticObject == nullptr) {
			hr = pObjectAudioStream->ActivateSpatialAudioObject(ChannelMask_Mono, &pStaticObject);
			if (hr != S_OK) break;
		}

		// Get the buffer to write audio data
		hr = pStaticObject->GetBuffer(&buffer, &bufferLength);
		if (SUCCEEDED(hr)) {
			/* Process and copy data to main buffer */
			if (!buffer) continue;
			errCode = pAudioCallback->EndpointCallback((fr_f32*)buffer, frameCount, 1, (fr_i32)SampleRate, RenderType);

			if (FAILED(errCode)) { TypeToLog("WASAPI: Putting empty buffer to output"); }
		}
		else {
			/* Don't try to destroy device if the buffer is unavailable */
			if (hr == AUDCLNT_E_BUFFER_TOO_LARGE) {
				TypeToLog("WASAPI: Buffer is too large for endpoint buffer");
				continue;
			}

			TypeToLog("WASAPI: pRenderClient->GetBuffer() failed (render callback)");
			goto EndOfThread;
		}

		hr = pObjectAudioStream->EndUpdatingAudioObjects();
	}
	
EndOfThread:
	/* This functions must be called in thread, where you create service */
	_RELEASE(pObjectAudioStream);
	_RELEASE(pAudioClient);
	TypeToLog("WASAPI: Shutdowning thread");
	if (pThreadEvent->IsRaised()) pThreadEvent->Reset();
	if (pSyncEvent->IsRaised()) pSyncEvent->Reset();
	if (!IsInvalidHandle(hMMCSS)) AvRevertMmThreadCharacteristics(hMMCSS);
}

bool
CWASAPISpatialAudioEnpoint::CreateWasapiThread()
{
	DWORD dwThreadId = 0;

	if (pThreadEvent->IsRaised()) pThreadEvent->Reset();
#ifndef XBOX_BUILD
	hThread = (HANDLE)_beginthread(WASAPISpatialThreadProc, 0, this);
#else
	hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)WASAPIThreadProc, this, 0, &dwThreadId);
#endif

	if (!IsInvalidHandle(hThread)) {
		TypeToLog("WASAPI: Waiting for event");
		if (!pSyncEvent->Wait(2000)) {
			/* Terminate our thread if it was timeouted */
			if (WaitForSingleObject(hThread, 0) != WAIT_OBJECT_0) {
				TypeToLog("WASAPI: Failed to init thread");
				TerminateThread(hThread, (DWORD)-1);
			}

			return false;
		}
	}
	else return false;

	TypeToLog("WASAPI: Event triggered. Thread started");
	return true;
}

void
CWASAPISpatialAudioEnpoint::SetDeviceInfo(EndpointInformation& DeviceInfo)
{
	memcpy(&EndpointInfo, &DeviceInfo, sizeof(EndpointInformation));
}

void
CWASAPISpatialAudioEnpoint::GetDeviceInfo(EndpointInformation& DeviceInfo)
{
	memcpy(&DeviceInfo, &EndpointInfo, sizeof(EndpointInformation));
}

void
CWASAPISpatialAudioEnpoint::SetCallback(IAudioCallback* pCallback)
{
	pCallback->Clone((void**)&pAudioCallback);
}

bool
CWASAPISpatialAudioEnpoint::GetEndpointDeviceInfo()
{
	LPWSTR lpwDeviceId = nullptr;
	IPropertyStore* pPropertyStore = nullptr;
	PROPVARIANT value = { 0 };

	if (!pCurrentDevice) return false;
#ifndef XBOX_BUILD
	if (FAILED(pCurrentDevice->OpenPropertyStore(STGM_READ, &pPropertyStore))) {
		TypeToLog("WASAPI: pCurrentDevice->OpenPropertyStore() failed (endpoint)");
		return false;
	}

	/* Get friendly name of current device */
	PropVariantInit(&value);
	if (SUCCEEDED(pPropertyStore->GetValue(FFRPKEY_Device_FriendlyName, &value))) {
		if (value.vt == VT_LPWSTR) {
			/* Get size of new UTF-8 string */
			int StringSize = WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, -1, NULL, 0, NULL, NULL);
			char lpNewString[260] = {};

			/* Translate UTF-16 string to normalized UTF-8 string */
			if (StringSize && StringSize < 259) {
				if (WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, -1, lpNewString, 260, NULL, NULL)) {
					strcpy_s(EndpointInfo.EndpointName, lpNewString);
				}
			}
			else {
				strcpy_s(EndpointInfo.EndpointName, "Unknown Device Name");
			}
		}
	}
	else {
		TypeToLog("WASAPI: Failed to get friendly name");
		strcpy_s(EndpointInfo.EndpointName, "Unknown Device Name");
	}
	PropVariantClear(&value);

	/* Get UUID of current device */
	if (SUCCEEDED(pCurrentDevice->GetId(&lpwDeviceId))) {
		/* Get size of new UTF-8 string */
		int StringSize = WideCharToMultiByte(CP_UTF8, 0, lpwDeviceId, -1, NULL, 0, NULL, NULL);
		char lpNewString[260] = {};

		/* Translate UTF-16 string to normalized UTF-8 string */
		if (StringSize && StringSize < 259) {
			if (WideCharToMultiByte(CP_UTF8, 0, lpwDeviceId, -1, lpNewString, 260, NULL, NULL)) {
				strcpy_s(EndpointInfo.EndpointUUID, lpNewString);
			}
			else {
				strcpy_s(EndpointInfo.EndpointUUID, "Unknown Device UUID");
			}
		}
	}
	else {
		TypeToLog("WASAPI: Failed to get device UUID");
		strcpy_s(EndpointInfo.EndpointUUID, "Unknown Device UUID");
	}

	_RELEASE(pPropertyStore);
#endif
	return true;
}

bool
CWASAPISpatialAudioEnpoint::InitializeToPlay(fr_f32 Delay)
{
	DWORD dwStreamFlags = (AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);
	UINT32 BufferFrames = 0;
	HRESULT hr = 0;
	REFERENCE_TIME refTimeDefault = 0;
	REFERENCE_TIME refTimeMin = 0;
	REFERENCE_TIME refTimeAccepted = REFERENCE_TIME(Delay * 10000.f);
	WAVEFORMATEX* pWaveFormat = nullptr;
	WAVEFORMATEX waveFormat = {};
	IPropertyStore* pPropertyStore = nullptr;
	IAudioFormatEnumerator* pFormatEnumerator = nullptr;
	PROPVARIANT value = { 0 };

	if (!pCurrentDevice) return false;
	hr = pCurrentDevice->Activate(__uuidof(ISpatialAudioClient), CLSCTX_INPROC_SERVER, nullptr, (void**)&pAudioClient);
	if (FAILED(hr)) return false;

	if (!GetEndpointDeviceInfo()) {
		_RELEASE(pAudioClient);
		return false;
	}

	hr = pAudioClient->GetSupportedAudioObjectFormatEnumerator(&pFormatEnumerator);
	if (FAILED(hr)) {
		_RELEASE(pAudioClient);
		return false;
	}

	/* Check for needy format to activate device with it (default sample rate requested - 48000)*/
	UINT32 FormatsCount = 0;
	pFormatEnumerator->GetCount(&FormatsCount);
	for (UINT32 i = 0; i < FormatsCount; i++) {
		hr = pFormatEnumerator->GetFormat(i, &pWaveFormat);
		if (FAILED(hr)) {
			_RELEASE(pFormatEnumerator);
			_RELEASE(pAudioClient);
			return false;
		}

		/* We've got it! Continue to client execution*/
		if (pWaveFormat->nSamplesPerSec == RENDER_SAMPLE_RATE && pWaveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
			memcpy(&waveFormat, pWaveFormat, sizeof(WAVEFORMATEX));
			break;
		}
	}

	/* We don't find needy format, abort */
	if (waveFormat.nSamplesPerSec == 0) {
		_RELEASE(pFormatEnumerator);
		_RELEASE(pAudioClient);
		return false;
	}

	hr = pAudioClient->IsAudioObjectFormatSupported(&waveFormat);
	if (FAILED(hr)) {
		if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT) {
			TypeToLogFormated("WASAPI: Unsupported format to device: 0x%x", hr);
		}

		return false;
	}

	/* Create the event that will be used to signal the client for more data */
	hCompleteEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	/* Get current object type for set channels mask */
	hr = pAudioClient->GetNativeStaticObjectTypeMask(&typeOfObject);
	if (FAILED(hr)) {
		_RELEASE(pAudioClient);
		CloseHandle(hCompleteEvent);
		return false;
	}

	streamParams.ObjectFormat = &waveFormat;
	streamParams.StaticObjectTypeMask = ChannelMask_Mono;
	streamParams.MinDynamicObjectCount = 0;
	streamParams.MaxDynamicObjectCount = 0;
	streamParams.Category = AudioCategory_SoundEffects;
	streamParams.EventHandle = hCompleteEvent;
	streamParams.NotifyObject = nullptr;

	hr = pAudioClient->GetMaxFrameCount(&waveFormat, &BufferFrames);
	if (FAILED(hr)) {
		_RELEASE(pAudioClient);
		CloseHandle(hCompleteEvent);
		return false;
	}

	PropVariantInit(&activationParams);
	activationParams.vt = VT_BLOB;
	activationParams.blob.cbSize = sizeof(streamParams);
	activationParams.blob.pBlobData = reinterpret_cast<BYTE*>(&streamParams);
	hr = pAudioClient->ActivateSpatialAudioStream(&activationParams, __uuidof(pObjectAudioStream), (void**)&pObjectAudioStream);
	if (FAILED(hr)) {
		CloseHandle(hCompleteEvent);
		return false;
	}

	EndpointInfo.EndpointFormat.Frames = (fr_i32)BufferFrames;
	TypeToLogFormated("WASAPI: Device initialized (Shared, Sample rate: %i, Channels: %i, Latency: %i)", EndpointInfo.EndpointFormat.SampleRate, EndpointInfo.EndpointFormat.Channels, EndpointInfo.EndpointFormat.Frames);
	TypeToLogFormated("WASAPI: Device name: %s", EndpointInfo.EndpointName);
	TypeToLogFormated("WASAPI: Device GUID: %s", EndpointInfo.EndpointUUID);

	pTempBuffer = (fr_f32*)FastMemAlloc(sizeof(fr_f32) * EndpointInfo.EndpointFormat.Frames * EndpointInfo.EndpointFormat.Channels);
	CoTaskMemFree(pWaveFormat);
	return true;
}

bool
CWASAPISpatialAudioEnpoint::Open(fr_f32 Delay)
{
	DelayCustom = Delay;
	if (!Start()) return false;
	return true;
}

bool
CWASAPISpatialAudioEnpoint::Close()
{
	Stop();
	return true;
}

bool
CWASAPISpatialAudioEnpoint::Start()
{
	CreateWasapiThread();
	pStartEvent->Raise();
	return true;
}

bool
CWASAPISpatialAudioEnpoint::Stop()
{
	if (!IsInvalidHandle(hThread)) {
		pThreadEvent->Raise();
		pStartEvent->Reset();
		if (WaitForSingleObject(hThread, 1000) == WAIT_TIMEOUT) {
			TerminateThread(hThread, (DWORD)-1);
		}

		if (pTempBuffer) {
			FreeFastMemory(pTempBuffer);
			pTempBuffer = nullptr;
		}

		return true;
	}

	return false;
}
#endif