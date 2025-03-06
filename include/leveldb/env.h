#ifndef STORAGE_LEVELDB_INCLUDE_ENV_H_
#define STORAGE_LEVELDB_INCLUDE_ENV_H_

#include <leveldb/slice.h>
#include <leveldb/export.h>
#include <leveldb/status.h>
#include <cstdarg>

namespace leveldb {
class Logger;
class WritableFile;
class LEVELDB_EXPORT Env {
public:
    Env() = default;

    Env(const Env&) = delete;

    Env& operator=(const Env&) = delete;
   
    virtual ~Env() = default;

    static Env* Default(); // level db 使用的，一直存在不被释放

    virtual Status NewWritableFile(const std::string& fname, WritableFile** result) = 0;
    
    virtual Status RemoveFile(const std::string& fname) = 0; // 为什么原版没有
   };

class LEVELDB_EXPORT WritableFile 
{
public:
    WritableFile() = default;
    virtual ~WritableFile() = default;
    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;

    virtual Status Append(const Slice& data) = 0;
    virtual Status Close() = 0;
    virtual Status Flush() = 0;
    virtual Status Sync() = 0;
};

// 设想需要统计创建文件的数量，又不想和功能代码揉在一起，所以有个Wrapper类型
class LEVELDB_EXPORT EnvWrapper : public Env {
public:
    explicit EnvWrapper(Env* t) : target_(t) {}
    virtual ~EnvWrapper() = default;
    Env* target() const { return target_; }

    Status NewWritableFile(const std::string& f, WritableFile** r) override {
        return target_->NewWritableFile(f, r);
    }

    Status RemoveFile(const std::string& f) override {
        return target_->RemoveFile(f);
    }

private:
    Env* target_;
};

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