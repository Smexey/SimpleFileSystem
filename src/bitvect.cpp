
#include "bitvect.h"
#include <iostream>
#include "kernpart.h"

BitVector::BitVector(ClusterNo sz, KernPart* p) : part(p), size(sz) {
    // bitvect pocinje od nule, zauzima vise od jednog clustera???
    mutex = CreateSemaphore(0, 1, 1, 0);
    // jedan vise od size
    clusterNum = size / BitClusterSize + (ClusterNo)(size % BitClusterSize > 0);
    cachefree = new list<ClusterNo>();

    bitVect = new char*[clusterNum];
    for (size_t i = 0; i < clusterNum; i++) {
        bitVect[i] = new char[ClusterSize];
        part->readCluster(i, (char*)bitVect[i]);
    }

    // prvih clno je bitvector clusteri
    freeCl = clusterNum + 1;

    // trazi prvi slobodan
    findFree();
}

void BitVector::findFree() {
    // already locked!!!

    while (!(bitVect[freeCl / BitClusterSize][(freeCl % BitClusterSize) / 8] &
             ((1 << (7 - freeCl % 8))))) {
        freeCl = (freeCl + 1);
    }
    if (freeCl >= size) cout << "nema mesta" << endl;

    // inf while ako ne nadje lolololol
}

ClusterNo BitVector::getFirstEmpty() {
    CriticalSection lck(mutex);

    while (!cachefree->empty()) {
        ClusterNo tmp = cachefree->front();
        cachefree->pop_front();

        // if error?
        if ((bitVect[tmp / BitClusterSize][(tmp % BitClusterSize) / 8] & ((1 << (7 - tmp % 8))))) {
            return tmp;
        }
    }

    findFree();

    ClusterNo ret = freeCl;
    setNotFree(freeCl);

    // treba li mi?
    findFree();

    return ret;
}

void BitVector::free(ClusterNo cl) {
    CriticalSection lck(mutex);
    // cache free
    cachefree->push_back(cl);
    setFree(cl);
}

void BitVector::format() {
    CriticalSection lck(mutex);

    for (size_t i = 0; i < clusterNum; i++)
        for (int j = 0; j < ClusterSize; j++) bitVect[i][j] = -1;

    // prvih clno je zauzeto za bitvect
    size_t i = 0;
    for (i = 0; i < clusterNum; i += 8) {
        bitVect[0][i / 8] = ~0;
    }
    for (; i <= clusterNum; i++) bitVect[0][i / 8] ^= (1 << (7 - i % 8));

    // krece od posle bitvect
    freeCl = clusterNum + 1;
    cachefree->clear();
}

void BitVector::writeToDisk() {
    for (size_t i = 0; i < clusterNum; i++) part->writeCluster(i, bitVect[i]);
}

ClusterNo BitVector::getRootPosition() { return clusterNum; }

BitVector::~BitVector() {
    writeToDisk();

    for (size_t i = 0; i < clusterNum; i++) delete bitVect[i];
    delete cachefree;
    delete bitVect;
}
