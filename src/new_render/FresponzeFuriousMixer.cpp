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
#include "FresponzeFuriousMixer.h"
#include "FresponzeWavFile.h"
#include "FresponzeOpusFile.h"
#include "FresponzeMasterEmitter.h"
#include "FresponzeResonanceEmitter.h"

#define RING_BUFFERS_COUNT 2

CFuriousMixer::CFuriousMixer()
{
	AddRef();
}

CFuriousMixer::~CFuriousMixer()
{
	FreeStuff();
}

void
CFuriousMixer::FreeStuff()
{
	ListenersNode* pNode = pFirstListener;
	while (pNode) {
		ListenersNode* pNextNode = pNode->pNext;
		_RELEASE(pNode->pListener);
		pNode = pNextNode;
	}
}

bool
CFuriousMixer::SetNewFormat(PcmFormat fmt)
{
	int counter = 0;
	ListenersNode* pNode = pFirstListener;
	while (pNode) {
		if (pNode->pListener) pNode->pListener->SetFormat(fmt);
		pNode = pNode->pNext;
		counter++;
	}

	return !!counter;
}

bool
CFuriousMixer::SetMixFormat(PcmFormat& NewFormat)
{
	SetNewFormat(NewFormat);
	SetBufferSamples(NewFormat.Frames);
	MixFormat = NewFormat;
	return true;
}

bool
CFuriousMixer::GetMixFormat(PcmFormat& ThisFormat)
{
	ThisFormat = MixFormat;
	return true;
}

bool
CFuriousMixer::CreateNode(ListenersNode*& pNode)
{
	ListenersNode* pCurrent = pLastListener;
	if (!pCurrent) {
		pLastListener = new ListenersNode;
		pFirstListener = pLastListener;
	}
	else {
		pLastListener->pNext = new ListenersNode;
		pLastListener->pNext->pPrev = pLastListener;
		pLastListener = pLastListener->pNext;
	}

	pNode = pLastListener;
	return true;
}

bool
CFuriousMixer::DeleteNode(ListenersNode* pNode)
{
	ListenersNode* pCurrent = pLastListener;
	if (!pNode) return false;
	while (pCurrent) {
		if (pCurrent == pNode) {
			if (pLastListener == pCurrent) pLastListener = pCurrent->pPrev;
			if (pFirstListener == pCurrent) pFirstListener = pCurrent->pNext;
			if (pCurrent->pPrev) pCurrent->pPrev->pNext = pCurrent->pNext;
			if (pCurrent->pNext) pCurrent->pNext->pPrev = pCurrent->pPrev;
			_RELEASE(pCurrent->pListener);
			delete pCurrent;
			return true;
		}
		pCurrent = pCurrent->pPrev;
	}

	return false;
}

// The MSVC compiler don't want to inline this function, so a rename it with 'D' prefix
inline
void*
GetDFormatListener(char* pListenerOpenLink)
{
	if (!strcmp(GetFilePathFormat(pListenerOpenLink), ".wav")) return new CRIFFMediaResource();
#ifdef FRESPONZE_USE_OPUS
	if (!strcmp(GetFilePathFormat(pListenerOpenLink), ".opus")) return new COpusMediaResource();
#endif
	return nullptr;
}

bool
CFuriousMixer::AddEmitterToListener(ListenersNode* pListener, IBaseEmitter* pEmmiter)
{
	PcmFormat tempFormat = {};
	if (!pListener->pListener->AddEmitter(pEmmiter)) return false;
	pEmmiter->SetListener(pListener->pListener);

	pListener->pListener->GetFormat(tempFormat);
	pEmmiter->SetFormat(&tempFormat);

	return true;
}

bool
CFuriousMixer::DeleteEmitterFromListener(ListenersNode* pListener, IBaseEmitter* pEmmiter)
{
	if (!pListener->pListener->DeleteEmitter(pEmmiter)) return false;
	pEmmiter->SetListener(nullptr);
	return true;
}

bool
CFuriousMixer::CreateListener(void* pListenerOpenLink, ListenersNode*& pNewListener, PcmFormat ListFormat)
{
	if (!ListFormat.Bits) ListFormat = MixFormat;

	IMediaResource* pNewResource = (IMediaResource*)GetDFormatListener((char*)pListenerOpenLink);
	if (!pNewResource) return false;
	if (!pNewResource->OpenResource(pListenerOpenLink)) {
		_RELEASE(pNewResource);
		return false;
	}

	if (ListFormat.Bits) pNewResource->SetFormat(ListFormat);
	CreateNode(pNewListener);
	pNewListener->pListener = new CMediaListener(pNewResource);
	pNewListener->pListener->SetFormat(ListFormat);
	return true;
}

bool
CFuriousMixer::DeleteListener(ListenersNode* pListNode)
{
	return DeleteNode(pListNode);
}

bool
CFuriousMixer::CreateEmitter(IBaseEmitter*& pEmitterToCreate, fr_i32 Type)
{
	switch (Type)
	{
	case 0: pEmitterToCreate = GetAdvancedEmitter(); break;
	case 1: pEmitterToCreate = GetSteamEmitter(); break;
	case 2: pEmitterToCreate = GetResonanceEmitter(); break;
	default:
		break;
	}

	return true;
}

bool
CFuriousMixer::Record(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	return false;
}

bool
CFuriousMixer::Update(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	fr_i32 UpdatedSamples = 0;
	if (RingBuffer.GetLeftBuffers() <= 0) {
		/* No buffers in queue - no data. Render it */
		if (!Render(BufferedSamples, Channels, SampleRate)) return false;
	}

	fr_i32 ret = RingBuffer.ReadData(pBuffer, Frames * Channels);
	UpdatedSamples += ret / Channels;
	/*
		If we can't read more data because WASAPI or other output send
		non-typical samples count query to ring buffer, we must render
		data to push new data to required buffer with other write position
	*/
	if (UpdatedSamples < Frames) {
		if (!Render(BufferedSamples, Channels, SampleRate)) return false;
		ret = RingBuffer.ReadData(&pBuffer[ret], (Frames - UpdatedSamples) * Channels);
		UpdatedSamples += ret / Channels;
		if (UpdatedSamples < Frames) {
			return false;
		}
	}

	return true;
}

bool
CFuriousMixer::Render(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	ListenersNode* pListNode = pFirstListener;
	PcmFormat fmt = { };



	return true;
}

bool
CFuriousMixer::Flush()
{
	return true;
}
