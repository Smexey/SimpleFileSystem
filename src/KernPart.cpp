#include "KernPart.h"

KernPart::KernPart(Partition* p) : part(p) {
    bitvect = new BitVector(p->getNumOfClusters(), this);
    
    part->readCluster(bitvect->getRootPosition(),rootBuff)

    

    // openFiles = new Hashmap
}

int KernPart::readCluster(ClusterNo n, char* buffer) { part->readCluster(n, buffer); }

ClusterNo KernPart::getNumOfClusters() const { return part->getNumOfClusters(); }

int KernPart::writeCluster(ClusterNo n, const char* buffer) { part->writeCluster(n, buffer); }

void KernPart::saveandclose() {}

ClusterNo KernPart::getNewEmpty() { return bitvect->getFirstEmpty(); }

void KernPart::freeCluster(ClusterNo) {}
