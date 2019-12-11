#pragma once
#include "Part.h"

class KernPart {
private:
    Partition* part;

public:
    KernPart(Partition* p) : part(p) {}
    ~KernPart() {}

    void saveandclose();
    virtual int readCluster(ClusterNo, char *buffer);
};
