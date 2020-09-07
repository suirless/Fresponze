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

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS_PLATFORM 1
#endif

#ifdef WINDOWS_PLATFORM
#ifdef DLL_PLATFORM
#ifdef LIB_EXPORTS
#define FRAPI __declspec(dllexport)
#else
#define FRAPI __declspec(dllimport)
#endif
#else 
#define FRAPI
#endif
#else
#define FRAPI
#endif

enum FresponzePlatforms : unsigned long {
	/* Public platforms */
	eWindowsPlatform,
	eLinuxPlatform,
	eMacOSPlatform,
	eSwitchPlatform,
	eAndroidPlatform,
	eIOSPlatform,

	/* Private platforms */
	ePlayStation4Platform,
	ePlayStation5Platform,
	eXboxOnePlatform,
	eXboxSXPlatform
};

enum FresponzeHosts : unsigned long {
	eWindowsCoreHost = 1 << 1,			// WASAPI host
	eXboxCoreHost = 1 << 2,				// WASAPI host without enumerator
	eXboxSXCoreHost = 1 << 3,			// WASAPI host with hardware acceleration (Xbox Series X only)
	eXAudio2Host = 1 << 4,				// XAudio2 host using software mixer
	eXAudio2AdvancedHost = 1 << 5,		// XAudio2 host using hardware acceleration (if supported)
	eALSAHost = 1 << 6,					// ALSA Mixer host
	eCoreAudioHost = 1 << 7,			// macOS Core Audio host
	eLibnxHost = 1 << 8,				// Nintendo Switch libnx voice system host
	eOpenSLESHost = 1 << 9,				// Android legacy host
	eAAudioHost = 1 << 10,				// Android new host
	eAVAudioSessionHost = 1 << 11,		// iOS AVAudioSession host
	ePlayStation4Host = 1 << 12,		// PlayStation 4 host
	ePlayStation5Software = 1 << 13,	// PlayStation 5 host using software mixer
	ePlayStation5Hardware = 1 << 14		// PlayStation 5 host using hardware processing (HRTF, Filters, etc...)
};

#define CheckFlag(x, y) (!(x & y))

#ifdef WINDOWS_PLATFORM
#include <windows.h>
#define CURRENT_PLATFORM eWindowsPlatform
#define SUPPORTED_HOSTS eWindowsCoreHost | eXAudio2Host | eXAudio2AdvancedHost 
#elif defined(LINUX_PLATFORM)
#define CURRENT_PLATFORM eLinuxPlatform
#define SUPPORTED_HOSTS eALSAHost 
#elif defined(MACOS_PLATFORM)
#define CURRENT_PLATFORM eMacOSPlatform
#define SUPPORTED_HOSTS eCoreAudioHost 
#elif defined(SWITCH_PLATFORM)
#define CURRENT_PLATFORM eSwitchPlatform
#define SUPPORTED_HOSTS eLibnxHost 
#elif defined(ANDROID_PLATFORM)
#define CURRENT_PLATFORM eAndroidPlatform
#define SUPPORTED_HOSTS eAAudioHost | eOpenSLESHost
#elif defined(IOS_PLATFORM)
#define CURRENT_PLATFORM eIOSPlatform
#define SUPPORTED_HOSTS eAVAudioSessionHost 
#elif defined(PS4_PLATFORM)
#define CURRENT_PLATFORM ePlayStation4Platform
#define SUPPORTED_HOSTS ePlayStation4Host 
#elif defined(PS5_PLATFORM)
#define CURRENT_PLATFORM ePlayStation5Platform
#define SUPPORTED_HOSTS ePlayStation5Software | ePlayStation5Hardware
#elif defined(XBOXONE_PLATFORM)
#define CURRENT_PLATFORM eXboxOnePlatform
#define SUPPORTED_HOSTS eXboxCoreHost | eXAudio2Host | eXAudio2AdvancedHost 
#elif defined(XBOXSX_PLATFORM)
#define CURRENT_PLATFORM eXboxSXPlatform
#define SUPPORTED_HOSTS eXboxCoreHost | eXboxSXCoreHost | eXAudio2Host | eXAudio2AdvancedHost 
#endif

#define FRESPONZE_VERSION_MAJOR 0
#define FRESPONZE_VERSION_MINOR 400		
#define FRESPONZE_VERSION_INDEX 842

#define FRESPONZE_USE_LIBSNDFILE 0
#define FRESPONZE_USE_OPUS 1
#define FRESPONZE_USE_VORBIS 1

#if ((FRESPONZE_USE_OPUS == 1) || (FRESPONZE_USE_VORBIS == 1))
#define FRESPONZE_USE_OGG 1
#else
#define FRESPONZE_USE_OGG 0
#endif
