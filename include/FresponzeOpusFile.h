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
#include "opusfile.h"

class COpusMediaResource : public IMediaResource
{
private:
	bool isOpened = false;
	fr_i32 previous_li = 0;
	fr_i32 FileReadSize = 0;
	fr_i64 pcm_offset = 0;
	fr_i64 PtrSize = 0;
	fr_i64 FSeek = 0;
	fr_ptr FilePtr = nullptr;
	OggOpusFile* of = nullptr;
	PcmFormat formatOfFile = {};
	OpusFileCallbacks cb = { nullptr, nullptr, nullptr, nullptr };
	IBaseResampler* resampler = nullptr;

public:
	COpusMediaResource(IFreponzeMapFile* pNewMapper = nullptr);
	~COpusMediaResource();

	bool OpenResource(void* pResourceLinker) override;
	bool CloseResource() override;

	void GetFormat(PcmFormat& format) override;
	void SetFormat(PcmFormat outputFormat) override;

	void GetVendorName(const char*& vendorName) override;		// vendor name, in tags structure
	void GetVendorString(const char*& vendorString) override;	// vendor comment, in tags structure

	fr_i64 Read(fr_i64 FramesCount, fr_f32** ppFloatData) override;
	fr_i64 ReadRaw(fr_i64 FramesCount, fr_f32** ppFloatData) override;

	fr_i64 SetPosition(fr_i64 FramePosition) override;
	fr_i64 GetPosition() override;
};
