#include "KernPart.h"

int KernPart::readCluster(ClusterNo n, char* buffer) { part->readCluster(n, buffer); }

ClusterNo KernPart::getNumOfClusters() const { return part->getNumOfClusters(); }

int KernPart::writeCluster(ClusterNo n, const char* buffer) { part->writeCluster(n, buffer); }

void KernPart::saveandclose() {}
ClusterNo KernPart::getNewEmpty() {}
void KernPart::freeCluster(ClusterNo) {}
