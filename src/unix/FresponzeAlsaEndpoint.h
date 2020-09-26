#include "FresponzeTypes.h"
#include "FresponzeEndpoint.h"
#include "asoundlib.h"
#include <sys/time.h>

class CAlsaAudioEndpoint final : public IAudioEndpoint
{
private:
    int ThreadHandle = 0;
    snd_pcm_t* pAlsaHandle = nullptr;
    snd_pcm_sframes_t BufferFrames = 0;
    fr_f32 DelayCustom = 0.f;

    bool SetDeviceFormat(PcmFormat DeviceFormat);
    bool PullData(fr_f32* pData, fr_i32 DataSize);

public:
    CAlsaAudioEndpoint(fr_i32 DeviceType, void* pDeviceContext, PcmFormat DeviceFormat = {})
    {
        AddRef();
        pSyncEvent = new CPosixEvent;
        pStartEvent = new CPosixEvent;
        pAlsaHandle = (snd_pcm_t*)pDeviceContext;
        EndpointInfo.Type = DeviceType;
        if (!DeviceFormat.Bits) {
            DeviceFormat.Bits = 32;
            DeviceFormat.IsFloat = true;
            DeviceFormat.Channels = 2;
            DeviceFormat.SampleRate = 44100;
            DeviceFormat.Frames = 4410;
            DeviceFormat.Index = -1;
        }

        EndpointInfo.EndpointFormat = DeviceFormat;
    }

    ~CAlsaAudioEndpoint() override
    {
        Close();
    }

    void GetDeviceFormat(PcmFormat& pcmFormat) override { pcmFormat = EndpointInfo.EndpointFormat; }
    void GetDevicePointer(void*& pDevice) override;
    void ThreadProc();
    void SetDeviceInfo(EndpointInformation& DeviceInfo) override;
    void GetDeviceInfo(EndpointInformation& DeviceInfo) override;
    void SetCallback(IAudioCallback* pCallback) override;
    bool Open(fr_f32 Delay) override;
    bool Close() override;
    bool Start() override;
    bool Stop() override;
};
