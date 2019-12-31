#pragma once
#include "KernPart.h"
#include "fs.h"
#include "part.h"

typedef unsigned long ClusterNo;
const ClusterNo clusIndNum = 512ul;
const ClusterNo nullClusterNo = -1;  // init values
const ClusterNo singleTableInd = clusIndNum / 2;
class File;

class KernFile {
    friend class KernPart;
    // FileControl* fc;
    char mode;
    KernPart* part;
    BytesCnt size;

    // cursors
    BytesCnt curr = 0;
    BytesCnt currClus = 0;
    BytesCnt currOffs = 0;

    // current data offset in clusters
    ClusterNo currentDataClusOffs = nullClusterNo;
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

    // returns true if rd/wr
    bool readByte(char* where);
    bool writeByte(char* ch);

    void setcurr(BytesCnt n) {
        curr = n;
        currClus = curr / ClusterSize;
        currOffs = curr % ClusterSize;
    }

public:
    FileControl* fc;
    // KernFile();
    KernFile(FileControl* fc, KernPart* p, char m);

    ~KernFile();

    char write(BytesCnt, char* buffer);
    BytesCnt read(BytesCnt, char* buffer);
    char seek(BytesCnt);
    BytesCnt filePos();
    char eof();
    BytesCnt getFileSize();
    char truncate();
};