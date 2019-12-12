#include "KernFile.h"

KernFile::KernFile(KernPart* p, char m) : part(p), mode(m) {
    // HEADER CLASS sa size, start itd
    // size = header.size

    switch (mode) {
        case 'a':
            curr = size;
            currClus = curr / ClusterSize;
            currOffs = curr % ClusterSize;
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

    curr = size;
    currClus = curr / ClusterSize;
    currOffs = curr % ClusterSize;
    return 1;
}

BytesCnt KernFile::getFileSize() { return size; }

BytesCnt KernFile::read(BytesCnt b, char* buffer) {}

char KernFile::write(BytesCnt b, char* buffer) {}



char KernFile::truncate() {}
