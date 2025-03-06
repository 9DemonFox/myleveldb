#include "gtest/gtest.h"
#include "leveldb/env.h"
#include <unordered_set>
#include <cstdio>
#include <iostream>
#include <fstream>

namespace leveldb {
TEST(EnvPosixTest, TestWritableFile_DemonFox) {
    WritableFile *wf;
    Env::Default()->NewWritableFile("./TestWritableFile_DemonFox.txt", &wf);
    wf->Append("Hello World");
    wf->Sync();
    std::ifstream infile("./test.txt");
    ASSERT_TRUE(infile.is_open());
    char buffer[32];
    ASSERT_TRUE(infile.getline(buffer, 32));
    ASSERT_STREQ("Hello World", buffer);
    Env::Default()->RemoveFile("./TestWritableFile_DemonFox.txt");
}
}