// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
#include <fcntl.h>
#include <string>
#include <leveldb/env.h>
#include <util/posix_logger.h>
#include "gtest/gtest.h"

namespace leveldb {
TEST(PosixLogger, WriteTest_DemonFox) {
    {
        int fd = ::open("test.log", O_APPEND | O_WRONLY | O_CREAT, 0644);
        std::FILE* fp = fdopen(fd, "w");
        Logger* plogger = new PosixLogger(fp);
        Log(plogger, "Hello, %s", "world!");
    }

    // fd 析构时关闭了
    char buf[100];
    fgets(buf, 100, fopen("test.log", "r"));
    std::string s(buf);
    EXPECT_TRUE(s.find("Hello, world!") != std::string::npos);
}

}