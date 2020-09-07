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
#pragma once
#include "FresponzeFileSystem.h"

class CWindowsMapFile : public IFreponzeMapFile
{
private:
	fr_i64 filemapStart = 0;
	fr_i64 mapviewSize = 0;

public:
	CWindowsMapFile();
	~CWindowsMapFile();

	bool Open(const fr_utf8* FileLink, fr_i32 Flags) override;
	void Close() override;

	fr_i64 GetSize() override;

	bool MapFile(fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags) override;
	bool MapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr, fr_u64 OffsetFile, fr_i32 ProtectFlags) override;

	bool UnmapFile(fr_ptr& OutPtr) override;
	bool UnmapPointer(fr_i64 SizeToMap, fr_ptr& OutPtr) override;
};
