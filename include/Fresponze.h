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
#include "FresponzeEndpoint.h"
#include "FresponzeEnumerator.h"
#include "FresponzeHardware.h"
#include "FresponzeAdvancedMixer.h"

#ifdef USE_FUNCS_PROTOTYPES
typedef fr_err(FrInitializeInstance_t)(void** ppInstance);
typedef fr_err(FrDestroyInstance_t)(void* pInstance);
typedef fr_err(FrGetRemoteInterface_t)(void** ppRemoteInterface);
#else
extern "C"
{
	fr_err FRAPI FrInitializeInstance(void** ppInstance);
	fr_err FRAPI FrDestroyInstance(void* pInstance);
	fr_err FRAPI FrGetRemoteInterface(void** ppRemoteInterface);
}
#endif

enum MixerType
{
	eMixerNoneType,
	eMixerGameType,
	eMixerAdvancedType,
	eMixerCustomType
};

enum EndpointType
{
	eEndpointNoneType,
	eEndpointWASAPIType,
	eEndpointXAudio2Type
};

struct FresponzeInformation
{
	fr_i32 FresponzeVersion = 0;
};

class IFresponzeRemote : public IBaseInterface
{
public:
	virtual void GetFresponzeInfo(FresponzeInformation* info) = 0;
};

class CFresponzeRemote : public IFresponzeRemote
{
public:
	CFresponzeRemote();
	virtual void GetFresponzeInfo(FresponzeInformation* info) = 0;
};

class IFresponze : public IBaseInterface
{
public:
	virtual void GetMixerInterface(MixerType mixerType, void** ppMixerInterface) = 0;
	virtual void GetHardwareInterface(EndpointType endpointType, void* pCustomCallback, void** ppHardwareInterface) = 0;
};


class CFresponze : public IFresponze
{
public:
	CFresponze() { AddRef(); }
	void GetMixerInterface(MixerType mixerType, void** ppMixerInterface) override;
	void GetHardwareInterface(EndpointType endpointType, void* pCustomCallback, void** ppHardwareInterface) override;
};
