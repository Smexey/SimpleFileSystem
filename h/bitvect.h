#pragma once
#include <stdio.h>
#include <list>
#include "part.h"

const unsigned long BitClusterSize = 2048 * 8;

using namespace std;

class KernPart;
class KernFile;

class BitVector {
    typedef unsigned char byte;

public:
    BitVector(ClusterNo size, KernPart* p);
    ClusterNo getFirstEmpty();
    void free(ClusterNo clNo);
    void format();
    void writeToDisk();
    ~BitVector();

    ClusterNo getRootPosition();

private:
    void findfree();

    void setfree(ClusterNo cl) {
        bitVect[cl / BitClusterSize][(cl % BitClusterSize) / 8] |=
            (1 << (7 - cl % 8));
    }

    void setnotfree(ClusterNo cl) {
        bitVect[cl / BitClusterSize][(cl % BitClusterSize) / 8] ^=
            (1 << (7 - cl % 8));
    }
    ClusterNo size;
    ClusterNo clNo;

    char** bitVect;

    KernPart* part;

    list<ClusterNo>* cachefree;
    ClusterNo freeCl;

    // friend KernelFile;
    // friend PartitionKernel;
};