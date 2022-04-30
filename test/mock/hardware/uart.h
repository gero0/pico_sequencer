#include <cstdint>
#include <cstdio>

#define uart0 0

void uart_write_blocking(int, const uint8_t *src, size_t len);