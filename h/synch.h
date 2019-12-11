#pragma once
#include "Windows.h"

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)
#define SignalAndWait(x,y) SignalObjectAndWait(x,y,INFINITE, false);

class CriticalSectionLock {
    CRITICAL_SECTION& criticalSection;

public:
    CriticalSectionLock(CRITICAL_SECTION& cs) : criticalSection(cs) {
        EnterCriticalSection(&criticalSection);
    }
    ~CriticalSectionLock() { LeaveCriticalSection(&criticalSection); }
};
