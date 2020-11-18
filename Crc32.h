// //////////////////////////////////////////////////////////
// Crc32.h
// uint8_t, uint32_t, int32_t
#include <cstdint>
#include <cstddef>

/// compute CRC32 (bitwise algorithm)
uint32_t crc32_bitwise (const void* data, size_t length, uint32_t previousCrc32 = 0);
