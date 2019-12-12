#pragma once
#include "Part.h"

class KernPart {
private:
    Partition* part;

public:
    KernPart(Partition* p) : part(p) {}
    ~KernPart() {}
    virtual int readCluster(ClusterNo, char* buffer);
    virtual ClusterNo getNumOfClusters() const;
    virtual int writeCluster(ClusterNo, const char* buffer);

    
    ClusterNo getNewEmpty();
    void saveandclose();
};
