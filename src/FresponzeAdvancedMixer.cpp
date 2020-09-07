/*********************************************************************
* Copyright (C) Anton Kovalev (vertver), 2020. All rights reserved.
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
#include "FresponzeAdvancedMixer.h"
#include "FresponzeWavFile.h"
#include "FresponzeOpusFile.h"
#include "FresponzeMasterEmitter.h"

#define RING_BUFFERS_COUNT 2

CAdvancedMixer::CAdvancedMixer()
{
	AddRef();
}

CAdvancedMixer::~CAdvancedMixer()
{
	FreeStuff();
}

void
CAdvancedMixer::FreeStuff()
{
	ListenersNode* pNode = pFirstListener;
	while (pNode) {
		ListenersNode* pNextNode = pNode->pNext;
		_RELEASE(pNode->pListener);
		pNode = pNextNode;
	}
}

bool
CAdvancedMixer::SetNewFormat(PcmFormat fmt)
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
CAdvancedMixer::SetMixFormat(PcmFormat& NewFormat)
{
	if (!SetNewFormat(NewFormat)) return false;
	SetBufferSamples(NewFormat.Frames);
	MixFormat = NewFormat;
	return true;
}

bool
CAdvancedMixer::GetMixFormat(PcmFormat& ThisFormat)
{
	ThisFormat = MixFormat;
	return true;
}

bool
CAdvancedMixer::CreateNode(ListenersNode*& pNode)
{
	ListenersNode* pCurrent = pLastListener;
	if (!pCurrent) {
		pLastListener = new ListenersNode;
		pFirstListener = pLastListener;
	} else {
		pLastListener->pNext = new ListenersNode;
		pLastListener->pNext->pPrev = pLastListener;
		pLastListener = pLastListener->pNext;
	}

	pNode = pLastListener;
	return true;
}

bool
CAdvancedMixer::DeleteNode(ListenersNode* pNode)
{
	ListenersNode* pCurrent = pLastListener;
	if (!pNode) return false;
	while (pCurrent) {
		if (pCurrent == pNode) {
			if (pLastListener == pCurrent) pLastListener = pCurrent->pPrev;
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

void*
GetFormatListener(char* pListenerOpenLink)
{
	if (!strcmp(GetFilePathFormat(pListenerOpenLink), ".wav")) return new CRIFFMediaResource();
	if (!strcmp(GetFilePathFormat(pListenerOpenLink), ".opus")) return new COpusMediaResource();
	return nullptr;
} 

bool 
CAdvancedMixer::AddEmitterToListener(ListenersNode* pListener, IBaseEmitter* pEmmiter)
{
	if (!pListener->pListener->AddEmitter(pEmmiter)) return false;
	pEmmiter->SetListener(pListener->pListener);
	return true;
}

bool 
CAdvancedMixer::DeleteEmitterFromListener(ListenersNode* pListener, IBaseEmitter* pEmmiter)
{
	if (!pListener->pListener->DeleteEmitter(pEmmiter)) return false;
	pEmmiter->SetListener(nullptr);
	return true;
}

bool
CAdvancedMixer::CreateListener(void* pListenerOpenLink, ListenersNode*& pNewListener, PcmFormat ListFormat)
{
	if (!ListFormat.Bits) ListFormat = MixFormat;

	IMediaResource* pNewResource = (IMediaResource*)GetFormatListener((char*)pListenerOpenLink);
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
CAdvancedMixer::DeleteListener(ListenersNode* pListNode)
{
	return DeleteNode(pListNode);
}

bool
CAdvancedMixer::CreateEmitter(IBaseEmitter*& pEmitterToCreate, fr_i32 Type)
{
	switch (Type)
	{
	case 0: pEmitterToCreate = GetAdvancedEmitter(); break;
	case 1: pEmitterToCreate = GetSteamEmitter(); break;
	default:
		break;
	}

	return true;
}

bool
CAdvancedMixer::Record(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	return false;
}

bool
CAdvancedMixer::Update(fr_f32* pBuffer, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
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

/*  Test function for generating float sin (300 to 600 Hz)
	static bool state = false;
	static fr_f32 phase = 0.f;
	static fr_f32 freq = 150.f;
	fr_f32* pBuf = (fr_f32*)pByte;
	for (size_t i = 0; i < AvailableFrames * CurrentChannels; i++) {
		if (freq >= 600.f / CurrentChannels) state = !state;
		pBuf[i] = sinf(phase * 6.283185307179586476925286766559005f) * 0.1f;
		phase = fmodf(phase + freq / SampleRate, 1.0f);
		freq = !state ? freq + 0.001f : freq - 0.001f;
		if (freq <= 300.f / CurrentChannels) state = !state; 
	}
*/

bool
CAdvancedMixer::Render(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate)
{
	ListenersNode* pListNode = pFirstListener;
	PcmFormat fmt = { };

	if (!pFirstListener) return false;
	/* Update buffer size if output endpoint change sample rate/bitrate/*/
	if (RingBuffer.GetLeftBuffers()) return false;
	RingBuffer.SetBuffersCount(RING_BUFFERS_COUNT);
	RingBuffer.Resize(Frames * Channels);
	OutputBuffer.Resize(Frames * Channels);
	tempBuffer.Resize(Channels, Frames);
	mixBuffer.Resize(Channels, Frames);

	/*	#############################################	
		OPT 001: Optimization issue with 'for' cycle.
		Priority: Medium

		We can process the signal only once, rather than every cycle time.
		This can increase perfomance by 10-70%, because in this case
		we don't seek the file pointer.

		Current state: processing
		############################################# 
	*/
	for (size_t i = 0; i < RING_BUFFERS_COUNT; i++) {
		tempBuffer.Clear();
		mixBuffer.Clear();
		pListNode = pFirstListener;
		while (pListNode) {
			/* Source restart issue  */
			EmittersNode* pEmittersNode = nullptr;
			if (!pListNode->pListener) break;
			pListNode->pListener->GetFirstEmitter(&pEmittersNode);
			while (pEmittersNode) {
				tempBuffer.Clear();
				pEmittersNode->pEmitter->Process(tempBuffer.GetBuffers(), Frames);
				for (size_t o = 0; o < Channels; o++) {
					MixerAddToBuffer(mixBuffer.GetBufferData((fr_i32)o), tempBuffer.GetBufferData((fr_i32)o), Frames);
				}

				pEmittersNode = pEmittersNode->pNext;
			}

			pListNode = pListNode->pNext;
		}

		/* Update ring buffer state for pushing new data */
		PlanarToLinear(mixBuffer.GetBuffers(), OutputBuffer.Data(), Frames * Channels, Channels);
		RingBuffer.PushBuffer(OutputBuffer.Data(), Frames * Channels);
		RingBuffer.NextBuffer();
	}

	return true;
}

bool 
CAdvancedMixer::Flush()
{
	return true;
}