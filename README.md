# Fresponze
Fast, simple and modern middleware for game engines

![Fresponze](https://github.com/suirless/Fresponze/blob/master/fresponze-suirless.png)

# Features
This library lets you write audio without any problems with system audio in C or C++ that
will compile and run on Windows, macOS and Linux. 

# Used libraries
* [Resonance Audio](https://github.com/resonance-audio/resonance-audio)
* [R8Brain](https://github.com/avaneev/r8brain-free-src)
* [Opus](https://github.com/xiph/opus)
* [Opusfile](https://github.com/xiph/opusfile)
* [OGG](https://github.com/xiph/ogg)
* [libnx](https://github.com/switchbrew/libnx)
* [ImGui (for examples)](https://github.com/ocornut/imgui)

# Audio backends
* WASAPI (95%, perfectly working)
* XAudio2 (10%, in development)
* ALSA (5%, in development)
* AAudio (0%, in development)
* AVAudioEngine (0%, in development)
* CoreAudio (0%, in development)
* PortAudio (0%, in development)

We don't want to add DirectSound, WinMM and etc. because...
* We don't like to support legacy code
* [PortAudio](http://www.portaudio.com/)

# Current state
In development, pre-alpha 

# License
Apache-2. See "LICENSE" file for more information.
