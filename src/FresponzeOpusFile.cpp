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
#include "FresponzeOpusFile.h"

#define OPUS_BUFFER 11520		// 120 ms on 48000Hz

COpusMediaResource::COpusMediaResource(IFreponzeMapFile* pNewMapper)
{
	AddRef();
	resampler = GetCurrentResampler();
	if (!pNewMapper) pMapper = (IFreponzeMapFile*)GetMapFileSystem();
	else pNewMapper->Clone((void**)&pMapper);
}

COpusMediaResource::~COpusMediaResource()
{
	if (resampler) delete resampler;
	CloseResource();
	_RELEASE(pMapper);
}

bool 
COpusMediaResource::OpenResource(void* pResourceLinker)
{
	if (isOpened) return true;
	fr_i32 ret = 0;
	fr_i32 li = 0;
	const OpusHead* head = nullptr;
	const OpusTags* tags = nullptr;

	/*	#############################################
		OPT 003: Perfomance issue with file mapping
		Priority: High

		System file system does not have any caching algorithm for virtual mapping,
		so in this case the file must be loaded in memory, otherwise we get ring 
		buffer issue with drive timings (it's can be longer than 100ms).

		Current state: processing
		#############################################
	*/
	if (!pMapper->Open((const fr_utf8*)pResourceLinker, eReadFlag | eMustExistFlag)) return false;
	if (!pMapper->MapFile(FilePtr, 0, eMappingRead)) {
		pMapper->Close();
		return false;
	} 

	PtrSize = pMapper->GetSize();
	of = op_open_memory((fr_u8*)FilePtr, PtrSize, &ret);
	if (!of) {
		return false;
	}

	fr_f32* bufferFrames = (float*)FastMemAlloc(128 * 48 * sizeof(float) * sizeof(float));
	pcm_offset = (op_pcm_tell(of) - 48000);
	ret = op_read_float_stereo(of, bufferFrames, (128 * 48 * sizeof(float)));
	BugAssert(!(ret == OP_HOLE), "Corrupted OPUS file segment");
	if (ret <= 0) {
		FreeFastMemory(bufferFrames);
		return false;
	}

	li = op_current_link(of);
	head = op_head(of, li);

	formatOfFile.Bits = 32;
	formatOfFile.Index = 0;
	formatOfFile.Channels = head->channel_count;
	formatOfFile.IsFloat = true;
	formatOfFile.SampleRate = 48000;		// use full quality Opus

	if (op_seekable(of)) {
		formatOfFile.Frames = (fr_i32)op_pcm_total(of, li);
		tags = op_tags(of, li);
		BugAssert(!(ret = op_raw_seek(of, 0)), "Can't seek OPUS file");
		if (!!ret) {
			FreeFastMemory(bufferFrames);
			return false;
		}
	}

	FreeFastMemory(bufferFrames);
	return true;
}

bool 
COpusMediaResource::CloseResource()
{
	if (of) op_free(of);
	if (pMapper) {
		pMapper->UnmapFile(FilePtr); 
		pMapper->Close();
		FilePtr = nullptr;
	}

	return true;;
}

void 
COpusMediaResource::GetVendorName(const char*& vendorName)
{
	vendorName = "OPUS";
}

void 
COpusMediaResource::GetVendorString(const char*& vendorString)
{
	vendorString = "Opus decoder";
}

void 
COpusMediaResource::GetFormat(PcmFormat& format)
{
	format = formatOfFile;
}

void 
COpusMediaResource::SetFormat(PcmFormat outputFormat)
{
	this->outputFormat = outputFormat;
	resampler->Reset(outputFormat.Frames, formatOfFile.SampleRate, outputFormat.SampleRate, formatOfFile.Channels, !!outputFormat.Index);
}

fr_i64
COpusMediaResource::Read(fr_i64 FramesCount, fr_f32** ppFloatData)
{
	fr_i32 CopySize = 0;
	fr_i32 TempChannels = outputFormat.Channels;
	fr_i32 ret = 0;
	fr_i64 ret64 = 0;
	fr_i32 li = 0;
	const OpusHead* head = nullptr;
	const OpusTags* tags = nullptr;
	fr_i64 frame_out = 0;

	if (!TempChannels) return 0;
	if (!ppFloatData || !ppFloatData[0]) return 0;

	/* Translate current frames count for output buffer to file format frames count */
	CalculateFrames64(FramesCount, outputFormat.SampleRate, formatOfFile.SampleRate, frame_out);
	transferBuffers.Resize(formatOfFile.Channels, (fr_i32)max(FramesCount, frame_out));
	tempBuffer.Resize(OPUS_BUFFER);
	FileReadSize = (fr_i32)frame_out;
	while (FileReadSize) {
		/* The file can be corrupted, so we must to check it before read data */
		fr_i32 ptr_shift = ((fr_i32)frame_out - FileReadSize) * formatOfFile.Channels;
		ret = op_read_float(of, tempBuffer.Data() + ptr_shift, FileReadSize * formatOfFile.Channels, &li);
		if (ret == OP_HOLE) continue;
		else if (ret < 0) return -1;
		else if (!ret) {
			break;		// the end is here
		}

		/* If our new block has new channel count - we must to verify with new format. */
		if (li != previous_li) {
			head = op_head(of, li);
			formatOfFile.Channels = head->channel_count;
			previous_li = li;
		}

		FileReadSize -= ret;
	}

	LinearToPlanar(transferBuffers.GetBuffers(), tempBuffer.Data(), frame_out * formatOfFile.Channels, formatOfFile.Channels);

	/*	#############################################
		OPT 002: Perfomance issue with resampler
		Priority: High

		In this issue, we process file by resampler every time for every file.
		The mixer can process and mix data in one sample rate, and we resample
		data only on output.

		Current state: processing
		#############################################
	*/
	//BEGIN_TEST
	if (outputFormat.SampleRate != formatOfFile.SampleRate) {
		resampler->Resample((fr_i32)frame_out, transferBuffers.GetBuffers(), transferBuffers.GetBuffers());
	}
	//END_TEST

	/* if mono - set middle channels mode for stereo */
	if (formatOfFile.Channels == 1 && outputFormat.Channels >= 2) {
		for (size_t i = 0; i < 2; i++) {
			memcpy(ppFloatData[i], transferBuffers.GetBufferData(0), FramesCount * sizeof(fr_f32));
		}
	}
	else {
		for (fr_i64 i = 0; i < min(formatOfFile.Channels, outputFormat.Channels); i++) {
			memcpy(ppFloatData[i], transferBuffers.GetBufferData((fr_i32)i), FramesCount * sizeof(fr_f32));
		}
	}

	CalculateFrames64(frame_out - FileReadSize, formatOfFile.SampleRate, outputFormat.SampleRate, ret64);
	FSeek += frame_out - FileReadSize;
	if (ret64 < FramesCount) {
		for (fr_i64 i = 0; i < min(formatOfFile.Channels, outputFormat.Channels); i++) {
			memset(&ppFloatData[i][ret64], 0, (FramesCount - ret64) * sizeof(fr_f32));
		}
	}


	return ret64;
}

fr_i64
COpusMediaResource::ReadRaw(fr_i64 FramesCount, fr_f32** ppFloatData)
{
	return -1;
}

fr_i64 
COpusMediaResource::SetPosition(fr_i64 FramePosition)
{
	fr_i64 ret = 0;
	if (!op_seekable(of)) 
		return -1;
	fr_i64 frame_out = 0;
	ret = op_pcm_seek(of, FramePosition);
	if (ret == OP_EINVAL) {		// that means we are done
		FramePosition = 0;
		ret = op_pcm_seek(of, FramePosition);
	
	}
	BugAssert((!ret), "Can't seek OPUS file");

	FSeek = op_pcm_tell(of);
	return FSeek;// ret;
}

fr_i64
COpusMediaResource::GetPosition()
{
	return FSeek;	//#TODO:
}