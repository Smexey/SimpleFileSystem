// File: fs.h
#pragma once
#include "KernPart.h"
#include "criticalsection.h"
#include "fs.h"

class KernFS {
    KernPart* kernpart;
    CRITICAL_SECTION cs;
    HANDLE mountedsem;

protected:
    KernFS(const KernFS&);             // Prevent construction by copying
    KernFS& operator=(const KernFS&);  // Prevent assignment
public:

    KernFS();
    ~KernFS();

    char mount(Partition* partition);  // montira particiju
    // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
    char unmount();  // demontira particiju
                     // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
    char format();   // formatira particiju;
                     // vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
    FileCnt readRootDir();
    // vraca -1 u slucaju neuspeha ili broj fajlova u slucaju uspeha
    char doesExist(char* fname);  // argument je naziv fajla sa
                                  // apsolutnom putanjom

    File* open(char* fname, char mode);
    char deleteFile(char* fname);
};