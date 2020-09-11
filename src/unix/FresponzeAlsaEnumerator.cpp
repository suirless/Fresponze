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
#include "FresponzeAlsaEnumerator.h"

CAlsaAudioEnumerator::CAlsaAudioEnumerator()
{
    AddRef();
}

CAlsaAudioEnumerator::~CAlsaAudioEnumerator()
{

}

void
GetControlName(char* controlName,
               char* deviceName) {
    // Example
    // deviceName: "front:CARD=Intel,DEV=0"
    // controlName: "hw:CARD=Intel"
    char* pos1 = strchr(deviceName, ':');
    char* pos2 = strchr(deviceName, ',');
    if (!pos2) {
        // Can also be default:CARD=Intel
        pos2 = &deviceName[strlen(deviceName)];
    }
    if (pos1 && pos2) {
        strcpy(controlName, "hw");
        int nChar = (int)(pos2 - pos1);
        strncpy(&controlName[2], pos1, nChar);
        controlName[2 + nChar] = '\0';
    } else {
        strcpy(controlName, deviceName);
    }
}

bool
CAlsaAudioEnumerator::EnumerateDevices()
{
    int CurIndex = 0;
    int CardIndex = -1;
    const char* TypeOfElems = "Output";
    const char* PrefixElement = "dsnoop:";

    if (OutputDevicesInfo) {
        FreeFastMemory(OutputDevicesInfo);
        OutputDevicesInfo = nullptr;
    }

    while (!snd_card_next(&CardIndex) && (CardIndex >= 0)) {
        fr_ptr* ppDeviceInfo = nullptr;
        if (snd_device_name_hint(CardIndex, "pcm", &ppDeviceInfo) != 0) {
            continue;
        }

        for (fr_ptr* ppList = ppDeviceInfo; *ppList != nullptr; ++ppList) {
            char* pActualType = snd_device_name_get_hint(*ppList, "IOID");
            if (pActualType) {
                bool isNotNeedyDevice = (strcmp(pActualType, TypeOfElems) != 0);
                free(pActualType);
                if (isNotNeedyDevice) {
                    continue;
                }
            }

            char* pDeviceName = snd_device_name_get_hint(*ppList, "NAME");
            if (!pDeviceName) {
                continue;
            }

            free(pDeviceName);
            this->OutputDevices++;
        }
    }

    CardIndex = -1;
    OutputDevicesInfo = (EndpointInformation*)FastMemAlloc(sizeof(EndpointInformation) * this->OutputDevices);

    while (!snd_card_next(&CardIndex) && (CardIndex >= 0)) {
        fr_ptr* ppDeviceInfo = nullptr;
        if (snd_device_name_hint(CardIndex, "pcm", &ppDeviceInfo) != 0) {
            continue;
        }

        for (fr_ptr* ppList = ppDeviceInfo; *ppList != NULL; ++ppList) {
            char* pActualType = snd_device_name_get_hint(*ppList, "IOID");
            if (pActualType) {
                bool isNotNeedyDevice = (strcmp(pActualType, TypeOfElems) != 0);
                free(pActualType);
                if (isNotNeedyDevice) {
                    continue;
                }
            }

            char* pDeviceName = snd_device_name_get_hint(*ppList, "NAME");
            char* pDeviceDesc = nullptr;
            if (!pDeviceName) {
                continue;
            }

            if (strcmp(pDeviceName, "default") != 0 && strcmp(pDeviceName, "null") != 0 && strcmp(pDeviceName, "pulse") != 0 &&
                strncmp(pDeviceName, PrefixElement, strlen(PrefixElement)) != 0) {
                pDeviceDesc = snd_device_name_get_hint(*ppList, "DESC");
                if (!pDeviceDesc) {
                    pDeviceDesc = pDeviceName;
                }
            }

            if (!pDeviceDesc) pDeviceDesc = pDeviceName;
            OutputDevicesInfo[CurIndex].EndpointId = CurIndex;
            OutputDevicesInfo[CurIndex].Type = RenderType;
            strcpy(OutputDevicesInfo[CurIndex].EndpointName, pDeviceDesc);
            strcpy(OutputDevicesInfo[CurIndex].EndpointUUID, pDeviceName);
            CurIndex++;
        }
    }

    for (int i = 0; i < this->OutputDevices; ++i) {
        printf("Device %i: %s\n", i, OutputDevicesInfo[i].EndpointUUID);
    }

    return true;
}

bool
CAlsaAudioEnumerator::GetDevicesCount(fr_i32 EndpointType, fr_i32& Count)
{
    Count = this->OutputDevices;
    return true;
}

void
CAlsaAudioEnumerator::GetInputDeviceList(EndpointInformation*& InputDevices)
{
    InputDevices = InputDevicesInfo;
}

void
CAlsaAudioEnumerator::GetOutputDeviceList(EndpointInformation*& OutputDevices)
{
    OutputDevices = OutputDevicesInfo;
}

bool
CAlsaAudioEnumerator::GetDefaultDevice(fr_i32 EndpointType, IAudioEndpoint*& pOutDevice)
{
    if (EndpointType != RenderType) return false;
    int ret = snd_pcm_open(&pOutputDevice, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (ret == -EBUSY) {
        for (int i = 0; i < 50; i++) {
            usleep(10000);
            ret = snd_pcm_open(&pOutputDevice, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
            if (ret == 0) {
                break;
            }
        }
    } else if (ret != 0) {
        return false;
    }

    pOutDevice = new CAlsaAudioEndpoint( RenderType, pOutputDevice);
    return true;
}

bool
CAlsaAudioEnumerator::GetDeviceById(fr_i32 EndpointType, fr_i32 DeviceId, IAudioEndpoint*& pOutDevice)
{
    return false;
}

bool
CAlsaAudioEnumerator::GetDeviceByUUID(fr_i32 EndpointType, char* DeviceUUID, IAudioEndpoint*& pOutDevice)
{
    size_t string_size = strlen(DeviceUUID);
    if (string_size >= sizeof(ControlName)) {
        return false;
    }

    if (pMixer) {
        snd_mixer_free(pMixer);
        snd_mixer_detach(pMixer, ControlName);
        pMixer = nullptr;
    }

    if (snd_mixer_open(&pMixer, 0) < 0) {
        return false;
    }

    GetControlName(ControlName, DeviceUUID);
    if (snd_mixer_attach(pMixer, ControlName) < 0) {
        memset(ControlName, 0, sizeof(ControlName));
        snd_mixer_close(pMixer);
        return false;
    }

    if (snd_mixer_selem_register(pMixer, nullptr, nullptr) < 0) {
        snd_mixer_detach(pMixer, ControlName);
        snd_mixer_close(pMixer);
        return false;
    }

    //#TODO: Fix opening device by name
    snd_pcm_t* p_device = nullptr;
    int ret = snd_pcm_open(&p_device, DeviceUUID, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (ret == -EBUSY) {
        for (int i = 0; i < 50; i++) {
            usleep(10000);
            ret = snd_pcm_open(&pOutputDevice, DeviceUUID, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
            if (ret == 0) {
                break;
            }
        }
    } else if (ret != 0) {
        snd_mixer_free(pMixer);
        snd_mixer_detach(pMixer, ControlName);
        snd_mixer_close(pMixer);
        return false;
    }

    if (!p_device) {
        return  false;
    }

    pOutDevice = new CAlsaAudioEndpoint(RenderType, pOutputDevice);
    return true;
}
