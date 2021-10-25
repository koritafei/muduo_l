#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <endian.h>
#include <stdint.h>

#include <cstdint>

namespace muduo {

namespace net {

namespace sockets {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t hostToNetwork64(uint64_t host64) {
  return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
  return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
  return htobe16(host16);
}

inline uint64_t networkTohost64(uint64_t net64) {
  return be64toh(net64);
}

inline uint32_t networkTohost32(uint32_t net32) {
  return be32toh(net32);
}

inline uint16_t networkTohost16(uint16_t net16) {
  return be16toh(net16);
}

#pragma GCC diagnostic pop

}  // namespace sockets

}  // namespace net

}  // namespace muduo

#endif /* __ENDIAN_H__ */
