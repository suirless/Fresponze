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

class IAudioVolume : public IBaseInterface
{
protected:
	fr_f32 VirtualVolume = 0.f;

public:
	virtual bool GetVolume(fr_f32& fVolume) = 0;
	virtual bool GetVirtualVolume(fr_f32& fVolume) = 0;
	virtual bool SetVirtualVolume(fr_f32 fVolume) = 0;
	virtual bool SetVolume(fr_f32 fVolume) = 0;
	virtual bool IsMuted() = 0;
	virtual bool Mute(bool bState) = 0;
};
