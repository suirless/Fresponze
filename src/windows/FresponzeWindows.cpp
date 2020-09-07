/*****************************************************************
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
#include "FresponzeTypes.h"
#include "FresponzeFileSystemWindows.h"
#define ALIGN_SIZE(Size, AlSize)        ((Size + (AlSize-1)) & (~(AlSize-1)))
#define ALIGN_SIZE_64K(Size)            ALIGN_SIZE(Size, 65536)
#define ALIGN_SIZE_16(Size)             ALIGN_SIZE(Size, 16)

HANDLE hFresponzeHeap = nullptr;
fr_u32 MemoryGranularity = 0;

fr_i64 
DebugStamp()
{
	LARGE_INTEGER larg_ret = {};
	fr_i64 freq = 0;
	if (!freq) {
		LARGE_INTEGER larg = {};
		QueryPerformanceFrequency(&larg);
		freq = larg.QuadPart;
	}

	QueryPerformanceCounter(&larg_ret);
	return larg_ret.QuadPart / (freq / 10000);
}

bool
Fresponze::InitMemory()
{
	SYSTEM_INFO sysInfo = {};
	GetNativeSystemInfo(&sysInfo);
	MemoryGranularity = sysInfo.dwAllocationGranularity;
	hFresponzeHeap = HeapCreate(0, 0x040000, 0);
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) return false;
	GetDebugTime(nullptr, 0);
	return !IsInvalidHandle(hFresponzeHeap);
}

void
Fresponze::DestroyMemory()
{
	CoUninitialize();
	if (!IsInvalidHandle(hFresponzeHeap)) HeapDestroy(hFresponzeHeap);
}

void*
FastMemAlloc(
	fr_i32 SizeToAllocate
)
{
#ifndef _DEBUG
	return HeapAlloc(hFresponzeHeap, 0, SizeToAllocate);
#else
	void* pret = HeapAlloc(hFresponzeHeap, 0, SizeToAllocate);
	if (pret) memset(pret, 0, SizeToAllocate);
	return pret;
#endif
}

void*
FastMemRealloc(
	void* Ptr,
	fr_i32 SizeToAllocate
)
{
	return HeapReAlloc(hFresponzeHeap, 0, Ptr, SizeToAllocate);
}


void* 
VirtMemAlloc(
	fr_i64 SizeToAllocate
)
{
	return VirtualAlloc(nullptr, ALIGN_SIZE(SizeToAllocate, (fr_i64)MemoryGranularity), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void
FreeFastMemory(
	void* Ptr
)
{
	HeapFree(hFresponzeHeap, 0, Ptr);
}

void 
FreeVirtMemory(
	void* Ptr, 
	size_t Size
)
{
	VirtualFree(Ptr, 0, MEM_RELEASE);
}

void* 
GetMapFileSystem()
{
	return new CWindowsMapFile();
}

CWinEvent::CWinEvent()
{
	hEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
}

CWinEvent::~CWinEvent()
{
	if (!IsInvalidHandle(hEvent)) CloseHandle(hEvent);
}

void
CWinEvent::Raise()
{
	SetEvent(hEvent);
}

void
CWinEvent::Reset()
{
	ResetEvent(hEvent);
}

void
CWinEvent::Wait()
{
	WaitForSingleObject(hEvent, INFINITE);
}

bool
CWinEvent::Wait(
	fr_i32 TimeToWait
)
{
	DWORD dwRet = WaitForSingleObject(hEvent, (DWORD)TimeToWait);
	return dwRet == WAIT_OBJECT_0;
}

bool
CWinEvent::IsRaised()
{
	return WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0;
}
