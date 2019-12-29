#include "kernfs.h"

KernFS::KernFS() {
    InitializeCriticalSection(&cs);
    mountedsem = CreateSemaphore(NULL, 1, 1, NULL);  // samo jedan montira
}

KernFS::~KernFS() { unmount(); }

char KernFS::mount(Partition *partition) {
    if (partition == 0) {
        return 0;
    }
    wait(mountedsem);

    CriticalSectionLock lck(cs);

    kernpart = new KernPart(partition);
    return 0;
}

char KernFS::unmount() {
    if (kernpart == nullptr) return 0;

    // synch
    CriticalSectionLock lck(cs);
    kernpart->saveandclose();

    
    delete kernpart;
    kernpart = nullptr;
    signal(mountedsem);
    return 0;
}

char KernFS::format() {
    if (kernpart == nullptr) return 0;
}

FileCnt KernFS::readRootDir() {
    if (kernpart == nullptr) return 0;
}

char KernFS::doesExist(char *fname) {
    if (kernpart == nullptr) return 0;

    return 0;
}

File *KernFS::open(char *fname, char mode) { return nullptr; }
char KernFS::deleteFile(char *fname) {
    if (kernpart == nullptr) return 0;

    return 0;
}
