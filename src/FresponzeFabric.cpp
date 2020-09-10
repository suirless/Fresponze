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
#include "Fresponze.h"
#ifdef WINDOWS_PLATFORM
#include "FresponzeWasapiHardware.h"
#else
#include <stdarg.h>
#endif
#include "FresponzeAdvancedMixer.h"

void* hModule = nullptr;

void 
TypeToLogFormated(const char* Text, ...)
{
	static fr_string1k outputString = {};
	static fr_wstring1k woutputString = {};
	va_list arg;
	int done;

#ifndef WINDOWS_PLATFORM
	va_start(arg, Text);
	vsprintf(outputString, Text, arg);
	TypeToLog(outputString);
	va_end(arg);
#else
	va_start(arg, Text);
	vsprintf(outputString, Text, arg);
	if (MultiByteToWideChar(CP_UTF8, 0, outputString, -1, woutputString, sizeof(woutputString))) {
		TypeToLogW(woutputString);
	} else {
		TypeToLog(outputString);
	}
	va_end(arg);
#endif
}

#ifdef WINDOWS_PLATFORM
void
TypeToLogW(const wchar_t* Text)
{
	static fr_wstring1k outputString = {};
	GetDebugTimeW(outputString, sizeof(outputString));
	wprintf_s(L"%s%s\n", outputString, Text);

#ifdef _DEBUG
	wcscat_s(outputString, Text);
	OutputDebugStringW(outputString);
	OutputDebugStringW(L"\n");
#endif
}
#endif

void
TypeToLog(const char* Text)
{
	static fr_string1k outputString = {};
	GetDebugTime(outputString, sizeof(outputString));
	printf("%s%s\n", outputString, Text);
#ifdef _DEBUG
#ifdef WINDOWS_PLATFORM
	strcat_s(outputString, Text);
	OutputDebugStringA(outputString);
	OutputDebugStringA("\n");
#else
	printf("%s%s\n", outputString, Text);
#endif
#endif
}

void
TypeToLog(long long count)
{
	static fr_string1k outputString = {};
	static fr_string64 countString = {};
	GetDebugTime(outputString, sizeof(outputString));

#ifdef WINDOWS_PLATFORM
	_i64toa(count, countString, 10);
	strcat_s(outputString, countString);
	OutputDebugStringA(outputString);
	OutputDebugStringA("\n");
#else
	printf("%s%s\n", outputString, countString);
#endif
}

void
CFresponze::GetHardwareInterface(
	EndpointType endpointType, 
	void* pCustomCallback,
	void** ppHardwareInterface
)
{
	switch (endpointType)
	{
	case eEndpointNoneType:
	case eEndpointWASAPIType:
#ifdef WINDOWS_PLATFORM
		*ppHardwareInterface = (void*)(new CWASAPIAudioHardware((IAudioCallback*)pCustomCallback));
		break;
#endif
	case eEndpointXAudio2Type:
#if 0
		*ppHardwareInterface = (void*)(new CXAudio2AudioHardware((IAudioCallback*)pCustomCallback));
#endif
		break;
	default:
		break;
	}
}

void 
CFresponze::GetMixerInterface(
	MixerType mixerType,
	void** ppMixerInterface
)
{
	switch (mixerType)
	{
	case eMixerNoneType:
		break;
	case eMixerGameType:
		break;
	case eMixerAdvancedType:
		*ppMixerInterface = new CAdvancedMixer();
		break;
	case eMixerCustomType:
		break;
	default:
		break;
	}
}

void* 
AllocateInstance()
{
	return new CFresponze;
}

void 
DeleteInstance(
	void* pInstance
)
{
	CFresponze* pFresponze = (CFresponze*)pInstance;
	_RELEASE(pFresponze);
}

extern "C"
{
	fr_err
	FRAPI
	FrInitializeInstance(
		void** ppOutInstance
	)
	{
		if (!Fresponze::InitMemory()) return -1;
#if defined(WINDOWS_PLATFORM) && defined(DLL_PLATFORM)
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&hModule, (HMODULE*)&hModule);
#endif
		*ppOutInstance = AllocateInstance();
		if (!*ppOutInstance) return -1;

		return 0;
	}

	fr_err 
	FRAPI
	FrDestroyInstance(
		void* pInstance
	)
	{
		Fresponze::DestroyMemory();
		DeleteInstance(pInstance);
		return 0;
	}

	fr_err
	FRAPI
	FrGetRemoteInterface(
		void** ppRemoteInterface
	)
	{
		return -1;
	}
} 
