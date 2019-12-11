
#include "bitvect.h"
#include <iostream>
#include "kernpart.h"

BitVector::BitVector(ClusterNo sz, KernPart* p) : part(p), size(sz) {
    // bitvect pocinje od nule, zauzima vise od jednog clustera???

    // jedan vise od size
    clNo = size / BitClusterSize + (ClusterNo)(size % BitClusterSize > 0);
    cachefree = new list<ClusterNo>();

    bitVect = new char*[clNo];
    for (size_t i = 0; i < clNo; i++) {
        bitVect[i] = new char[ClusterSize];
        part->readCluster(i, (char*)bitVect[i]);
    }

    // prvih clno je bitvector clusteri
    freeCl = clNo + 1;

    // trazi prvi slobodan
    findfree();
}

void BitVector::findfree() {
    // finds next free from current freeCL
    // greska ako ne nadje nista?
    while (!bitVect[freeCl / BitClusterSize][(freeCl % BitClusterSize) / 8] &
           (1 << (7 - freeCl % 8)))
        freeCl = (freeCl + 1) % size;

    // inf while ako ne nadje lolololol
}

ClusterNo BitVector::getFirstEmpty() {
    while (!cachefree->empty()) {
        ClusterNo tmp = cachefree->front();
        cachefree->pop_front();

        if (!bitVect[tmp / BitClusterSize][(tmp % BitClusterSize) / 8] &
            ((1 << (7 - tmp % 8)))) {
            cout << "BitVector::getFirstEmpty() - "
                    "?????????"
                 << endl;
            continue;
        }

        return tmp;
    }

    findfree();

    ClusterNo ret = freeCl;
    setnotfree(freeCl);

    // treba li mi?
    findfree();

    return ret;
}

void BitVector::free(ClusterNo cl) {
    // cash free
    cachefree->push_back(cl);
    setfree(cl);
}

void BitVector::format() {
    for (size_t i = 0; i < clNo; i++)
        for (int j = 0; j < ClusterSize; j++) bitVect[i][j] = -1;

    // prvih clno je zauzeto za bitvect
    size_t i = 0;
    for (i = 0; i < clNo; i += 8) {
        bitVect[0][i / 8] = ~0;
    }
    for (; i <= clNo; i++) bitVect[0][i / 8] ^= (1 << (7 - i % 8));

    // krece od posle bitvect
    freeCl = clNo + 1;
    cachefree->clear();
}

void BitVector::writeToDisk() {
    for (int i = 0; i < clNo; i++) part->writeCluster(i, bitVect[i]);
}

ClusterNo BitVector::getRootPosition() { return clNo; }

BitVector::~BitVector() {
    writeToDisk();

    for (size_t i = 0; i < clNo; i++) delete bitVect[i];
    delete cachefree;
    delete bitVect;
}
