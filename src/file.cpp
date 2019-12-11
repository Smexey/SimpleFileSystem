#include "file.h"
#include "KernFile.h"

File::File() { myImpl = new KernFile(); }

File::~File() { delete myImpl; }

char File::write(BytesCnt b, char* buffer) {
    if (myImpl) return myImpl->write(b, buffer);
}
BytesCnt File::read(BytesCnt b, char* buffer) {
    if (myImpl) myImpl->read(b, buffer);
}

char File::seek(BytesCnt b) {
    if (myImpl) return myImpl->seek(b);
}
BytesCnt File::filePos() {
    if (myImpl) return myImpl->filePos();
}

char File::eof() {
    if (myImpl) return myImpl->eof();
}
BytesCnt File::getFileSize() {
    if (myImpl) return myImpl->getFileSize();
}
char File::truncate() {
    if (myImpl) return myImpl->truncate();
}
