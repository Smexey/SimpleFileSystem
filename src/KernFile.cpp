#include "KernFile.h"

KernFile::KernFile(KernPart* p, char m) : part(p), mode(m) {
    // HEADER CLASS sa size, start itd
    // FC class-> numofwr numofrd synch ...
    // size = header.size

    // load root???????????
    switch (mode) {
        case 'a':
            setcurr(size);
            // treba ucitati sva tri cachea
            break;

        case 'w':
            setcurr(0);
            truncate();
            break;

        default:
            break;
    }
}

KernFile::~KernFile() {}

BytesCnt KernFile::filePos() { return curr; }

char KernFile::eof() {
    if (curr > size) return 1;
    if (curr == size) return 2;
    return 0;
}

char KernFile::seek(BytesCnt b) {
    if (b > size) return 0;
    setcurr(b);
    return 1;
}

BytesCnt KernFile::getFileSize() { return size; }

bool KernFile::readByte(char* ch) {
    // return false na fail readcluster???

    if (curr >= size) return false;
    // check cache
    if (currClus == currentDataClusOffs) {
        *ch = dataCache[currOffs];
        setcurr(curr + 1);
        return true;
    }

    // load into cache
    if (currClus < singleTableInd) {
        // 1 level indexing
        ClusterNo t = ((ClusterNo*)rootCache)[currClus];
        if (dirtyData) {
            part->writeCluster(dataCacheClusNum, dataCache);
        }
        dataCacheClusNum = t;
        part->readCluster(dataCacheClusNum, dataCache);
        dirtyData = false;

        *ch = dataCache[currOffs];
        currentDataClusOffs = currClus;
        setcurr(curr + 1);

    } else {
        // 2 level indexing

        ClusterNo t;
        // ((x-256)/512) offs od pola
        // + 256 offs od starta
        ClusterNo lvl1ind = (currClus - singleTableInd) / clusIndNum + singleTableInd;

        t = ((ClusterNo*)rootCache)[lvl1ind];

        // check cache for lvl2 index
        if (helpCacheClusNum != t) {
            // load it into cache
            if (dirtyHelp) {
                part->writeCluster(helpCacheClusNum, helpCache);
            }
            helpCacheClusNum = t;
            part->readCluster(helpCacheClusNum, helpCache);
            dirtyHelp = false;
        }

        // ((x-256)%512) offs od starta jednog od lvl2 clus
        ClusterNo lvl2ind = (currClus - singleTableInd) % clusIndNum;
        t = ((ClusterNo*)helpCache)[lvl2ind];

        // new lvl2 => data also not cached
        if (dirtyData) {
            part->writeCluster(dataCacheClusNum, dataCache);
        }
        dataCacheClusNum = t;
        part->readCluster(dataCacheClusNum, dataCache);
        dirtyData = false;

        *ch = dataCache[currOffs];

        currentDataClusOffs = currClus;
        setcurr(curr + 1);
    }
    return true;
}

BytesCnt KernFile::read(BytesCnt b, char* buffer) {
    BytesCnt i;
    for (i = 0; i < b && readByte(buffer + i); i++)
        ;
    return i;
}

bool KernFile::writeByte(char* ch) {
    // moze da se upise gde oces

    // return false na fail readcluster???

    // check cache
    if (currClus == currentDataClusOffs) {
        *ch = dataCache[currOffs];
        dirtyData = true;

        if (curr == size) size++;  // ako upisuje na kraj
        setcurr(curr + 1);

        return true;
    }

    // load into cache
    if (currClus < singleTableInd) {
        // need new data cache
        if (dirtyData) {
            part->writeCluster(dataCacheClusNum, dataCache);
            dirtyData = false;
        }

        // kraj datacache
        if (curr == size && size % ClusterSize == 0) {
            // new lvl1 ind
            dataCacheClusNum = part->getNewEmpty();
            ((ClusterNo*)rootCache)[currClus] = dataCacheClusNum;
            dirtyRoot = true;
            size++;  // jer se upisuje na kraj svakako
        } else {
            dataCacheClusNum = ((ClusterNo*)rootCache)[currClus];
            // ne upisuje na kraj znaci ne menja se size
        }

        part->readCluster(dataCacheClusNum, dataCache);
        currentDataClusOffs = currClus;

        dataCache[currOffs] = *ch;
        dirtyData = true;

        setcurr(curr + 1);
    } else {
        // 2 level indexing

        ClusterNo t;
        // ((x-256)/512) offs od pola
        // + 256 offs od starta
        ClusterNo lvl1ind = (curr - singleTableInd) / clusIndNum + singleTableInd;

        t = ((ClusterNo*)rootCache)[lvl1ind];

        //(curr - singleTableInd) % clusIndNum;  *ClusterSize
        bool endofhelpcache =
            size == curr && (size - ClusterSize * singleTableInd) % (ClusterSize * clusIndNum) == 0;

        // size je uvek curr?
        // los uslov za append???
        if (endofhelpcache) {
            // if (dirtyHelp) {
            //     part->writeCluster(helpCacheClusNum, helpCache);
            //     dirtyHelp = false;
            // }
            // new lvl2 ind

            // namesti t da bude los za sledeci if
            t = part->getNewEmpty();
            ((ClusterNo*)rootCache)[lvl1ind] = t;
            dirtyRoot = true;

            // // redundantno? mozda za append?
            // part->readCluster(helpCacheClusNum, helpCache);
            // dirtyHelp = false;
        }
        if (helpCacheClusNum != t) {
            if (dirtyHelp) {
                part->writeCluster(helpCacheClusNum, helpCache);
            }
            helpCacheClusNum = t;
            part->readCluster(helpCacheClusNum, helpCache);
            dirtyHelp = false;
        }

        ClusterNo lvl2ind = (currClus - singleTableInd) % clusIndNum;
        t = ((ClusterNo*)helpCache)[lvl2ind];

        if (curr == size && size % ClusterSize == 0) {
            // novi stavi u t
            t = part->getNewEmpty();
            ((ClusterNo*)helpCache)[lvl2ind] = t;
            dirtyHelp = true;
            size++;  // jer se upisuje na kraj svakako
        }

        if (dataCacheClusNum != t) {
            if (dirtyData) {
                part->writeCluster(dataCacheClusNum, dataCache);
            }
            dataCacheClusNum = t;
            part->readCluster(dataCacheClusNum, dataCache);
            dirtyData = false;
        }

        part->readCluster(dataCacheClusNum, dataCache);
        // dataCacheClusNum = currClus;
        dataCache[currOffs] = *ch;
        dirtyData = true;
        currentDataClusOffs = currClus;

        setcurr(curr + 1);
    }
    return true;
}

char KernFile::write(BytesCnt b, char* buffer) {
    BytesCnt i;
    for (i = 0; i < b && writeByte(buffer + i); i++)
        ;
    return (i == b);
}

char KernFile::truncate() {
    ClusterNo currClusDel = currClus + (int)currOffs > 0;
    ClusterNo sizeInClus = size / ClusterSize + size % ClusterSize ? 1 : 0;

    // delete lvl1
    for (; currClusDel < sizeInClus && currClusDel < singleTableInd; currClusDel++)
        part->freeCluster(((ClusterNo*)rootCache)[currClusDel]);

    // delete lvl2
    char delCache[ClusterSize];
    // in case start is >256, has to load first
    if (currClus > singleTableInd && !((currClusDel - singleTableInd) % 512 == 0)) {
        part->readCluster(
            ((ClusterNo*)rootCache)[(currClusDel - singleTableInd) / clusIndNum + singleTableInd],
            delCache);
    }

    for (; currClusDel < sizeInClus; currClusDel++) {
        if ((currClusDel - singleTableInd) % 512 == 0) {
            // load del(help) cache
            ClusterNo rootDelNumlvl2 = (currClusDel - singleTableInd) / clusIndNum + singleTableInd;
            part->readCluster(((ClusterNo*)rootCache)[rootDelNumlvl2], delCache);
        }
        part->freeCluster(((ClusterNo*)delCache)[(currClusDel - singleTableInd) % clusIndNum]);
    }

    return 1;
}
