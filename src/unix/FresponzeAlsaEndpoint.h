#include "FresponzeTypes.h"
#include "FresponzeEndpoint.h"
#include "asoundlib.h"
#include <sys/time.h>

class CAlsaAudioEndpoint final : public IAudioEndpoint
{
private:
    snd_pcm_t* pAlsaHandle = nullptr;
    snd_pcm_sframes_t BufferFrames = 0;
    fr_f32 DelayCustom = 0.f;

    bool SetDeviceFormat(PcmFormat DeviceFormat);

public:
    CAlsaAudioEndpoint(fr_i32 DeviceType, void* pDeviceContext, PcmFormat DeviceFormat = {})
    {
        AddRef();
        pAlsaHandle = (snd_pcm_t*)pDeviceContext;
        EndpointInfo.Type = DeviceType;
        if (!DeviceFormat.Bits) {
            DeviceFormat.Bits = 32;
            DeviceFormat.IsFloat = true;
            DeviceFormat.Channels = 2;
            DeviceFormat.SampleRate = 48000;
            DeviceFormat.Frames = 4800;
            DeviceFormat.Index = -1;
            SetDeviceFormat(DeviceFormat);
        }
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
