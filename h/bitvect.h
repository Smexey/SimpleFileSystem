#pragma once
#include <stdio.h>
#include <list>
#include "part.h"
#include "synch.h"

const unsigned long BitClusterSize = 2048 * 8;

using namespace std;

class KernPart;
class KernFile;

class BitVector {

    typedef unsigned char byte;
    CRITICAL_SECTION cs;
public:
    BitVector(ClusterNo size, KernPart* p);
    ClusterNo getFirstEmpty();
    void free(ClusterNo);
    void format();
    void writeToDisk();
    ~BitVector();

    ClusterNo getRootPosition();

private:
    void findFree();

    void setFree(ClusterNo cl) {
        bitVect[cl / BitClusterSize][(cl % BitClusterSize) / 8] |=
            (1 << (7 - cl % 8));
    }

    void setNotFree(ClusterNo cl) {
        bitVect[cl / BitClusterSize][(cl % BitClusterSize) / 8] ^=
            (1 << (7 - cl % 8));
    }
    ClusterNo size;
    ClusterNo clusterNum;

    char** bitVect;

    KernPart* part;

    //dequeue?
    list<ClusterNo>* cachefree;
    ClusterNo freeCl;

    // friend KernelFile;
    // friend PartitionKernel;
};