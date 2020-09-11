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
#include "FresponzeAlsaEndpoint.h"

inline
void
CopyDataToBuffer(
        float* FileData,
        void* OutData,
        size_t FramesCount,
        bool IsFloat,
        int Bits,
        int Channels
)
{
    size_t sizeToRead = FramesCount * Channels;

    if (IsFloat) {
        memcpy(OutData, FileData, sizeToRead * sizeof(fr_f32));
    } else  {
        short* pShortData = (short*)FileData;
        switch (Bits) {
            case 16:
                for (size_t i = 0; i < sizeToRead; i++) {
                    pShortData[i] = maxmin(((int)(FileData[i] * 32768.0f)), -32768, 32767);
                }
                break;
            default:
                break;
        }
    }
}

inline
void
CopyDataFromBuffer(
        void* FileData,
        float* OutData,
        size_t FramesCount,
        bool IsFloat,
        int Bits,
        int Channels
)
{
    size_t sizeToRead = FramesCount * Channels;

    if (IsFloat) {
        memcpy(OutData, FileData, sizeToRead * sizeof(fr_f32));
    }
    else {
        short* pShortData = (short*)FileData;
        switch (Bits) {
            case 16:
                for (size_t i = 0; i < sizeToRead; i++) {
                    OutData[i] = (float)pShortData[i] < 0 ? pShortData[i] / 32768.0f : pShortData[i] / 32767.0f;
                }
                break;
            default:
                break;
        }
    }
}

fr_i32
GetDeviceLatency(fr_i32 SampleRate, fr_i32 FramesCount)
{
    return ((double)FramesCount / (double)SampleRate) * 10000000.;
}

bool CAlsaAudioEndpoint::SetDeviceFormat(PcmFormat DeviceFormat)
{
    snd_pcm_format_t DeviceSampleFormat = (DeviceFormat.Bits == 16 ? SND_PCM_FORMAT_S16_LE : (DeviceFormat.Bits == 32 ? (SND_PCM_FORMAT_FLOAT_LE) : SND_PCM_FORMAT_FLOAT_LE));
    fr_i32 DeviceLatency = GetDeviceLatency(DeviceFormat.SampleRate, DeviceFormat.Frames);
    int ret = snd_pcm_set_params(pAlsaHandle, DeviceSampleFormat, SND_PCM_ACCESS_RW_INTERLEAVED, DeviceFormat.Channels, DeviceFormat.SampleRate, 1, DeviceLatency);
    if (ret < 0) {
        return false;
    }

    EndpointInfo.EndpointFormat = DeviceFormat;
    return true;
}

void
CAlsaAudioEndpoint::GetDevicePointer(
        void*& pDevice
)
{
    pDevice = pAlsaHandle;
}

void
CAlsaAudioEndpoint::ThreadProc()
{
    if (pAudioCallback) {
        pAudioCallback->FlushCallback();
        pAudioCallback->FormatCallback(&fmtToPush);
    }


}

void
CAlsaAudioEndpoint::SetDeviceInfo(EndpointInformation& DeviceInfo)
{
    memcpy(&EndpointInfo, &DeviceInfo, sizeof(EndpointInformation));
}

void
CAlsaAudioEndpoint::GetDeviceInfo(EndpointInformation& DeviceInfo)
{
    memcpy(&DeviceInfo, &EndpointInfo, sizeof(EndpointInformation));
}

void
CAlsaAudioEndpoint::SetCallback(IAudioCallback* pCallback)
{
    pCallback->Clone((void**)&pAudioCallback);
}

bool
CAlsaAudioEndpoint::Open(fr_f32 Delay)
{
    DelayCustom = Delay;
    if (!Start()) return false;
    return true;
}

bool
CAlsaAudioEndpoint::Close()
{
    Stop();
    return true;
}

bool
CAlsaAudioEndpoint::Start()
{
    return false;
}

bool
CAlsaAudioEndpoint::Stop()
{
    return false;
}
