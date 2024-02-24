#pragma once

// stdint throws some minor macro redefinition warnings w/ WDK, which we need
// to bypass.
#pragma warning(push, 0)
#pragma warning(disable: 4083)
#pragma warning(disable: 4005)
#include <stdint.h>
#pragma warning(pop)


uint32_t crc32buf(uint8_t* buf, uint32_t len);

uint32_t crc32str(const char* str);

