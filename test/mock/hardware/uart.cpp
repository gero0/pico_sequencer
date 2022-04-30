#include "uart.h"
#include "CppUTestExt/MockSupport.h"

void uart_write_blocking(int, const uint8_t *src, size_t len){
    mock().actualCall("uart_write_blocking");
}