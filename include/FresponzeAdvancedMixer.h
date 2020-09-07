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
#include "FresponzeMixer.h"
#include "FresponzeListener.h"

class CAdvancedMixer : public IAdvancedMixer
{
protected:
	fr_i32 BufferedSamples = 0;
	ListenersNode* pFirstListener = nullptr;
	ListenersNode* pLastListener = nullptr;

	void FreeStuff();
	bool SetNewFormat(PcmFormat fmt);

	bool CreateNode(ListenersNode*& pNode);
	bool DeleteNode(ListenersNode* pNode);

public:
	CAdvancedMixer();
	~CAdvancedMixer();
	void SetBufferSamples(fr_i32 SamplesIn) override { BufferedSamples = SamplesIn; }

	bool SetMixFormat(PcmFormat& NewFormat) override;
	bool GetMixFormat(PcmFormat& ThisFormat) override;

	bool Record(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) override;
	bool Update(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) override;
	bool Render(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) override;
	bool Flush() override;

	bool AddEmitterToListener(ListenersNode* pListener, IBaseEmitter* pEmmiter) override;
	bool DeleteEmitterFromListener(ListenersNode* pListener, IBaseEmitter* pEmmiter) override;

	bool CreateListener(void* pListenerOpenLink /* local or internet link */, ListenersNode*& pNewListener, PcmFormat ListFormat = {}) override;
	bool DeleteListener(ListenersNode* pListNode) override;

	bool CreateEmitter(IBaseEmitter*& pEmitterToCreate, fr_i32 Type) override;
};