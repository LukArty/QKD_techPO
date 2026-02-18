#include <bootloader.h>

/* ================= UART ================= */

bool uart_write(FT_HANDLE ft, const uint8_t* data, int len)
{
    DWORD written = 0;
    return (FT_Write(ft, (LPVOID)data, len, &written) == FT_OK) &&
           (written == (DWORD)len);
}

bool uart_read(FT_HANDLE ft, uint8_t* b)
{
    DWORD read = 0;
    if (FT_Read(ft, b, 1, &read) != FT_OK)
        return false;

    return read == 1;
}

bool wait_ack(FT_HANDLE ft, int timeout_ms)
{
    uint8_t r;
    uint32_t start = GET_TICK_COUNT();

    while ((GET_TICK_COUNT() - start) < (uint32_t)timeout_ms) {
        if (uart_read(ft, &r))
            return (r == ACK);
    }
    return false;
}

bool send_cmd(FT_HANDLE ft, uint8_t cmd)
{
    uint8_t p[2] = { cmd, uint8_t(cmd ^ 0xFF) };
    return uart_write(ft, p, 2) && wait_ack(ft, 1000);
}

/* ================= BOOTLOADER ================= */

bool bl_mass_erase(FT_HANDLE ft)
{
    uint8_t buf[3];

    /* ==== EXTENDED ERASE ==== */
    if (send_cmd(ft, CMD_EXT_ERASE)) {
        buf[0] = 0xFF;
        buf[1] = 0xFF;
        buf[2] = 0x00; // checksum

        uart_write(ft, buf, 3);

        if (!wait_ack(ft, 1000))
            return false;
        if (!wait_ack(ft, 20000))
            return false;

        return true;
    }

    /* ==== STANDARD ERASE ==== */
    if (!send_cmd(ft, CMD_ERASE))
        return false;

    buf[0] = 0xFF;
    buf[1] = 0x00;

    uart_write(ft, buf, 2);
    return wait_ack(ft, 20000);
}

bool bl_write(FT_HANDLE ft, uint32_t addr, uint8_t* data, int len)
{
    uint8_t buf[300];
    uint8_t cs;

    if (!send_cmd(ft, CMD_WRITE_MEMORY))
        return false;

    /* ==== ADDRESS ==== */
    buf[0] = (addr >> 24) & 0xFF;
    buf[1] = (addr >> 16) & 0xFF;
    buf[2] = (addr >> 8)  & 0xFF;
    buf[3] =  addr        & 0xFF;

    cs = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
    buf[4] = cs;

    uart_write(ft, buf, 5);
    if (!wait_ack(ft, 1000))
        return false;

    /* ==== DATA ==== */
    buf[0] = (uint8_t)(len - 1);
    memcpy(&buf[1], data, len);

    cs = buf[0];
    for (int i = 0; i < len; i++)
        cs ^= data[i];

    buf[len + 1] = cs;

    uart_write(ft, buf, len + 2);
    return wait_ack(ft, 3000);
}

bool bl_go(FT_HANDLE ft, uint32_t addr)
{
    uint8_t buf[5];
    uint8_t cs;

    if (!send_cmd(ft, CMD_GO))
        return false;

    buf[0] = (addr >> 24) & 0xFF;
    buf[1] = (addr >> 16) & 0xFF;
    buf[2] = (addr >> 8)  & 0xFF;
    buf[3] =  addr        & 0xFF;

    cs = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
    buf[4] = cs;

    uart_write(ft, buf, 5);
    return wait_ack(ft, 1000);
}
