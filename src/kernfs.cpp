#include "kernfs.h"
#include <stdio.h>
#include <iostream>
#include "File.h"
#include "KernPart.h"
#include "synch.h"

Entry::Entry(const Entry& ent) {
    strncpy(name, ent.name, fileNameLen);
    strncpy(ext, ent.ext, fileExtLen);

    size = ent.size;
    indexCluster = ent.indexCluster;
    reserved = ent.reserved;
}

ostream& operator<<(ostream& os, const Entry& e) {
    return os << e.name[fileNameLen] << "." << e.ext[fileExtLen] << " size:" << e.size;
}

bool Entry::nameEq(char* fname) {
    size_t k = 0;
    bool help = false;
    for (size_t i = 0; i < fileNameLen; i++) {
        if (i < strlen(fname) && fname[i] == '.') {
            help = true;
            k = i + 1;
        }
        if (!help && name[i] != fname[i]) return false;
        if (help && name[i] != ' ') return false;
    }
    help = false;
    for (size_t i = 0; i < fileExtLen; i++) {
        if (k >= strlen(fname)) return false;
        if (ext[i] != fname[k++]) return false;
    }
    return true;
}

KernFS::KernFS() : kernpart(nullptr) {
    mountedsem = CreateSemaphore(0, 1, 1, NULL);  // samo jedan montira
    mutex = CreateSemaphore(0, 1, 1, 0);
}

KernFS::~KernFS() {
    unmount();
    // ReleaseSemaphore(mountedsem);
}

char KernFS::mount(Partition* partition) {
    CriticalSection lck(mutex);
    if (partition == nullptr) return 0;
    signalAndWait(mutex, mountedsem);
    wait(mutex);

    kernpart = new KernPart(partition);
    return 0;
}

char KernFS::unmount() {
    CriticalSection lck(mutex);

    if (kernpart == nullptr) return 0;

    if (!kernpart->mark()) {
        return 0;
    }

    delete kernpart;
    kernpart = nullptr;
    signal(mountedsem);
    return 0;
}

char KernFS::format() {
    CriticalSection lck(mutex);
    if (kernpart == nullptr) return 0;
    return kernpart->format();
}

FileCnt KernFS::readRootDir() {
    CriticalSection lck(mutex);
    if (kernpart == nullptr) return 0;
    return kernpart->readRootDir();
}

char KernFS::doesExist(char* fname) {
    CriticalSection lck(mutex);
    if (kernpart == nullptr) return 0;
    return kernpart->doesExistRootDir(fname);
}

File* KernFS::open(char* fname, char mode) {
    CriticalSection lck(mutex);
    if (kernpart == nullptr) return nullptr;
    return kernpart->openFile(fname, mode);
}
char KernFS::deleteFile(char* fname) {
    CriticalSection lck(mutex);
    if (kernpart == nullptr) return 0;
    return kernpart->deleteFile(fname);
}
