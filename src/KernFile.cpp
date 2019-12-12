#include "KernFile.h"

KernFile::KernFile(KernPart* p, char m) : part(p), mode(m) {
    

}

KernFile::~KernFile() {}

BytesCnt KernFile::read(BytesCnt b, char* buffer) {}
char KernFile::write(BytesCnt b, char* buffer) {}

char KernFile::seek(BytesCnt b) {}
BytesCnt KernFile::filePos() {}

char KernFile::eof() {}
BytesCnt KernFile::getFileSize() {}
char KernFile::truncate() {}
