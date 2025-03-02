#include <leveldb/env.h>

namespace leveldb {
void Log(Logger* info_log, const char* format, ...) {
    if (info_log != nullptr) {
        std::va_list ap;
        va_start(ap, format); // format 是最后一个固定参数的位置
        info_log->Logv(format, ap);
        va_end(ap);
    }
}

}

