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
#include "FresponzeMediaResource.h"
#include "FresponzeEffect.h"

enum EListenerState : fr_i32
{
	eStopState = 0,
	ePauseState,
	ePlayState,
	eReplayState
};

class IBaseEmitter : public IBaseEffect
{
protected:
	EffectNodeStruct* pFirstEffect = nullptr;
	EffectNodeStruct* pLastEffect = nullptr;
	void* pParentListener = nullptr;
	fr_i64 FilePosition = 0;

public:
	virtual void AddEffect(IBaseEffect* pNewEffect) = 0;
	virtual void DeleteEffect(IBaseEffect* pNewEffect) = 0;

	virtual void SetListener(void* pListener) = 0;
	virtual void SetState(fr_i32 state) = 0;
	virtual void SetPosition(fr_i64 FPosition) = 0;

	virtual void*  GetListener() = 0;
	virtual fr_i32 GetState() = 0;
	virtual fr_i64 GetPosition() = 0;
};

IBaseEmitter* GetAdvancedEmitter();

struct EmittersNode;
struct EmittersNode 
{
	EmittersNode* pNext = nullptr;
	EmittersNode* pPrev = nullptr;
	IBaseEmitter* pEmitter = nullptr;
};

class IMediaListener : public IBaseInterface
{
public:
	virtual bool AddEmitter(IBaseEmitter* pNewEmitter) = 0;
	virtual bool DeleteEmitter(IBaseEmitter* pEmitter) = 0;
	virtual bool GetFirstEmitter(EmittersNode** pFirstEmitter) = 0;

	virtual bool SetResource(IMediaResource* pInitialResource) = 0;

	virtual fr_i32 SetPosition(fr_f32 FloatPosition) = 0;		// 0.0f to 1.0f
	virtual fr_i32 SetPosition(fr_i64 FramePosition) = 0;		// 0 to x (end) 
	virtual fr_i64 GetPosition() = 0;

	virtual fr_i32 GetFullFrames() = 0;

	virtual fr_i32 GetFormat(PcmFormat& fmt) = 0;
	virtual fr_i32 SetFormat(PcmFormat fmt) = 0;

	virtual fr_i32 Process(fr_f32** ppOutputFloatData, fr_i32 frames) = 0;
};

class CMediaListener : public IMediaListener
{
protected:
	fr_i64 framesPos = 0;
	PcmFormat ResourceFormat = {};
	PcmFormat ListenerFormat = {};
	IMediaResource* pLocalResource = nullptr;
	EmittersNode* pFirstEmitter = nullptr;
	EmittersNode* pLastEmitter = nullptr;

	void FreeStuff();

public:
	/*
		Resource prototype must include constructor with initial media 
		resource interface (IMediaResource). It's can be either 
		internal Opus/Vorbis decoder or a custom decoder module
		from other DLL.
	*/
	CMediaListener(IMediaResource* pInitialResource = nullptr);
	~CMediaListener();

	bool AddEmitter(IBaseEmitter* pNewEmitter) override;
	bool DeleteEmitter(IBaseEmitter* pEmitter) override;
	bool GetFirstEmitter(EmittersNode** pFirstEmitter) override;

	bool SetResource(IMediaResource* pInitialResource) override;

	fr_i32 SetPosition(fr_f32 FloatPosition) override;		// 0.0f to 1.0f
	fr_i32 SetPosition(fr_i64 FramePosition) override;		// 0 to x (end) 
	fr_i64 GetPosition() override;

	fr_i32 GetFullFrames() override;

	fr_i32 GetFormat(PcmFormat& fmt) override;
	fr_i32 SetFormat(PcmFormat fmt) override;

	fr_i32 Process(fr_f32** ppOutputFloatData, fr_i32 frames) override;
};


struct ListenersNode;
struct ListenersNode
{
	ListenersNode* pNext = nullptr;
	ListenersNode* pPrev = nullptr;
	IMediaListener* pListener = nullptr;
};
