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
#include "FresponzeFileSystemWindows.h"

extern fr_u32 MemoryGranularity;

CWindowsMapFile::CWindowsMapFile()
{
	AddRef();
}

CWindowsMapFile::~CWindowsMapFile()
{

}

bool
CWindowsMapFile::Open(const fr_utf8* FileLink, fr_i32 Flags)
{
	fr_wstring4k maxPathString = {};
	DWORD dwGenericFlags = 0;
	DWORD dwSharedFlags = 0;

	if (MultiByteToWideChar(CP_UTF8, 0, FileLink, -1, maxPathString, ARRAYSIZE(maxPathString)) <= 0) return false;

	/* Convert local flags to global and try to open file handle */
	if (Flags & eReadFlag)			dwGenericFlags	|= GENERIC_READ;
	if (Flags & eWriteFlag)			dwGenericFlags	|= GENERIC_WRITE;
	if (Flags & eCreateAlwaysFlag)	dwSharedFlags	|= CREATE_ALWAYS;
	if (Flags & eMustExistFlag)		dwSharedFlags	|= OPEN_EXISTING;
	pFileHandle = (fr_ptr)CreateFileW(maxPathString, dwGenericFlags, FILE_SHARE_READ, nullptr, dwSharedFlags, 0, nullptr);
	if (IsInvalidHandle(pFileHandle)) return false;

	/* 
		If we want to create map of file - we must open file mapping handle. 
		Only after that we can use MapViewOfFile() function
	*/
	pMapHandle = (fr_ptr)CreateFileMappingW((HANDLE)pFileHandle, nullptr, Flags & eWriteFlag ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (IsInvalidHandle(pMapHandle)) {
		CloseHandle((HANDLE)pFileHandle);
		return false;
	}

	return true;
}

void	
CWindowsMapFile::Close()
{
	if (!IsInvalidHandle(pMapHandle))	{ CloseHandle(pMapHandle);	pMapHandle = nullptr; }
	if (!IsInvalidHandle(pFileHandle))	{ CloseHandle(pFileHandle); pMapHandle = nullptr; }
}

fr_i64	
CWindowsMapFile::GetSize()
{
	LARGE_INTEGER larg = {};
	if (!GetFileSizeEx((HANDLE)pFileHandle, &larg)) return -1;
	return larg.QuadPart;
}

bool	
CWindowsMapFile::MapFile(fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags)
{
	return this->MapPointer(this->GetSize(), OutPtr, OffsetFile, ProtectFlags);
}

bool	
CWindowsMapFile::MapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags)
{
	DWORD dwAccessFlags = 0;
	if (ProtectFlags & eMappingRead)	dwAccessFlags |= FILE_MAP_READ;
	if (ProtectFlags & eMappintWrite)	dwAccessFlags |= FILE_MAP_WRITE;

	OutPtr = MapViewOfFileEx((HANDLE)pMapHandle, dwAccessFlags, (OffsetFile >> 32), (OffsetFile & 0xFFFFFFFF), SizeToMap, nullptr);
	return !!(OutPtr);
}

bool	
CWindowsMapFile::UnmapFile(fr_ptr& OutPtr)
{
	return !!this->UnmapPointer(0, OutPtr);		// igone size for unmap
}

bool	
CWindowsMapFile::UnmapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr)
{
	bool bRet = !!UnmapViewOfFile(OutPtr);
	if (OutPtr) OutPtr = nullptr;
	return bRet;
}
