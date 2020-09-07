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
#include "FresponzeListener.h"

CMediaListener::CMediaListener(IMediaResource* pInitialResource)
{
	AddRef();
	pInitialResource->Clone((void**)&pLocalResource);
	pLocalResource->GetFormat(ResourceFormat);
	ListenerFormat = ResourceFormat;
}

CMediaListener::~CMediaListener()
{
	_RELEASE(pLocalResource);
	FreeStuff();
}

void
CMediaListener::FreeStuff()
{
	EmittersNode* pNode = pFirstEmitter;
	EmittersNode* pThisNode = nullptr;
	while (pNode) {
		pThisNode = pNode->pNext;
		_RELEASE(pNode->pEmitter);
		delete pNode;
		pNode = pThisNode;
	}
}

bool
CMediaListener::AddEmitter(IBaseEmitter* pNewEmitter)
{
	if (!pLastEmitter) {
		pFirstEmitter = new EmittersNode;
		memset(pFirstEmitter, 0, sizeof(EmittersNode));
		pLastEmitter = pFirstEmitter;
		pNewEmitter->Clone((void**)&pLastEmitter->pEmitter);
	} else {
		EmittersNode* pTemp = new EmittersNode;
		memset(pFirstEmitter, 0, sizeof(EmittersNode));
		pNewEmitter->Clone((void**)&pTemp->pEmitter);
		pLastEmitter->pNext = pTemp;
		pTemp->pPrev = pLastEmitter;
		pLastEmitter = pTemp;
	}

	return true;
}

bool 
CMediaListener::DeleteEmitter(IBaseEmitter* pEmitter)
{
	EmittersNode* pNode = nullptr;
	GetFirstEmitter(&pNode);
	while (pNode) {
		if (pNode->pEmitter == pEmitter) {
			pNode->pPrev->pNext = pNode->pNext;
			pNode->pNext->pPrev = pNode->pPrev;
			_RELEASE(pNode->pEmitter);
			delete pNode;
			return true;
		}
	}

	return false;
}

bool 
CMediaListener::GetFirstEmitter(EmittersNode** pFirstEmitter)
{
	*pFirstEmitter = this->pFirstEmitter;
	return true;
}

bool	
CMediaListener::SetResource(IMediaResource* pInitialResource)
{
	_RELEASE(pLocalResource);
	return pInitialResource->Clone((void**)&pLocalResource);
}

fr_i32	
CMediaListener::SetPosition(fr_f32 FloatPosition)
{
	return SetPosition(fr_i64(((fr_f64)ResourceFormat.Frames * fabs(FloatPosition))));
}

fr_i32	
CMediaListener::SetPosition(fr_i64 FramePosition)
{
	fr_i64 outputFrames = 0;
	CalculateFrames64(FramePosition, ListenerFormat.SampleRate, ResourceFormat.SampleRate, outputFrames);
	fr_i32 ret = pLocalResource->SetPosition(outputFrames);
	CalculateFrames64(ret, ResourceFormat.SampleRate, ListenerFormat.SampleRate, outputFrames);
	return outputFrames;
}

fr_i64
CMediaListener::GetPosition()
{
	fr_i64 outputFrames = pLocalResource->GetPosition();
	CalculateFrames64(outputFrames, ResourceFormat.SampleRate, ListenerFormat.SampleRate, outputFrames);
	return outputFrames;
}

fr_i32 
CMediaListener::GetFullFrames()
{
	fr_i64 outputFrames = 0;
	CalculateFrames64(ResourceFormat.Frames, ResourceFormat.SampleRate, ListenerFormat.SampleRate, outputFrames);
	return (fr_i32)outputFrames;
}

fr_i32	
CMediaListener::GetFormat(PcmFormat& fmt)
{
	fmt = ListenerFormat;
	return 0;
}

fr_i32
CMediaListener::SetFormat(PcmFormat fmt)
{
	EmittersNode* pProcessEmitter = pFirstEmitter;
	ListenerFormat = fmt;
	pLocalResource->SetFormat(ListenerFormat);
	pLocalResource->GetFormat(ResourceFormat);
	while (pProcessEmitter) {
		pProcessEmitter->pEmitter->SetFormat(&ListenerFormat);
		pProcessEmitter = pProcessEmitter->pNext;
	}
	return 0;
}

fr_i32	
CMediaListener::Process(fr_f32** ppOutputFloatData, fr_i32 frames)
{
	fr_i32 inFrames = 0;
	inFrames = (fr_i32)pLocalResource->Read(frames, ppOutputFloatData);
	framesPos = pLocalResource->GetPosition();
	return inFrames;
}
