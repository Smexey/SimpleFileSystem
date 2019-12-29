#pragma once
#include "Part.h"
#include "bitvect.h"
// #include "part.h"
class KernPart {
private:
    Partition* part;
    BitVector* bitvect;
    HANDLE readsem;
    HANDLE writesem;

    char rootBuff[ClusterSize];

public:
    KernPart(Partition* p);
    ~KernPart() {}
    virtual int readCluster(ClusterNo, char* buffer);
    virtual ClusterNo getNumOfClusters() const;
    virtual int writeCluster(ClusterNo, const char* buffer);

    void freeCluster(ClusterNo);
    ClusterNo getNewEmpty();
    void saveandclose();
};
