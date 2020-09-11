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
#include "FresponzeTypes.h"
#include <pthread.h>
#define ALIGN_SIZE(Size, AlSize)        ((Size + (AlSize-1)) & (~(AlSize-1)))
#define ALIGN_SIZE_64K(Size)            ALIGN_SIZE(Size, 65536)
#define ALIGN_SIZE_16(Size)             ALIGN_SIZE(Size, 16)

fr_i64
DebugStamp()
{
    return 0;
}

bool
Fresponze::InitMemory()
{
    return true;
}

void
Fresponze::DestroyMemory()
{

}

void*
FastMemAlloc(
        fr_i32 SizeToAllocate
)
{
    return malloc(SizeToAllocate);
}

void*
FastMemRealloc(
        void* Ptr,
        fr_i32 SizeToAllocate
)
{
    return realloc(Ptr, SizeToAllocate);
}


void*
VirtMemAlloc(
        fr_i64 SizeToAllocate
)
{
    return nullptr;
}

void
FreeFastMemory(
        void* Ptr
)
{
    free(Ptr);
}

void
FreeVirtMemory(
        void* Ptr,
        size_t Size
)
{

}

void*
GetMapFileSystem()
{
    return nullptr;
}

CPosixEvent::CPosixEvent()
{
    m_id.signaled = false;
    pthread_mutex_init(&m_id.mutex, nullptr);
    pthread_cond_init(&m_id.cond, nullptr);
}

CPosixEvent::~CPosixEvent()
{
    pthread_mutex_destroy(&m_id.mutex);
    pthread_cond_destroy(&m_id.cond);
}

void
CPosixEvent::Raise()
{
    pthread_mutex_lock(&m_id.mutex);
    pthread_cond_signal(&m_id.cond);
    m_id.signaled = true;
    pthread_mutex_unlock(&m_id.mutex);
}

void
CPosixEvent::Reset()
{
    pthread_mutex_lock(&m_id.mutex);
    pthread_cond_signal(&m_id.cond);
    m_id.signaled = false;
    pthread_mutex_unlock(&m_id.mutex);
}

void
CPosixEvent::Wait()
{
    pthread_mutex_lock(&m_id.mutex);

    while (!m_id.signaled) {
        pthread_cond_wait(&m_id.cond, &m_id.mutex);
    }

    m_id.signaled = false; // due in WaitForSingleObject() "Before returning, a wait function modifies the state of some types of synchronization"
    pthread_mutex_unlock(&m_id.mutex);
}

bool
CPosixEvent::Wait(
        fr_i32 TimeToWait
)
{
    bool result = true;
    pthread_mutex_lock(&m_id.mutex);

    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += (long) TimeToWait * 1000 * 1000;
    if (ts.tv_nsec > 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }

    while (!m_id.signaled) {
        int res = pthread_cond_timedwait(&m_id.cond, &m_id.mutex, &ts);
        if (res == 110) {
            result = m_id.signaled;
            break;
        }
    }

    m_id.signaled = false;
    pthread_mutex_unlock(&m_id.mutex);
    return result;
}

bool
CPosixEvent::IsRaised()
{
    return false;
}
