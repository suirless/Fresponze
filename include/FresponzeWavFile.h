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
#include "FresponzeMediaResource.h"

class CRIFFMediaResource : public IMediaResource
{
private:
	IBaseResampler* resampler = nullptr;

	fr_f32 GetSample(fr_i64 Index);

public:
	CRIFFMediaResource(IFreponzeMapFile* pNewMapper = nullptr);
	~CRIFFMediaResource();

	bool OpenResource(void* pResourceLinker) override;
	bool CloseResource() override;

	void GetVendorName(const char*& vendorName) override;		// vendor name, in tags structure
	void GetVendorString(const char*& vendorString) override;	// vendor comment, in tags structure
	void GetFormat(PcmFormat& format) override;
	void SetFormat(PcmFormat outputFormat) override;

	fr_i64 Read(fr_i64 FramesCount, fr_f32** ppFloatData) override;
	fr_i64 ReadRaw(fr_i64 FramesCount, fr_f32** ppFloatData) override;

	fr_i64 SetPosition(fr_i64 FramePosition) override;
	fr_i64 GetPosition() override;
};
