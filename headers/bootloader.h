#pragma once
#include <cstdint>
#include <cstdio>

/*
 * STM32 ROM Bootloader programmer (BIN) via FTDI UART
 * --------------------------------------------------
 * • BIN вместо HEX
 * • WRITE MEMORY по 256 байт
 * • Скорость как STM32CubeProgrammer
 *
 * Протокол: AN3155
 * FTDI D2XX (Windows / Linux)
 */

#include <cstdint>
#include <cstring>
#include <ftd2xx.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#define GET_TICK_COUNT() GetTickCount()
#else
#include <unistd.h>
#include <sys/time.h>
#define sleep_ms(ms) usleep((ms) * 1000)

static uint32_t GET_TICK_COUNT()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
#endif

/* ================= CONFIG ================= */

#define ACK                 0x79

#define CMD_WRITE_MEMORY    0x31
#define CMD_ERASE           0x43
#define CMD_EXT_ERASE       0x44
#define CMD_GO              0x21

bool uart_write(FT_HANDLE ft, const uint8_t* data, int len);
bool uart_read(FT_HANDLE ft, uint8_t* b);
bool wait_ack(FT_HANDLE ft, int timeout_ms);
bool send_cmd(FT_HANDLE ft, uint8_t cmd);

bool bl_mass_erase(FT_HANDLE ft);
bool bl_write(FT_HANDLE ft, uint32_t addr, uint8_t* data, int len);
bool bl_go(FT_HANDLE ft, uint32_t addr);
