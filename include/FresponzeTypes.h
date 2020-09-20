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
#pragma once
#include "FresponzeConfig.h"
#define _USE_MATH_DEFINES 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_CHANNELS 8


#ifdef WINDOWS_PLATFORM
#undef max
#undef min
#include <algorithm>
#define maxmin(a, minimum, maximum) std::min(std::max(a, minimum), maximum)
#endif

inline
float
dbtol(float Decibels)
{
	return powf(10.0f, Decibels / 20.0f);
}

inline
float
ltodb(float Volume)
{
	if (Volume == 0) return -3.402823466e+38f; 
	return 20.0f * log10f(Volume);
}

inline
double
fastatan(double x)
{
	return (x / (1.0 + 0.28 * (x * x)));
}

typedef signed char fr_i8;
typedef unsigned char fr_u8;
typedef signed short fr_i16;
typedef unsigned short fr_u16;
typedef signed int fr_i32;
typedef unsigned int fr_u32;
typedef signed long long fr_i64;
typedef unsigned long long fr_u64;
typedef float fr_f32;
typedef double fr_f64;

typedef signed char fr_s8;
typedef signed short fr_s16;
typedef signed int fr_s32;
typedef signed long long fr_s64;

typedef fr_u64 uint64;
typedef fr_i8 int8;
typedef fr_i16 int16;
typedef fr_i64 int64;
typedef fr_f32 r32;
typedef fr_f64 r64;
typedef fr_i32 b32;
typedef fr_f64 real64;
typedef fr_i32 fr_err;

typedef void* fr_ptr;
typedef char fr_utf8;
typedef fr_utf8* fr_PStr;
typedef fr_utf8 const* fr_PConstStr;
typedef fr_utf8 fr_string16[16];
typedef fr_utf8 fr_string32[32];
typedef fr_utf8 fr_string64[64];
typedef fr_utf8 fr_string128[128];
typedef fr_utf8 fr_string256[256];
typedef fr_utf8 fr_string512[512];
typedef fr_utf8 fr_string1k[1024];
typedef fr_utf8 fr_string2k[2048];
typedef fr_utf8 fr_string4k[4096];
typedef fr_utf8 fr_string8k[8192];

#ifndef NO_WIDECHAR
typedef wchar_t fr_utf16;
typedef fr_utf16* fr_PWStr;
typedef fr_utf16 const* fr_PWConstStr;
typedef fr_utf16 fr_wstring16[16];
typedef fr_utf16 fr_wstring32[32];
typedef fr_utf16 fr_wstring64[64];
typedef fr_utf16 fr_wstring128[128];
typedef fr_utf16 fr_wstring256[256];
typedef fr_utf16 fr_wstring512[512];
typedef fr_utf16 fr_wstring1k[1024];
typedef fr_utf16 fr_wstring2k[2048];
typedef fr_utf16 fr_wstring4k[4096];
typedef fr_utf16 fr_wstring8k[8192];
#endif

namespace Fresponze {
	bool InitMemory();
	void DestroyMemory();
}

void* FastMemAlloc(fr_i32 SizeToAllocate);
void* VirtMemAlloc(fr_i64 SizeToAllocate);
void FreeFastMemory(void* Ptr);
void FreeVirtMemory(void* Ptr, size_t Size);
void* FastMemRealloc(void* Ptr, fr_i32 SizeToAllocate);

inline
void*
FastMemTryRealloc(
	void* Ptr,
	fr_i32 SizeToAllocate,
	fr_i32 OldPtrSize
)
{
	void* temp_ptr = nullptr;
	if (Ptr) temp_ptr = FastMemRealloc(Ptr, SizeToAllocate);
	if (!temp_ptr) {
		temp_ptr = FastMemAlloc(SizeToAllocate);
		if (Ptr) {
			memcpy(temp_ptr, Ptr, OldPtrSize);
			FreeFastMemory(Ptr);
		}
	}

	return temp_ptr;
}

typedef struct
{
	fr_i32 top;
	fr_i32 left;
	fr_i32 bottom;
	fr_i32 right;
} SRect;
 
typedef struct
{
	fr_f32 x, y;
} Vec2;

typedef struct
{
	fr_f32 x, y, z;
} Vec3;

typedef struct
{
	fr_f32 x, y, z, w;
} Vec4;

typedef struct
{
	Vec4 mat[2];
} VMatrix4x2;

typedef struct
{
	Vec4 mat[3];
} VMatrix4x3;

typedef struct
{
	Vec4 mat[4];
} VMatrix4x4;

typedef struct
{
	Vec3 mat[2];
} VMatrix3x2;

typedef struct
{
	Vec3 mat[3];
} VMatrix3x3;

typedef struct
{
	Vec3 mat[4];
} VMatrix3x4;

typedef struct
{
	Vec2 mat[2];
} VMatrix2x2;

typedef struct
{
	Vec2 mat[3];
} VMatrix2x3;

typedef struct
{
	Vec2 mat[4];
} VMatrix2x4;

typedef enum
{
	ChannelUndefined = 0x7fffffff,
	ChannelMono = 0,
	ChannelLeft,
	ChannelRight,
	ChannelCenter,
	ChannelLFE,
	ChannelLeftSurround,
	ChannelRightSurround,
	ChannelLeftCenter,
	ChannelRightCenter,
	ChannelSurround,
	ChannelCenterSurround = ChannelSurround,
	ChannelSideLeft,
	ChannelSideRight,
	ChannelTopMiddle,
	ChannelTopFL,
	ChannelTopFC,
	ChannelTFR,
	ChannelTRL,
	ChannelTRC,
	ChannelTRR,
	ChannelLFE2
} BaseSpeakerType;

typedef enum
{
	ArrangementChannelsUserDefined = -2,
	ArrangementChannelsEmpty = -1,
	ArrangementChannelsMono = 0,
	ArrangementChannelsStereo,
	ArrangementChannelsStereoSurround,
	ArrangementChannelsStereoCenter,
	ArrangementChannelsStereoSide,
	ArrangementChannelsStereoCLfe,
	ArrangementChannels30Cine,
	ArrangementChannels30Music,
	ArrangementChannels31Cine,
	ArrangementChannels31Music,
	ArrangementChannels40Cine,
	ArrangementChannels40Music,
	ArrangementChannels41Cine,
	ArrangementChannels41Music,
	ArrangementChannels50,
	ArrangementChannels51,
	ArrangementChannels60Cine,
	ArrangementChannels60Music,
	ArrangementChannels61Cine,
	ArrangementChannels61Music,
	ArrangementChannels70Cine,
	ArrangementChannels70Music,
	ArrangementChannels71Cine,
	ArrangementChannels71Music,
	ArrangementChannels80Cine,
	ArrangementChannels80Music,
	ArrangementChannels81Cine,
	ArrangementChannels81Music,
	ArrangementChannels102,
	ArrangementChannelsNULL
} BaseSpeakerArrangementType;

typedef struct
{
	int ChannelsCount;
	int ArrangmentType;
	int SpeakersTypes[MAX_CHANNELS];		// REAPER can support 64 channels
} BaseSpeakerArrangement;

typedef enum
{
	CategoryUnknown = 0,
	CategoryEffect,
	CategorySynth,
	CategoryAnalysis,
	CategoryMastering,
	CategorySpacializer,
	CategoryRoomFx,
	CategorySurroundFx,
	CategoryRestoration,
	CategoryOfflineProcess,
	CategoryShell,
	CategoryGenerator,
	CategoryMaxCount
} BasePluginCategory;

typedef struct
{
	fr_i32 NoteLength;
	fr_i32 NoteOffset;
	fr_i8 Data[4];
	fr_i8 DetuneLevel;
	fr_i8 NoteOffVelocity;
} MIDIEvent;

typedef struct
{
	fr_i32 EventsCount;
	MIDIEvent* pEventsArray;
}  MIDIEvents;

typedef struct
{
	fr_i16 Index;
	fr_i16 Bits;
	fr_i16 IsFloat;
	fr_i16 Channels;
	fr_i32 Frames;
	fr_i32 SampleRate;
} PcmFormat;

typedef struct {
	fr_utf8 riff_header[4];
	fr_i32 wav_size;
	fr_utf8 wave_header[4];
	fr_utf8 fmt_header[4];
	fr_i32 fmt_chunk_size;
	fr_i16 audio_format;
	fr_i16 num_channels;
	fr_i32 sample_rate;
	fr_i32 byte_rate;
	fr_i16 sample_alignment;
	fr_i16 bit_depth;
	fr_utf8 data_header[4];
	fr_i32 data_bytes;
} wav_header;

enum ETypeEndpoint : fr_i32
{
	NoneType,
	RenderType,
	CaptureType,
	ProxyType
};

struct EndpointInformation
{
	fr_i32 Type;
	fr_i32 EndpointId;
	PcmFormat EndpointFormat;
	fr_string256 EndpointName;
	fr_string256 EndpointUUID;
};

/*
	ex.
	4410 buffer length - 100ms
	44100 hz samplerate

	4410 / 44100 = 0.1 s
	48000 * 0.1s = 4800 frames
*/
inline
void
CalculateFrames(
	fr_i32	FramesCount,
	fr_i32	InputSampleRate,
	fr_i32	OutputSampleRate,
	fr_i32& OutputFramesCount
)
{
	OutputFramesCount = FramesCount * OutputSampleRate / InputSampleRate;
}

inline
void
CalculateFrames64(
	fr_i64	FramesCount,
	fr_i64	InputSampleRate,
	fr_i64	OutputSampleRate,
	fr_i64& OutputFramesCount
)
{
	if (!InputSampleRate) return;
	OutputFramesCount = FramesCount * OutputSampleRate / InputSampleRate;;
}

template
<typename TYPE>
class CBuffer
{
private:
	fr_i32 DataSize = 0;
	TYPE* pLocalData = nullptr;

public:
	CBuffer() : DataSize(0), pLocalData(nullptr) {}
	CBuffer(fr_i32 SizeToResize)
	{
		pLocalData = (TYPE*)FastMemAlloc(SizeToResize * sizeof(TYPE));
		DataSize = SizeToResize;
	}

	CBuffer(TYPE* pData, fr_i32 SizeToResize)
	{
		pLocalData = (TYPE*)FastMemAlloc(SizeToResize * sizeof(TYPE));
		memcpy(pLocalData, pData, SizeToResize * sizeof(TYPE));
		DataSize = SizeToResize;
	}

	TYPE& operator[] (size_t index) 
	{ 
		if (index >= DataSize || index < 0) pLocalData[0];
		return pLocalData[index];
	}

	void Clear()
	{
		if (pLocalData) { memset(pLocalData, 0, DataSize * sizeof(TYPE)); }
	}


	void Free()
	{
		if (pLocalData) { FreeFastMemory(pLocalData); pLocalData = nullptr; }
		DataSize = 0;
	}

	void Resize(fr_i32 SizeToResize)
	{
		if (SizeToResize != DataSize) {
			pLocalData = (TYPE*)FastMemTryRealloc(pLocalData, SizeToResize * sizeof(TYPE), DataSize * sizeof(TYPE));
			DataSize = SizeToResize;
		}
	}

	void Push(TYPE* pData, fr_i32 SizeToResize)
	{
		Resize(SizeToResize);
		memcpy(pLocalData, pData, SizeToResize * sizeof(TYPE));
	}

	fr_i32 Size()
	{
		return DataSize;
	}

	TYPE* Data()
	{
		return pLocalData;
	}

	~CBuffer()
	{
		Free();
	}
};

struct ProcessBuffer {
	fr_u32 isQueueBuffer = 0;
	fr_u32 PositionRead = 0;
	fr_u32 PositionWrite = 0;
	fr_u32 Position = 0;
	fr_u32 Frames = 0;
	fr_f32* Data[MAX_CHANNELS] = {};
};

typedef CBuffer<fr_f64> CDoubleBuffer;
typedef CBuffer<fr_f32> CFloatBuffer;
typedef CBuffer<fr_i32> CIntBuffer;
typedef CBuffer<fr_i16> CShortBuffer;
typedef CBuffer<fr_i8>  CByteBufffer;

template
<typename TYPE>
class C2DBuffer
{
private:
	fr_i32 BuffersCount = 0;
	fr_i32 DataSize = 0;
	TYPE** pDoublePointer = nullptr;

public:
	C2DBuffer() {}

	TYPE& GetFrame(size_t index, size_t double_index)
	{
		if (index >= BuffersCount || index < 0) {
			index = 0;
		} 

		return pDoublePointer[index][double_index > DataSize ? DataSize : double_index];
	}

	TYPE*& operator[] (size_t index)
	{
		if (index >= BuffersCount || index < 0) return pDoublePointer[0];
		return pDoublePointer[index];
	}

	void Clear()
	{
		for (size_t i = 0; i < BuffersCount; i++) {
			if (pDoublePointer[i]) {
				memset(pDoublePointer[i], 0, DataSize * sizeof(TYPE)); 
			}
		}
	}

	void Free()
	{
		if (!pDoublePointer) return;
		for (size_t i = 0; i < BuffersCount; i++) if (pDoublePointer[i]) {
			pDoublePointer[i] = nullptr;
			FreeFastMemory(pDoublePointer[i]); 
		}
		FreeFastMemory(pDoublePointer);
	}

	void SetBuffersCount(fr_i32 NewBuffersCount)
	{
		TYPE** tempDoubleBuffer = (TYPE**)FastMemAlloc(sizeof(TYPE*) * (NewBuffersCount));
		if (pDoublePointer) {
			for (size_t i = 0; i < BuffersCount; i++) { 
				tempDoubleBuffer[i] = pDoublePointer[i]; 
			}
			FreeFastMemory(pDoublePointer);
			pDoublePointer = nullptr;
		} else {
			for (size_t i = 0; i < NewBuffersCount; i++) {
				tempDoubleBuffer[i] = nullptr;
			}
		}

		BuffersCount = NewBuffersCount;
		pDoublePointer = tempDoubleBuffer;
	}

	void Resize(fr_i32 NewBuffersCount, fr_i32 SizeToResize)
	{
		if (NewBuffersCount > BuffersCount) SetBuffersCount(NewBuffersCount);
		if (SizeToResize > DataSize) {
			for (size_t i = 0; i < BuffersCount; i++) { 
				pDoublePointer[i] = (TYPE*)FastMemTryRealloc(pDoublePointer[i], SizeToResize * sizeof(TYPE), DataSize * sizeof(TYPE));
			}
			DataSize = SizeToResize;
		}
	}

	void PushPacked(TYPE* pData, fr_i32 SizeToResize, fr_i32 NewBuffersCount)
	{
		if (SizeToResize > DataSize) Resize(NewBuffersCount, SizeToResize);
		for (size_t i = 0; i < SizeToResize * BuffersCount; i++) {
			pDoublePointer[i % NewBuffersCount][i / NewBuffersCount] = pData[i];
		}
	}

	void PushPacked(TYPE* pData, fr_i32 SizeToResize, fr_i32 NewBuffersCount, fr_i32 BufferPosition)
	{
		if (SizeToResize > DataSize) Resize(NewBuffersCount, SizeToResize);
		for (size_t i = BufferPosition; i < SizeToResize * BuffersCount; i++) {
			pDoublePointer[i % NewBuffersCount][i / NewBuffersCount] = pData[i - BufferPosition];
		}
	}

	void Push(TYPE* pData, fr_i32 SizeToResize)
	{
		if (SizeToResize > DataSize) Resize(BuffersCount, SizeToResize);
		for (size_t i = 0; i < BuffersCount; i++) memcpy(pDoublePointer[i], pData, SizeToResize * sizeof(TYPE));
	}

	void Push(TYPE** pData, fr_i32 SizeToResize, fr_i32 NewBuffersCount)
	{
		if (SizeToResize > DataSize) Resize(NewBuffersCount, SizeToResize);
		for (size_t i = 0; i < NewBuffersCount; i++) { memcpy(pDoublePointer[i], pData[i], SizeToResize); }
	}

	fr_i32 GetBuffersCount()
	{
		return BuffersCount;
	}

	fr_i32 GetBufferSize()
	{
		return DataSize;
	}

	TYPE* GetBufferData(fr_i32 Index)
	{
		if (Index > BuffersCount - 1) return nullptr;
		return pDoublePointer[Index];
	}

	TYPE** GetBuffers()
	{
		return pDoublePointer;
	}

	~C2DBuffer()
	{
		Free();
	}
};

typedef C2DBuffer<fr_f64> C2DDoubleBuffer;
typedef C2DBuffer<fr_f32> C2DFloatBuffer;
typedef C2DBuffer<fr_i32> C2DIntBuffer;
typedef C2DBuffer<fr_i16> C2DShortBuffer;
typedef C2DBuffer<fr_i8>  C2DByteBufffer;

template
<typename TYPE>
class CRingBuffer
{
private:
	fr_i32 CurrentBuffer = 0;
	fr_i32 BuffersCount = 0;
	fr_i32 BuffersSize = 0;
	fr_i32 BuffersLeft = 0;
	fr_i32 BufferPosition = 0;
	CBuffer<TYPE>** ppBuffers = nullptr;

public:
	CRingBuffer() {}
	CRingBuffer(fr_i32 CountOfBuffers, fr_i32 SizeOfBuffers)
	{
		SetBuffersCount(CountOfBuffers);
		Resize(SizeOfBuffers);
	}

	void SetBuffersCount(fr_i32 CountOfBuffers)
	{
		CBuffer<TYPE>** ppTempBuffers = nullptr;
		if (BuffersCount != CountOfBuffers) {
			ppTempBuffers = (CBuffer<TYPE>**)FastMemAlloc(sizeof(void*) * CountOfBuffers);
			if (ppBuffers) {
				for (size_t i = 0; i < std::min(abs(BuffersCount), CountOfBuffers); i++) {
					if (ppBuffers[i]) ppTempBuffers[i] = ppBuffers[i];
				}
			} else {
				for (size_t i = 0; i < CountOfBuffers; i++) {
					ppTempBuffers[i] = new CBuffer<TYPE>();
				}
			}

			ppBuffers = ppTempBuffers;
			BuffersCount = CountOfBuffers;
		}
	}

	void Clear()
	{
		if (ppBuffers) {
			for (size_t i = 0; i < BuffersCount; i++) {
				if (ppBuffers[i]) memset(ppBuffers[i], 0, BuffersSize * sizeof(TYPE));
			}
		}
	}

	void Resize(fr_i32 SizeToResize)
	{
		if (ppBuffers && SizeToResize != BuffersSize) {
			for (size_t i = 0; i < BuffersCount; i++) {
				if (ppBuffers[i]) ppBuffers[i]->Resize(SizeToResize);
			}

			BuffersSize = SizeToResize;
		}
	}

	void PushBuffer(TYPE* InData, fr_i32 SizeOfData)
	{
		ppBuffers[CurrentBuffer]->Push(InData, SizeOfData);
		BuffersLeft++;
	}

	void PushToNextBuffer(TYPE* InData, fr_i32 SizeOfData)
	{
		fr_i32 BufIndex = CurrentBuffer;
		BufIndex++;
		if (BufIndex >= BuffersCount) {
			BufIndex = 0;
		}

		ppBuffers[BufIndex]->Push(InData, SizeOfData);
		BuffersLeft++;
	}

	void NextBuffer()
	{
		BufferPosition = 0;
		if (++CurrentBuffer >= BuffersCount) {
			CurrentBuffer = 0;
		}
	}

	fr_i32 GetCurrentBuffer()
	{
		return CurrentBuffer;
	}

	TYPE* GetBufferData(fr_i32 index)
	{
		if (!ppBuffers) return nullptr;
		return ppBuffers[index]->Data();
	}

	fr_i32 ReadData(TYPE* OutBuffer, fr_i32 Samples) {
		fr_i32 WriteSamples = Samples;
		fr_i32 ReadSamples = 0;
		fr_i32 SampleReturn = 0;
		TYPE* pCurrentBuffer = GetData();

		if (WriteSamples <= 0) return 0;
		while (WriteSamples > 0) {
			if (BufferPosition >= BuffersSize) {
				NextBuffer();
				pCurrentBuffer = GetData();
				BuffersLeft--;
			}

			if (BuffersLeft <= 0) {
				return ReadSamples;
			}

			ReadSamples = std::min(BuffersSize - BufferPosition, WriteSamples);
			memcpy(&OutBuffer[SampleReturn], &pCurrentBuffer[BufferPosition], ReadSamples * sizeof(TYPE));
			BufferPosition += ReadSamples;
			SampleReturn += ReadSamples;
			WriteSamples -= ReadSamples;

			if (BufferPosition >= BuffersSize) {
				NextBuffer();
				pCurrentBuffer = GetData();
				BuffersLeft--;
			}
		}

		return SampleReturn;
	}

	fr_i32 GetLeftSamples() { return (BuffersLeft * BuffersSize) - BufferPosition; }
	fr_i32 GetLeftBuffers() { return BuffersLeft; }

	TYPE* GetData()
	{
		return GetBufferData(CurrentBuffer);
	}

	fr_i32 GetBuffersCount()
	{
		return BuffersCount;
	}

	fr_i32 GetBufferSize()
	{ 
		return BuffersSize;
	}

	~CRingBuffer()
	{
		if (ppBuffers) {
			for (size_t i = 0; i < BuffersCount; i++) {
				if (ppBuffers[i]) delete ppBuffers[i];
			}

			FreeFastMemory(ppBuffers);
		}
	}
};

typedef CRingBuffer<fr_f64> CRingDoubleBuffer;
typedef CRingBuffer<fr_f32> CRingFloatBuffer;
typedef CRingBuffer<fr_i32> CRingIntBuffer;
typedef CRingBuffer<fr_i16> CRingShortBuffer;
typedef CRingBuffer<fr_i8>  CRingByteBufffer;

enum SoundState : fr_i32
{
	NoneState = 0,
	PlayingState,
	PausedState,
	StoppedState
};

class IBaseInterface
{
protected:
	long Counter = 0;

public:
	virtual bool Clone(void** ppOutInterface)
	{
		if (!ppOutInterface) return false;
		*ppOutInterface = this;
		AddRef();
		return true;
	}

	virtual long AddRef()
	{
#ifdef WINDOWS_PLATFORM
		return _InterlockedIncrement(&Counter);
#else
		return __sync_fetch_and_add(&Counter, 1);
#endif
	}

	virtual void Release()
	{
#ifdef WINDOWS_PLATFORM
		if (_InterlockedDecrement(&Counter) <= 0) {
			delete this;
		}
#else
		if (__sync_fetch_and_sub(&Counter, 1) <= 0) {
			delete this;
		}
#endif
	}

	virtual ~IBaseInterface() = default;
};

class IBaseEvent
{
public:
	virtual void Raise() = 0;
	virtual void Reset() = 0;
	virtual void Wait() = 0;
	virtual bool Wait(fr_i32 TimeToWait) = 0;
	virtual bool IsRaised() = 0;
};

class IAudioCallback : public IBaseInterface
{
public:
	virtual fr_err FlushCallback() = 0;
	virtual fr_err FormatCallback(PcmFormat* fmtToSwitch) = 0;
	virtual fr_err EndpointCallback(fr_f32* pData, fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate, fr_i32 CurrentEndpointType) = 0;
	virtual fr_err RenderCallback(fr_i32 Frames, fr_i32 Channels, fr_i32 SampleRate) = 0;
};

inline
void
MixerAddToBuffer(
	fr_f32* pFirstBuffer,
	fr_f32* pSecondBuffer,
	fr_i64 Samples
)
{
	for (fr_i64 i = 0; i < Samples; i++) {
		pFirstBuffer[i] += pSecondBuffer[i];
	}
}

inline
void
PlanarToLinear(
	fr_f32** pPlanar,
	fr_f32* pLinear,
	fr_i32 SamplesCount,
	fr_i32 Channels
)
{
	fr_i64 index = 0;
	fr_i64 index2 = 0;

	switch (Channels)
	{
	case 1: {
		while (SamplesCount >= 8) {
			pLinear[index] = pPlanar[0][index];
			pLinear[index + 1] = pPlanar[0][index + 1];
			pLinear[index + 2] = pPlanar[0][index + 2];
			pLinear[index + 3] = pPlanar[0][index + 3];
			pLinear[index + 4] = pPlanar[0][index + 4];
			pLinear[index + 5] = pPlanar[0][index + 5];
			pLinear[index + 6] = pPlanar[0][index + 6];
			pLinear[index + 7] = pPlanar[0][index + 7];
			index += 8;
			SamplesCount -= 8;
		}
	} break;
	case 2: {
		while (SamplesCount >= 8) {
			fr_f32 A = pPlanar[0][index2];		
			fr_f32 B = pPlanar[1][index2];		
			fr_f32 C = pPlanar[0][index2 + 1];
			fr_f32 D = pPlanar[1][index2 + 1];
			fr_f32 E = pPlanar[0][index2 + 2];
			fr_f32 F = pPlanar[1][index2 + 2];
			fr_f32 G = pPlanar[0][index2 + 3];
			fr_f32 H = pPlanar[1][index2 + 3];
			pLinear[index] = A;
			pLinear[index + 1] = B;
			pLinear[index + 2] = C;
			pLinear[index + 3] = D;
			pLinear[index + 4] = E;
			pLinear[index + 5] = F;
			pLinear[index + 6] = G;
			pLinear[index + 7] = H;
			index += 8;
			index2 += 4;
			SamplesCount -= 8;
		}
	} break;
	case 4: {
		while (SamplesCount >= 8) {
			fr_f32 A = pPlanar[0][index2];
			fr_f32 B = pPlanar[1][index2];
			fr_f32 C = pPlanar[2][index2];
			fr_f32 D = pPlanar[3][index2];
			fr_f32 E = pPlanar[0][index2 + 1];
			fr_f32 F = pPlanar[1][index2 + 1];
			fr_f32 G = pPlanar[2][index2 + 1];
			fr_f32 H = pPlanar[3][index2 + 1];
			pLinear[index] = A;
			pLinear[index + 1] = B;
			pLinear[index + 2] = C;
			pLinear[index + 3] = D;
			pLinear[index + 4] = E;
			pLinear[index + 5] = F;
			pLinear[index + 6] = G;
			pLinear[index + 7] = H;
			index += 8;
			index2 += 2;
			SamplesCount -= 8;
		}
	} break;
	case 6: {
		while (SamplesCount >= 6) {
			fr_f32 A = pPlanar[0][index2];
			fr_f32 B = pPlanar[1][index2];
			fr_f32 C = pPlanar[2][index2];
			fr_f32 D = pPlanar[3][index2];
			fr_f32 E = pPlanar[4][index2];
			fr_f32 F = pPlanar[5][index2];
			pLinear[index] = A;
			pLinear[index + 1] = B;
			pLinear[index + 2] = C;
			pLinear[index + 3] = D;
			pLinear[index + 4] = E;
			pLinear[index + 5] = F;
			index += 6;
			index2 += 1;
			SamplesCount -= 6;
		}
	} break;
	case 8: {
		while (SamplesCount >= 8) {
			fr_f32 A = pPlanar[0][index2];
			fr_f32 B = pPlanar[1][index2];
			fr_f32 C = pPlanar[2][index2];
			fr_f32 D = pPlanar[3][index2];
			fr_f32 E = pPlanar[4][index2];
			fr_f32 F = pPlanar[5][index2];
			fr_f32 G = pPlanar[6][index2];
			fr_f32 H = pPlanar[7][index2];
			pLinear[index] = A;
			pLinear[index + 1] = B;
			pLinear[index + 2] = C;
			pLinear[index + 3] = D;
			pLinear[index + 4] = E;
			pLinear[index + 5] = F;
			pLinear[index + 6] = G;
			pLinear[index + 7] = H;
			index += 8;
			index2++;
			SamplesCount -= 8;
		}
	} break;
	default:
		break;
	}

	while (SamplesCount > 0) {
		pLinear[index] = pPlanar[index % Channels][index / Channels];
		index++;
		SamplesCount--;
	}
}

inline
void
LinearToPlanar(
	fr_f32** pPlanar,
	fr_f32* pLinear,
	fr_i32 SamplesCount,
	fr_i32 Channels
)
{
	fr_i64 index = 0;
	fr_i64 index2 = 0;

	switch (Channels)
	{
	case 1: {
		while (SamplesCount >= 8) {
			pPlanar[0][index] = pLinear[index];
			pPlanar[0][index + 1] = pLinear[index + 1];
			pPlanar[0][index + 2] = pLinear[index + 2];
			pPlanar[0][index + 3] = pLinear[index + 3];
			pPlanar[0][index + 4] = pLinear[index + 4];
			pPlanar[0][index + 5] = pLinear[index + 5];
			pPlanar[0][index + 6] = pLinear[index + 6];
			pPlanar[0][index + 7] = pLinear[index + 7];
			index += 8;
			SamplesCount -= 8;
		}
	} break;
	case 2: {
		while (SamplesCount >= 8) {
			fr_f32 A = pLinear[index];
			fr_f32 B = pLinear[index + 1];
			fr_f32 C = pLinear[index + 2];
			fr_f32 D = pLinear[index + 3];
			fr_f32 E = pLinear[index + 4];
			fr_f32 F = pLinear[index + 5];
			fr_f32 G = pLinear[index + 6];
			fr_f32 H = pLinear[index + 7];
			pPlanar[0][index2] = A;
			pPlanar[1][index2] = B;
			pPlanar[0][index2 + 1] = C;
			pPlanar[1][index2 + 1] = D;
			pPlanar[0][index2 + 2] = E;
			pPlanar[1][index2 + 2] = F;
			pPlanar[0][index2 + 3] = G;
			pPlanar[1][index2 + 3] = H; 
			index += 8;
			index2 += 4;
			SamplesCount -= 8;
		}
	} break;
	case 4: {
		while (SamplesCount >= 8) {
			fr_f32 A = pLinear[index];
			fr_f32 B = pLinear[index + 1];
			fr_f32 C = pLinear[index + 2];
			fr_f32 D = pLinear[index + 3];
			fr_f32 E = pLinear[index + 4];
			fr_f32 F = pLinear[index + 5];
			fr_f32 G = pLinear[index + 6];
			fr_f32 H = pLinear[index + 7];
			pPlanar[0][index2] = A;
			pPlanar[1][index2] = B;
			pPlanar[2][index2] = C;
			pPlanar[3][index2] = D;
			pPlanar[0][index2 + 1] = E;
			pPlanar[1][index2 + 1] = F;
			pPlanar[2][index2 + 1] = G;
			pPlanar[3][index2 + 1] = H;
			index += 8;
			index2 += 2;
			SamplesCount -= 8;
		}
	} break;
	case 6: {
		while (SamplesCount >= 6) {
			fr_f32 A = pLinear[index];
			fr_f32 B = pLinear[index + 1];
			fr_f32 C = pLinear[index + 2];
			fr_f32 D = pLinear[index + 3];
			fr_f32 E = pLinear[index + 4];
			fr_f32 F = pLinear[index + 5];
			pPlanar[0][index2] = A;
			pPlanar[1][index2] = B;
			pPlanar[2][index2] = C;
			pPlanar[3][index2] = D;
			pPlanar[4][index2] = E;
			pPlanar[5][index2] = F;
			index += 6;
			index2 += 1;
			SamplesCount -= 6;
		}
	} break;
	case 8: {
		while (SamplesCount >= 8) {
			fr_f32 A = pLinear[index];
			fr_f32 B = pLinear[index + 1];
			fr_f32 C = pLinear[index + 2];
			fr_f32 D = pLinear[index + 3];
			fr_f32 E = pLinear[index + 4];
			fr_f32 F = pLinear[index + 5];
			fr_f32 G = pLinear[index + 6];
			fr_f32 H = pLinear[index + 7];
			pPlanar[0][index2] = A;
			pPlanar[1][index2] = B;
			pPlanar[2][index2] = C;
			pPlanar[3][index2] = D;
			pPlanar[4][index2] = E;
			pPlanar[5][index2] = F;
			pPlanar[6][index2] = G;
			pPlanar[7][index2] = H;
			index += 8;
			index2++;
			SamplesCount -= 8;
		}
	} break;
	default:
		break;
	}

	while (SamplesCount > 0){
		pPlanar[index % Channels][index / Channels] = pLinear[index];
		index++;
		SamplesCount--;
	}
}


inline
bool
MixComplexToArray(
	fr_f32* pComplex,
	fr_f32** ppArray,
	fr_i32 Channels,
	fr_i32 FramesToConvert
)
{
	if (!pComplex || !ppArray || !*ppArray) return false;

	for (fr_i64 i = 0; i < FramesToConvert / Channels; i++) {
		for (fr_i32 o = 0; o < Channels; o++) {
			ppArray[o][i] += pComplex[i * Channels + o];
		}
	}
}

inline
void
FloatToDouble(
	fr_f32** pFloat, 
	fr_f64** pDouble,
	fr_i32 ChannelsCount, 
	fr_i32 FramesCount
)
{
	for (fr_i32 i = 0; i < ChannelsCount; i++) {
		fr_f32* pTempData = pFloat[i];
		fr_f64* pDoubleData = pDouble[i];
		for (fr_i32 o = 0; o < FramesCount; o++) {
			pDoubleData[o] = pTempData[o];
		}
	}
}

inline
void
DoubleToFloat(
	fr_f32** pFloat,
	fr_f64** pDouble, 
	fr_i32 ChannelsCount,
	fr_i32 FramesCount
)
{
	for (fr_i32 i = 0; i < ChannelsCount; i++) {
		fr_f32* pTempData = pFloat[i];
		fr_f64* pDoubleData = pDouble[i];
		for (fr_i32 o = 0; o < FramesCount; o++) {
			pTempData[o] = (fr_f32)pDoubleData[o];
		}
	}
}

#ifdef WINDOWS_PLATFORM
inline char* utf16_to_utf8(const wchar_t* _src) {
	char* dst;
	size_t  len;
	size_t  si;
	size_t  di;
	len = wcslen(_src);
	dst = (char*)FastMemAlloc((fr_i32)(sizeof(*dst) * ((fr_i64)3 * (fr_i64)len + (fr_i64)1)));
	if (dst == NULL)return dst;
	for (di = si = 0; si < len; si++) {
		unsigned c0;
		c0 = _src[si];
		if (c0 < 0x80) {
			/*Can be represented by a 1-byte sequence.*/
			dst[di++] = (char)c0;
			continue;
		}
		else if (c0 < 0x800) {
			/*Can be represented by a 2-byte sequence.*/
			dst[di++] = (char)(0xC0 | c0 >> 6);
			dst[di++] = (char)(0x80 | c0 & 0x3F);
			continue;
		}
		else if (c0 >= 0xD800 && c0 < 0xDC00) {
			unsigned c1;
			/*This is safe, because c0 was not 0 and _src is NUL-terminated.*/
			c1 = _src[si + 1];
			if (c1 >= 0xDC00 && c1 < 0xE000) {
				unsigned w;
				/*Surrogate pair.*/
				w = ((c0 & 0x3FF) << 10 | c1 & 0x3FF) + 0x10000;
				/*Can be represented by a 4-byte sequence.*/
				dst[di++] = (char)(0xF0 | w >> 18);
				dst[di++] = (char)(0x80 | w >> 12 & 0x3F);
				dst[di++] = (char)(0x80 | w >> 6 & 0x3F);
				dst[di++] = (char)(0x80 | w & 0x3F);
				si++;
				continue;
			}
		}
		/*Anything else is either a valid 3-byte sequence, an invalid surrogate
		   pair, or 'not a character'.
		  In the latter two cases, we just encode the value as a 3-byte
		   sequence anyway (producing technically invalid UTF-8).
		  Later error handling will detect the problem, with a better
		   chance of giving a useful error message.*/
		dst[di++] = (char)(0xE0 | c0 >> 12);
		dst[di++] = (char)(0x80 | c0 >> 6 & 0x3F);
		dst[di++] = (char)(0x80 | c0 & 0x3F);
	}
	dst[di++] = '\0';
	return dst;
}
#endif

#define _RELEASE(p) { if (p) { (p)->Release(); (p) = nullptr;} }
#define ELEMENTSCOUNT(x) sizeof(x) / sizeof(sizeof(x[0]))

#ifdef WINDOWS_PLATFORM
#define IsInvalidHandle(x) (x == 0 || x == INVALID_HANDLE_VALUE)

inline 
void
PcmFormatToWaveFormatEx(
	PcmFormat& pcmFmt, 
	WAVEFORMATEX& waveEx
)
{
	waveEx.cbSize = sizeof(WAVEFORMATEX);
	waveEx.nChannels = pcmFmt.Channels;
	waveEx.nSamplesPerSec = pcmFmt.SampleRate;
	waveEx.wBitsPerSample = pcmFmt.Bits;
	waveEx.wFormatTag = pcmFmt.IsFloat ? 3 : 1;
	waveEx.nBlockAlign = waveEx.wBitsPerSample * waveEx.nChannels;
	waveEx.nAvgBytesPerSec = waveEx.nBlockAlign * waveEx.nSamplesPerSec;
}

inline
void
WaveFormatExToPcmFormat(
	WAVEFORMATEX& waveEx,
	PcmFormat& pcmFmt
)
{
	pcmFmt.Bits = waveEx.wBitsPerSample;
	pcmFmt.Channels = waveEx.nChannels;
	pcmFmt.IsFloat = waveEx.wFormatTag == 3 ? true : false;
	pcmFmt.SampleRate = waveEx.nSamplesPerSec;
}

class CWinEvent : public IBaseEvent
{
private:
	HANDLE hEvent = nullptr;

public:
	CWinEvent();
	~CWinEvent();

	void Raise() override;
	void Reset() override;
	void Wait() override;
	bool Wait(fr_i32 TimeToWait) override;
	bool IsRaised() override;
};
#else
class CPosixEvent : public IBaseEvent
{
public:
    CPosixEvent();
    ~CPosixEvent();

    void Raise() override;
    void Reset() override;
    void Wait() override;
    bool Wait(fr_i32 TimeToWait) override;
    bool IsRaised() override;

    struct EventHandle
    {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool signaled;
    };

private:
    EventHandle m_id;
};
#endif

enum EFSOpenFlags
{
	eNoFlag = 0x0,
	eReadFlag = 0x1,
	eWriteFlag = 0x2,
	eCreateAlwaysFlag = 0x4,
	eMustExistFlag = 0x8
};

enum EFSMapping
{
	eMappingRead = 0x1,
	eMappintWrite = 0x2
};

class IFreponzeMapFile : public IBaseInterface
{
protected:
	fr_i32 FileFlags = 0;
	fr_i64 FilePosition = 0;
	fr_ptr pFileHandle = nullptr;
	fr_ptr pMapHandle = nullptr;

public:
	virtual bool Open(const fr_utf8* FileLink, fr_i32 Flags) = 0;
	virtual void Close() = 0;

	virtual fr_i64 GetSize() = 0;

	virtual bool MapFile(fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags) = 0;
	virtual bool MapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags) = 0;

	virtual bool UnmapFile(fr_ptr& OutPtr) = 0;
	virtual bool UnmapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr) = 0;
};

inline
void
riff_to_pcm(
	wav_header* header,
	PcmFormat* format
)
{
	format->IsFloat = header->audio_format == 3;
	format->Bits = header->bit_depth;
	format->Channels = header->num_channels;
	format->Frames = header->data_bytes / (format->Bits / 8) / header->num_channels;
	format->SampleRate = header->sample_rate;
}

inline
fr_f32
i16tof32(fr_i16 wValue)
{
	fr_f32 fValue = .0f;
	fValue = (float)wValue / 32768.0f;
	return fValue;
}

inline
fr_i16
f32toi16(fr_f32 fValue)
{
	fr_i16 iValue = 0;
	iValue = (fr_i16)maxmin(((int)(fValue * 32768.0f)), -32768, 32767);
	return iValue;
}

inline
char*
GetFilePathFormat(char* pathToFile)
{
	char* StringEnd = pathToFile + strlen(pathToFile);
	while (StringEnd != pathToFile && *StringEnd != '.') StringEnd--;
	if (StringEnd == pathToFile) return nullptr;
	return StringEnd == pathToFile ? nullptr : StringEnd;
}

inline
void
GetDebugTime(char* pToPrint, size_t BufSize)
{
	static fr_i64 TickCount = 0;
	fr_i64 Time_ = 0;
	fr_i64 ChIDX = 0;
#ifdef WINDOWS_PLATFORM
	if (!TickCount) TickCount = GetTickCount64();
	if (!pToPrint) return;
	Time_ = GetTickCount64();
#else

#endif
	Time_ -= TickCount;
	fr_u64 Time_MS = Time_ % 1000; Time_ = Time_ / 1000;
	fr_u64 Time_Sec = Time_ % 60;   Time_ = Time_ / 60;
	fr_u64 Time_Min = Time_ % 60;   Time_ = Time_ / 60;
	fr_u64 Time_Hors = Time_ % 24;
	fr_u64 Time_Days = Time_ / 60;
	fr_u64 Time_MS_0 = Time_MS % 10; Time_MS = Time_MS / 10;
	fr_u64 Time_MS_1 = Time_MS % 10; Time_MS = Time_MS / 10;
	fr_u64 Time_MS_2 = Time_MS;
	fr_u64 Time_Sec_0 = Time_Sec % 10;
	fr_u64 Time_Sec_1 = Time_Sec / 10;
	fr_u64 Time_Min_0 = Time_Min % 10;
	fr_u64 Time_Min_1 = Time_Min / 10;
	fr_u64 Time_Hors_0 = Time_Hors % 10;
	fr_u64 Time_Hors_1 = Time_Hors / 10;
	pToPrint[ChIDX++] = '[';
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Hors_1;
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Hors_0;
	pToPrint[ChIDX++] = ':';
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Min_1;
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Min_0;
	pToPrint[ChIDX++] = ':';
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Sec_1;
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_Sec_0;
	pToPrint[ChIDX++] = ':';
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_MS_2;
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_MS_1;
	pToPrint[ChIDX++] = '0' + (fr_utf8)Time_MS_0;
	pToPrint[ChIDX++] = ']';
	pToPrint[ChIDX++] = ':';
	pToPrint[ChIDX++] = ' ';
	pToPrint[ChIDX++] = '\0';
}

inline
void
GetDebugTimeW(wchar_t* pToPrint, size_t BufSize)
{
	static fr_i64 TickCount = 0;
	fr_i64 Time_ = 0;
	fr_i64 ChIDX = 0;
#ifdef WINDOWS_PLATFORM
	if (!TickCount) TickCount = GetTickCount64();
	if (!pToPrint) return;
	Time_ = GetTickCount64();
#else

#endif

	Time_ -= TickCount;
	fr_u64 Time_MS = Time_ % 1000; Time_ = Time_ / 1000;
	fr_u64 Time_Sec = Time_ % 60;   Time_ = Time_ / 60;
	fr_u64 Time_Min = Time_ % 60;   Time_ = Time_ / 60;
	fr_u64 Time_Hors = Time_ % 24;
	fr_u64 Time_Days = Time_ / 60;
	fr_u64 Time_MS_0 = Time_MS % 10; Time_MS = Time_MS / 10;
	fr_u64 Time_MS_1 = Time_MS % 10; Time_MS = Time_MS / 10;
	fr_u64 Time_MS_2 = Time_MS;
	fr_u64 Time_Sec_0 = Time_Sec % 10;
	fr_u64 Time_Sec_1 = Time_Sec / 10;
	fr_u64 Time_Min_0 = Time_Min % 10;
	fr_u64 Time_Min_1 = Time_Min / 10;
	fr_u64 Time_Hors_0 = Time_Hors % 10;
	fr_u64 Time_Hors_1 = Time_Hors / 10;
	pToPrint[ChIDX++] = L'[';
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Hors_1;
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Hors_0;
	pToPrint[ChIDX++] = L':';
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Min_1;
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Min_0;
	pToPrint[ChIDX++] = L':';
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Sec_1;
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_Sec_0;
	pToPrint[ChIDX++] = L':';
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_MS_2;
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_MS_1;
	pToPrint[ChIDX++] = L'0' + (fr_utf8)Time_MS_0;
	pToPrint[ChIDX++] = L']';
	pToPrint[ChIDX++] = L':';
	pToPrint[ChIDX++] = L' ';
	pToPrint[ChIDX++] = L'\0';
}


void TypeToLogFormated(const char* Text, ...);
void TypeToLogW(const wchar_t* Text);
void TypeToLog(const char* Text);
void TypeToLog(long long count);

inline 
void
DebugAssert(
	bool value,
	const char* Text
)
{
	if (!value) {
		if (!Text) Text = "Assert for debug things";
#ifdef WINDOWS_PLATFORM
		OutputDebugStringA(Text);
#ifdef SHOW_MESSAGEBOXES
		MessageBoxA(nullptr, Text, "Fresponze debug error", MB_OK | MB_ICONHAND);
#endif
		DebugBreak();
#else
		printf(Text);
#endif

	}
}

void* GetMapFileSystem();

#ifndef _NDEBUG
#define BugAssert(xx, yy) DebugAssert(!!(xx), yy)
#define BugAssert1(xx) DebugAssert(!!(xx), nullptr)
#else
#define BugAssert(xx, yy) (xx)
#define BugAssert1(xx) (xx)
#endif

#ifdef ENABLE_PERFOMANCE_DEBUG
fr_i64 DebugStamp(); 
#define FRESPONZE_BEGIN_TEST \
	{ \
	fr_i64 DebugCounterTick = DebugStamp(); 
	
#define FRESPONZE_END_TEST \
	DebugCounterTick = DebugStamp() - DebugCounterTick;	\
		TypeToLog(DebugCounterTick); \
	}
#else
#define BEGIN_TEST 
#define END_TEST
#endif