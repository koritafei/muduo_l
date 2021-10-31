#ifndef __ZLIBSTREAM_H__
#define __ZLIBSTREAM_H__

#include <zconf.h>

#include "../base/Noncopyable.h"
#include "Buffer.h"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <zlib.h>

namespace muduo {

namespace net {

class ZlibInputStream : Noncopyable {
public:
  explicit ZlibInputStream(Buffer* output) : output_(output), zerror_(Z_OK) {
    memZero(&zstream_, sizeof zstream_);
    zerror_ = inflateInit(&zstream_);
  }

  ~ZlibInputStream() {
    finish();
  }

  bool write(StringPiece buf);
  bool write(Buffer* input);
  bool finish();

private:
  int decompress();

  Buffer*  output_;
  z_stream zstream_;
  int      zerror_;
};  // class ZlibInputStream

class ZlibOutputStream : Noncopyable {
public:
  explicit ZlibOutputStream(Buffer* output)
      : output_(output), zerror_(Z_OK), bufferSize_(1024) {
    memZero(&zstream_, sizeof zstream_);
    zerror_ = deflateInit(&zstream_, Z_DEFAULT_COMPRESSION);
  }

  ~ZlibOutputStream() {
    finish();
  }

  const char* zlibErrorMessage() const {
    return zstream_.msg;
  }

  int zlibErrorCode()const{
    return zerror_;
  }

  int64_t inputBytes() const {
    return zstream_.total_in;
  }

  int64_t outputBytes() const {
    return zstream_.total_out;
  }

  int internalOutputBufferSize() const {
    return bufferSize_;
  }

  bool write(StringPiece buf) {
    if (zerror_ != Z_OK) {
      return false;
    }

    assert(nullptr == zstream_.next_in && 0 == zstream_.avail_in);
    void* in          = const_cast<char*>(buf.data());
    zstream_.next_in  = static_cast<Bytef*>(in);
    zstream_.avail_in = buf.size();

    while (0 < zstream_.avail_in && Z_OK == zerror_) {
      zerror_ = compress(Z_NO_FLUSH);
    }

    if (0 == zstream_.avail_in) {
      assert(static_cast<const void*>(zstream_.next_in) == buf.end());
      zstream_.next_in = nullptr;
    }

    return zerror_ == Z_OK;
  }

  bool write(Buffer* buf) {
    if (Z_OK == zerror_) {
      return false;
    }

    void* in          = const_cast<char*>(buf->peek());
    zstream_.next_in  = static_cast<Bytef*>(in);
    zstream_.avail_in = static_cast<int>(buf->readableBytes());

    if (0 < zstream_.avail_in && zerror_ == Z_OK) {
      zerror_ = compress(Z_NO_FLUSH);
    }

    buf->retrieve(buf->readableBytes() - zstream_.avail_in);

    return zerror_ == Z_OK;
  }

  bool finish() {
    if (Z_OK != zerror_) {
      return false;
    }

    while (Z_OK == zerror_) {
      zerror_ = compress(Z_FINISH);
    }

    zerror_ = deflateEnd(&zstream_);
    bool ok = zerror_ == Z_OK;
    zerror_ = Z_STREAM_END;
    return ok;
  }

private:
  int compress(int flush) {
    output_->ensureWritableBytes(bufferSize_);
    zstream_.next_out  = reinterpret_cast<Bytef*>(output_->beginWrite());
    zstream_.avail_out = static_cast<int>(output_->writableBytes());
    int error          = ::deflate(&zstream_, flush);

    output_->hasWritten(output_->writableBytes() - zstream_.avail_out);

    if (0 == output_->writableBytes() && bufferSize_ < 65536) {
      bufferSize_ *= 2;
    }

    return error;
  }

  Buffer*  output_;
  z_stream zstream_;
  int      zerror_;
  int      bufferSize_;
};  // class ZlibOutputStream

}  // namespace net

}  // namespace muduo

#endif /* __ZLIBSTREAM_H__ */
