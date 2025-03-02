#ifndef STORAGE_LEVELDB_INCLUDE_ENV_H_
#define STORAGE_LEVELDB_INCLUDE_ENV_H_

#include <leveldb/export.h>
#include <cstdarg>

namespace leveldb {
class Logger;
class LEVELDB_EXPORT Logger {
public:
    Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    virtual ~Logger() = default; // 析构函数必须virtual， 当父类指向派生类时可以调用自己的析构函数
    virtual void Logv(const char* format, std::va_list ap) = 0;
};

void Log(Logger* info_log, const char* format, ...) __attribute__((format(printf, 2, 3)));

}  // namespace leveldb

#endif