#pragma once
#include "file.h"

class KernFile {
public:
    KernFile();

    ~KernFile();

    char write(BytesCnt, char* buffer);
    BytesCnt read(BytesCnt, char* buffer);
    char seek(BytesCnt);
    BytesCnt filePos();
    char eof();
    BytesCnt getFileSize();
    char truncate();
};