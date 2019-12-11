

#include "KernPart.h"

void KernPart::saveandclose() {}

int KernPart::readCluster(ClusterNo n, char* buffer) {
    part->readCluster(n, buffer);
}

ClusterNo KernPart::getNumOfClusters() const {
    return part->getNumOfClusters();
}

int KernPart::writeCluster(ClusterNo n, const char* buffer) {
    part->writeCluster(n, buffer);
}
