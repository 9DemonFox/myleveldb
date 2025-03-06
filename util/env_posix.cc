#include <leveldb/env.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <atomic>


namespace leveldb {

// Common flags defined for all posix open operations
#if defined(HAVE_O_CLOEXEC)
constexpr const int kOpenBaseFlags = O_CLOEXEC;
#else
constexpr const int kOpenBaseFlags = 0;
#endif  // defined(HAVE_O_CLOEXEC)

constexpr const size_t kWritableFileBufferSize = 65536; // 64KiB

Status PosixError(const std::string& context, int error_number) {
    if (error_number == ENOENT) {
      return Status::NotFound(context, std::strerror(error_number));
    } else {
      return Status::IOError(context, std::strerror(error_number));
    }
  }

class PosixWritableFile final :  public WritableFile {
public:
    PosixWritableFile(std::string filename, int fd)
        : pos_(0),
          fd_(fd),
          is_manifest_(IsManifest(filename)),
          filename_(std::move(filename)),
          dirname_(Dirname(filename_)) {}

    Status Append(const Slice& data) override {
        size_t write_size = data.size();
        const char* write_data = data.data();

        size_t copy_size = std::min(write_size, kWritableFileBufferSize - pos_);
        std::memcpy(buf_ + pos_, write_data, copy_size);
        write_data += copy_size;
        write_size -= copy_size;
        pos_ += copy_size;
        if (write_size == 0) {
            return Status::OK();
        }

        Status status = FlushBuffer();
        if(!status.ok()) {
            return status;
        }

        if(write_size < kWritableFileBufferSize) {
            std::memcpy(buf_, write_data, write_size);
            pos_ = write_size;
            return Status::OK();
        }
        return WriteUnbuffered(write_data, write_size);
    }

    Status WriteUnbuffered(const char* data, size_t size) {
        while (size > 0) {
          ssize_t write_result = ::write(fd_, data, size);
          if (write_result < 0) {
            if (errno == EINTR) {
              continue;  // Retry
            }
            return PosixError(filename_, errno);
          }
          data += write_result;
          size -= write_result;
        }
        return Status::OK();
      }

    Status Close() override {
      Status status = FlushBuffer();
      const int close_result = ::close(fd_);
      if (close_result < 0 && status.ok()) {
        status = PosixError(filename_, errno);
      }
      fd_ = -1;
      return status;
    }
  
    Status Flush() override { return FlushBuffer(); }
  
    Status Sync() override {
      // Ensure new files referred to by the manifest are in the filesystem.
      //
      // This needs to happen before the manifest file is flushed to disk, to
      // avoid crashing in a state where the manifest refers to files that are not
      // yet on disk.
      Status status = SyncDirIfManifest();
      if (!status.ok()) {
        return status;
      }
  
      status = FlushBuffer();
      if (!status.ok()) {
        return status;
      }
  
      return SyncFd(fd_, filename_);
    }

private:

    Status FlushBuffer() {
      Status status = WriteUnbuffered(buf_, pos_);
      pos_ = 0;
      return status;
    }

    Status SyncDirIfManifest() {
        Status status;
        if (!is_manifest_) {
            return status;
        }

        int fd = ::open(dirname_.c_str(), O_RDONLY | kOpenBaseFlags);
        if (fd < 0) {
            status = PosixError(dirname_, errno);
        } else {
        status = SyncFd(fd, dirname_);
            ::close(fd);
        }
        return status;
    }

    static Status SyncFd(int fd, const std::string& fd_path) {    
    #if HAVE_FDATASYNC
        bool sync_success = ::fdatasync(fd) == 0;
    #else
        bool sync_success = ::fsync(fd) == 0;
    #endif  // HAVE_FDATASYNC
    
        if (sync_success) {
          return Status::OK();
        }
        return PosixError(fd_path, errno);
      }
    
    static std::string Dirname(const std::string& filename) {
        std::string::size_type separator_pos = filename.rfind('/');
        if (separator_pos == std::string::npos) {
        return std::string(".");
        }
        // The filename component should not contain a path separator. If it does,
        // the splitting was done incorrectly.
        assert(filename.find('/', separator_pos + 1) == std::string::npos);

        return filename.substr(0, separator_pos);
    }

    static Slice Basename(const std::string& filename) {
        std::string::size_type separator_pos = filename.rfind('/');
        if (separator_pos == std::string::npos) {
          return Slice(filename);
        }
        // The filename component should not contain a path separator. If it does,
        // the splitting was done incorrectly.
        assert(filename.find('/', separator_pos + 1) == std::string::npos);
    
        return Slice(filename.data() + separator_pos + 1,
                     filename.length() - separator_pos - 1);
    }

    static bool IsManifest(const std::string& filename) {
        return Basename(filename).starts_with("MANIFEST");
    }

private:
    // buf_[0, pos_ - 1] 表示等待flush的数据
    char buf_[kWritableFileBufferSize];
    size_t pos_;
    int fd_;

    const bool is_manifest_;
    std::string filename_;
    std::string dirname_;
};


class PosixEnv : public Env {
public:
PosixEnv() {}
~PosixEnv() override {
  static const char msg[] = "PosixEnv singleton destroyed. Unsupported behavior!\n";
  std::fwrite(msg, 1, sizeof(msg), stderr);
  std::abort();
}
  
Status NewWritableFile(const std::string& filename, WritableFile** result) override {
  int fd = ::open(filename.c_str(),
  O_TRUNC | O_WRONLY | O_CREAT | kOpenBaseFlags, 0644);
  if (fd < 0) {
  *result = nullptr;
  return PosixError(filename, errno);
  }

  *result = new PosixWritableFile(filename, fd);
  return Status::OK();
}

Status RemoveFile(const std::string& f) override {
  if(::unlink(f.c_str()) != 0) {
    return PosixError(f, errno);
  }
  return Status::OK();
}

};



template <typename EnvType>
class SingletonEnv {
public:
    SingletonEnv() {
      static_assert(sizeof(env_storage_) >= sizeof(EnvType), "env_storage_ will not fit the Env");
      static_assert(std::is_standard_layout<SingletonEnv<EnvType>>::value); // C++11 std::is_standard_layout_v
      static_assert(offsetof(SingletonEnv<EnvType>, env_storage_) % alignof(EnvType) == 0, 
                    "env_storage_ does not meet the Env's alignment needs");
      static_assert(alignof(SingletonEnv<EnvType>) % alignof(EnvType) == 0,
                    "env_storage_ does not meet the Env's alignment needs");
      new (env_storage_) EnvType();
    }
    ~SingletonEnv() = default;

    SingletonEnv(const SingletonEnv&) = delete;
    SingletonEnv& operator=(const SingletonEnv&) = delete;
    Env* env() { return reinterpret_cast<Env*>(&env_storage_); }
    static void AssertEnvNotInitialized() {
    }

private:
  alignas(EnvType) char env_storage_[sizeof(EnvType)];

};

using PosixDefaultEnv = SingletonEnv<PosixEnv>;


Env* Env::Default() {
  static PosixDefaultEnv env_container; // c++11 静态变量线程安全
  return env_container.env();
}

}