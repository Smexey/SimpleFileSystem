#include "KernFile.h"

KernFile::KernFile() {}

KernFile::~KernFile() {}

char KernFile::write(BytesCnt b, char* buffer) {}
BytesCnt KernFile::read(BytesCnt b, char* buffer) {}

char KernFile::seek(BytesCnt b) {}
BytesCnt KernFile::filePos() {}

char KernFile::eof() {}
BytesCnt KernFile::getFileSize() {}
char KernFile::truncate() {}
