#ifndef STORAGE_LEVELDB_DB_LOG_WRITER_H_
#define STORAGE_LEVELDB_DB_LOG_WRITER_H_

#include <cstdint>
#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {
class WritableFile; // 声明， 不用引用头文件
namespace log {
class Writer {
public:
    Writer(WritableFile *dest);
    Writer(WritableFile *dest, uint64_t dest_length);
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;
    ~Writer();
    Status AddRecord(const Slice& slice);
private:
    Status EmitPhysicalRecord(RecordType t, const char* ptr, size_t length);
    WritableFile* dest_;
    int block_offset_; // 当前block的偏移量
    uint32_t type_crc_[kMaxRecordType + 1]; // 高性能，查表的方式计算已知的CRC
};

} // namespace log

} // namespace leveldb

#endif // STORAGE_LEVELDB_DB_LOG_WRITER_H_