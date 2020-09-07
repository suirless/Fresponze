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
#include "FresponzeResampler.h"
#include "FresponzeFileSystem.h"

class IMediaResource : public IBaseInterface
{
protected:
	fr_i64 FramePosition = 0;
	fr_i64 FileFrames = 0;
	fr_ptr pMappedArea = nullptr;
	CFloatBuffer tempBuffer = {};			// while we reading file
	C2DFloatBuffer transferBuffers = {};	// on read function
	PcmFormat fileFormat = {};				// input format, from file
	PcmFormat outputFormat = {};			// format for read function
	IFreponzeMapFile* pMapper = nullptr;

public:
	virtual bool OpenResource(void* pResourceLinker) = 0;
	virtual bool CloseResource() = 0;

	virtual void GetFormat(PcmFormat& format) = 0;
	virtual void SetFormat(PcmFormat outputFormat) = 0;

	virtual void GetVendorName(const char*& vendorName) = 0;
	virtual void GetVendorString(const char*& vendorString) = 0;

	virtual fr_i64 Read(fr_i64 FramesCount, fr_f32** ppFloatData) = 0;
	virtual fr_i64 ReadRaw(fr_i64 FramesCount, fr_f32** ppFloatData) = 0;

	virtual fr_i64 SetPosition(fr_i64 FramePosition) = 0;
	virtual fr_i64 GetPosition() = 0;
};
