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
#include "FresponzeTypes.h"

/*
	Please, don't use 'using namespace Fresponze'. It's can be broke
	your application code via namespace collusion.
*/
namespace Fresponze
{
	enum CurrentAllocator : fr_i32
	{
		eNoneAllocator,
		eStdAllocator,
		eVMapAllocator
	};

	struct AllocatorInformation
	{
		fr_i32 AllocatorGranularity;
		fr_i32 AllocatorPageSize;
		fr_ptr AllocatorHandle;				// File mapping or heap handle
	};

	fr_err CheckAllocator(CurrentAllocator* alloc);
	fr_err GetAllocatorInfo(AllocatorInformation* allocInfo);

	fr_ptr AllocateMemory(fr_i64 Sizeof);
	fr_err FreeMemory(fr_ptr PtrToMemory);


}
