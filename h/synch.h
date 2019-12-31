#pragma once
#include <iostream>
#include "Windows.h"
using namespace std;

#define signal(x) ReleaseSemaphore(x, 1, NULL)
#define wait(x) WaitForSingleObject(x, INFINITE)
#define signalAndWait(x, y) SignalObjectAndWait(x, y, INFINITE, false);
typedef HANDLE Sem;

class CriticalSection {
public:
    CriticalSection(Sem s) {
        sem = s;
        wait(sem);
    }
    ~CriticalSection() { signal(sem); }

private:
    Sem sem;
};
