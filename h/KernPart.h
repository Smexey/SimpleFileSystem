#pragma once
#include <list>
#include <map>
#include <string>
#include "BitVect.h"
#include "FS.h"
#include "KernFS.h"
#include "part.h"
#include "string"
#include "synch.h"

typedef unsigned long EntryNum;
const unsigned long ClusterEntryNo = 2048 / 32;
const unsigned long ClusterDataNo = 2048;
const unsigned long ClusterRootNo = 512;

class FileControl {
    friend class KernPart;
    friend class KernFile;
    unsigned long writers;
    unsigned long readers;
    unsigned long waitWriters;
    unsigned long waitReaders;
    Sem writeSem;
    Sem readSem;
    string fname;

    Entry entry;
    ClusterNo dirPos;
    ClusterNo dirOff;
    FileControl(char mode, char*, Entry*, ClusterNo, BytesCnt);

    Entry* getEntry() { return &entry; }
};

class KernPart {
private:
    friend class KernFile;
    Partition* part;

public:
    Sem myPartSem;
    Sem deleteSem;

    bool marked = false;
    map<string, FileControl*> mapOpen;
    BitVector* bitVect;
    char rootCache[ClusterSize];

    KernPart(Partition* p);
    ~KernPart();
    virtual int readCluster(ClusterNo, char* buffer);
    virtual ClusterNo getNumOfClusters() const;
    virtual int writeCluster(ClusterNo, const char* buffer);
    bool doesExistRootDir(char* fname);
    FileControl* fileControlFromRootDir(char* fname, char);

    void close(KernFile* kernfile);

    FileControl* makeNewFile(char* fname, char mode);
    void writeEntryToBuffer(char*, char*, ClusterNo, ClusterNo);
    File* openFile(char*, char);
    ClusterNo newClusterFS(char*, char*, ClusterNo, ClusterNo);

    bool deleteFile(char*);

    bool freeEntry(Entry*);

    char readRootDir();

    void freeCluster(ClusterNo);
    ClusterNo getNewEmpty();

    char format();
    bool mark();
};
