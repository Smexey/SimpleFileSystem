#pragma once
#include "KernPart.h"
#include "file.h"
#include "fs.h"

class KernFile {
    constexpr static ClusterNo nullClusterNo = -1;  // init values

    char mode;
    KernPart* part;
    BytesCnt size;

    // cursors
    BytesCnt curr = 0;
    BytesCnt currClus = 0;
    BytesCnt currOffs = 0;

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

    bool dirtyRoot = false;
    bool dirtyHelp = false;
    bool dirtyData = false;

    void readByte(char* where);
    void writeByte(char* ch);

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