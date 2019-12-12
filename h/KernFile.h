#pragma once
#include "KernPart.h"
#include "file.h"
#include "fs.h"

class KernFile {
    constexpr static ClusterNo nullClusterNo = -1;  // init values

    char mode;
    KernPart* part;

    //cursors
    BytesCnt curr;
    BytesCnt currClus;
    BytesCnt currOffs;

    // current data cluster offset
    ClusterNo currentData = nullClusterNo;
    // indexes of chached blocks
    ClusterNo dataCacheClusNum = nullClusterNo;
    ClusterNo helpCacheClusNum = nullClusterNo;
    ClusterNo rootCacheClusNum = nullClusterNo;

    // caches
    char rootCache[ClusterSize];
    char helpCache[ClusterSize];  // dynamic? smara me
    char dataCache[ClusterSize];

    bool dirtyRoot;
    bool dirtyHelp;
    bool dirtyData;

    void readByte(char * where);
    void writeByte(char * ch);
public:
    KernFile(KernPart* p, char m);

    ~KernFile();

    char write(BytesCnt, char* buffer);
    BytesCnt read(BytesCnt, char* buffer);
    char seek(BytesCnt);
    BytesCnt filePos();
    char eof();
    BytesCnt getFileSize();
    char truncate();
};