#include "KernFile.h"

KernFile::KernFile(KernPart* p, char m) : part(p), mode(m) {
    // HEADER CLASS sa size, start itd
    // FC class-> numofwr numofrd synch ...
    // size = header.size

    switch (mode) {
        case 'a':
            setcurr(size);
            break;

        case 'w':
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
    setcurr(size);
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
        setcurr(curr + 1);
        // posle ili pre setcurr? isto je? prilicno sam siguran da je svj
        currentDataClusOffs = currClus;
    } else {
        // 2 level indexing

        ClusterNo t;
        // ((x-256)/512) offs od pola
        // + 256 offs od starta
        ClusterNo lvl1ind = (curr - singleTableInd) / clusIndNum + singleTableInd;

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
        ClusterNo lvl2ind = (curr - singleTableInd) % clusIndNum;
        t = ((ClusterNo*)helpCache)[lvl2ind];

        // new lvl2 => data also not cached
        if (dirtyData) {
            part->writeCluster(dataCacheClusNum, dataCache);
        }
        dataCacheClusNum = t;
        part->readCluster(dataCacheClusNum, dataCache);
        dirtyData = false;

        *ch = dataCache[currOffs];
        setcurr(curr + 1);
        // posle ili pre setcurr? isto je? prilicno sam siguran da je svj
        currentDataClusOffs = currClus;
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
    // return false na fail readcluster???

    if (curr >= size) return false;
    // check cache
    if (currClus == currentDataClusOffs) {
        *ch = dataCache[currOffs];
        // dirty alrdy set
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

        // los uslov?????
        if ((curr == size) && (curr % ClusterSize == 0)) {
            // alloc
            ClusterNo newClus = part->getNewEmpty();

            // new lvl1 ind
            dataCacheClusNum = newClus;
            ((ClusterNo*)rootCache)[currClus] = dataCacheClusNum;
            dirtyRoot = true;
        }

        part->readCluster(dataCacheClusNum, dataCache);
        // dirtyData = false;
        // *ch = dataCache[currOffs];

        dataCache[currOffs] = *ch;
        dirtyData = true;

        setcurr(curr + 1);
        // posle ili pre setcurr? isto je? prilicno sam siguran da je svj
        currentDataClusOffs = currClus;
    } else {
        // 2 level indexing

        ClusterNo t;
        // ((x-256)/512) offs od pola
        // + 256 offs od starta
        ClusterNo lvl1ind = (curr - singleTableInd) / clusIndNum + singleTableInd;

        t = ((ClusterNo*)rootCache)[lvl1ind];



        ///////////////////////////////ne valja///////////////////////////




        //(curr - singleTableInd) % clusIndNum;  *ClusterSize
        bool f = (size - ClusterSize * singleTableInd) % (ClusterSize * clusIndNum) == 0;

        if (size == curr && f) {
            ClusterNo newClus = part->getNewEmpty();
            // new lvl2 ind
            dataCacheClusNum = newClus;
            ((ClusterNo*)rootCache)[currClus] = t;
            dirtyRoot = true;  // reduntant?
        }

        // redundantno apsolutno, jer svakako su novi podaci
        part->readCluster(helpCacheClusNum, helpCache);

        // // check cache for lvl2 index
        // if (helpCacheClusNum != t) {
        //     // load it into cache
        //     if (dirtyHelp) {
        //         part->writeCluster(helpCacheClusNum, helpCache);
        //     }
        //     helpCacheClusNum = t;
        //     part->readCluster(helpCacheClusNum, helpCache);
        //     dirtyHelp = false;
        // }

        // // ((x-256)%512) offs od starta jednog od lvl2 clus
        // ClusterNo lvl2ind = (curr - singleTableInd) % clusIndNum;
        // t = ((ClusterNo*)helpCache)[lvl2ind];

        // // new lvl2 => data also not cached, would have had a hit prior
        // if (dirtyData) {
        //     part->writeCluster(dataCacheClusNum, dataCache);
        // }
        // dataCacheClusNum = t;
        // part->readCluster(dataCacheClusNum, dataCache);
        // dirtyData = false;

        // *ch = dataCache[currOffs];
        // setcurr(curr + 1);
        // // posle ili pre setcurr? isto je? prilicno sam siguran da je svj
        // currentDataClusOffs = currClus;
    }
    return true;
}

char KernFile::write(BytesCnt b, char* buffer) {}

char KernFile::truncate() {}
