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
    int ret = 0;//snd_pcm_set_params(pAlsaHandle, DeviceSampleFormat, SND_PCM_ACCESS_RW_INTERLEAVED, DeviceFormat.Channels, DeviceFormat.SampleRate, 1, DeviceLatency);

    snd_pcm_hw_params_t* Params = nullptr;
    snd_pcm_hw_params_alloca(&Params);
    snd_pcm_hw_params_any(pAlsaHandle, Params);
    snd_pcm_hw_params_set_access(pAlsaHandle, Params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pAlsaHandle, Params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(pAlsaHandle, Params, 2);
    snd_pcm_hw_params_set_buffer_size(pAlsaHandle, Params, DeviceFormat.Frames);

    unsigned int val = DeviceFormat.SampleRate;
    int dir = 0;
    ret = snd_pcm_hw_params_set_rate_near(pAlsaHandle, Params, &val, &dir);
    if (ret < 0) {
        return false;
    }

    ret = snd_pcm_hw_params(pAlsaHandle, Params);
    if (ret < 0) {
        return false;
    }

    snd_pcm_hw_params_get_rate(Params, &val, &dir);
    DeviceFormat.SampleRate = val;
    snd_pcm_hw_params_get_channels(Params, &val);
    DeviceFormat.Channels = val;

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

bool
CAlsaAudioEndpoint::PullData(fr_f32* pData, fr_i32 DataSize)
{
    fr_i32 OutputDataSize = snd_pcm_writei(pAlsaHandle, pData, DataSize);
    if (OutputDataSize < 0) {
        OutputDataSize = snd_pcm_recover(pAlsaHandle, OutputDataSize, 0);
        if (OutputDataSize < 0) return false;
    }

    return true;
}

void
CAlsaAudioEndpoint::ThreadProc()
{
    if (!SetDeviceFormat(EndpointInfo.EndpointFormat)) return;
    auto pOutPtr = (fr_f32*)FastMemAlloc(sizeof(fr_f32) * EndpointInfo.EndpointFormat.Frames * EndpointInfo.EndpointFormat.Channels);
    if (pAudioCallback) {
        pAudioCallback->FlushCallback();
        pAudioCallback->FormatCallback(&EndpointInfo.EndpointFormat);
    }

    while (!pSyncEvent->IsRaised()) {
        if (!pAudioCallback) {
            pSyncEvent->Wait(100);
            TypeToLog("ALSA: No audio callback. Waiting for pointer...");
            continue;
        }

        fr_i32 AvailableFrames = EndpointInfo.EndpointFormat.Frames;
        fr_i32 ErrorCallback = pAudioCallback->EndpointCallback(pOutPtr, AvailableFrames, EndpointInfo.EndpointFormat.Channels, EndpointInfo.EndpointFormat.SampleRate, RenderType);
        if (ErrorCallback != 0) {
            TypeToLog("ALSA: Putting empty buffer to output");
            continue;
        }

        fr_i8* TempPtr = (fr_i8*)pOutPtr;
        while (AvailableFrames) {
            int err = 0;
            while ((err = snd_pcm_writei(pAlsaHandle, TempPtr, AvailableFrames) < 0)) {
                if (err == -EBADFD) {
                    err = snd_pcm_recover(pAlsaHandle, err, 0);//snd_pcm_prepare(pAlsaHandle);
                    if (err < 0) {
                        goto endThread;
                    }
                }
            }

            AvailableFrames -= err;
            TempPtr += err * EndpointInfo.EndpointFormat.Channels * sizeof(fr_f32);

        }

        /*
        while (AvailableFrames > 0) {
            fr_i32 TimeToSleep = (((double)AvailableFrames * 500.) / (double)EndpointInfo.EndpointFormat.SampleRate) ;
            int err = snd_pcm_writei(pAlsaHandle, TempPtr, AvailableFrames);
            if (err < 0) {
                if (err == -EAGAIN) {
                    usleep(1000);
                    continue;
                }

                err = snd_pcm_recover(pAlsaHandle, err, 0);//snd_pcm_prepare(pAlsaHandle);
                if (err < 0) {
                    goto endThread;
                }

                continue;
            } else if (err == 0) {
                usleep(TimeToSleep * 1000);
                continue;
            }

            //AvailableFrames -= AvailableFrames;
            TempPtr += err * sizeof(fr_f32);
            AvailableFrames -= err;
        }
         */
    }

    endThread:
    FreeFastMemory(pOutPtr);
    Close();
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
    pthread_t tid = 0;
    pthread_attr_t attr = {};
    pthread_attr_init(&attr);

    auto CallbackFunction = [](void* pThis) -> void* {
        auto Endpoint = (CAlsaAudioEndpoint*)pThis;
        Endpoint->ThreadProc();
        pthread_exit(0);
    };

    ThreadHandle = pthread_create(&tid, &attr, CallbackFunction, this);
    if (ThreadHandle < 0) {
        return false;
    }

    return true;
}

bool
CAlsaAudioEndpoint::Stop()
{
    return false;
}
