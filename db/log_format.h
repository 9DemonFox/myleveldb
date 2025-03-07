#ifndef STORAGE_LEVELDB_DB_LOG_FORMAT_H_
#define STORAGE_LEVELDB_DB_LOG_FORMAT_H_

namespace leveldb {

namespace log {
enum RecordType {
    // Zero is reserved for preallocated files
    kZeroType = 0,
    kFullType = 1,
    kFirstType = 2,
    kMiddleType = 3,
    kLastType = 4
};

static const int kMaxRecordType = kLastType;
static const int kBlockSize = 32768; // 32KB
static const int kHeaderSize = 7; // CRC=4 len=2 type=1byte

}
}
#endif // STORAGE_LEVELDB_DB_LOG_FORMAT_H_