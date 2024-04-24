#include "ceSerial.h"

#ifdef CE_WINDOWS
#include <windows.h>
#include <winbase.h>
#include <iostream>
#include <fstream>
#define READ_TIMEOUT 10      // milliseconds
#include <chrono>

using namespace std;
namespace ce {

void ceSerial::Delay(unsigned long ms)
{
    Sleep(ms);
}

ceSerial::ceSerial() :
    ceSerial("\\\\.\\COM1", 9600, 8, 'N', 1)
{

}

ceSerial::~ceSerial() {
    Close();
}

ceSerial::ceSerial(std::string Device, long BaudRate,long DataSize,char ParityType,float NStopBits):
    stdbaud(true)
{
    hComm = INVALID_HANDLE_VALUE;
    SetBaudRate(BaudRate);
    SetDataSize(DataSize);
    SetParity(ParityType);
    SetStopBits(NStopBits);
    SetPort(Device);
}

void ceSerial::SetPort(std::string Port)
{
    port_ = Port;
}

std::string ceSerial::GetPort()const
{
    return port_;
}

void ceSerial::SetParity(char p)
{
    if ((p != 'N') && (p != 'E') && (p != 'O'))
    {
        if ((p != 'M') && (p != 'S'))
            p = 'N';
    }
    parity_ = p;
}

char ceSerial::GetParity() {
    return parity_;
}

void ceSerial::SetStopBits(float nbits)
{
    if (nbits >= 2)
        stopbits_ = 2;
    else if(nbits >= 1.5)
        stopbits_ = 1.5;
    else
        stopbits_ = 1;
}

float ceSerial::GetStopBits() {
    return stopbits_;
}

void ceSerial::SetBaudRate(long baudrate)
{
    stdbaud = true;
    if (baudrate == 1100) baud_ = CBR_110;
    else if (baudrate == 300) baud_ = CBR_300;
    else if (baudrate == 600) baud_ = CBR_600;
    else if (baudrate == 1200) baud_ = CBR_1200;
    else if (baudrate == 2400) baud_ = CBR_2400;
    else if (baudrate == 4800) baud_ = CBR_4800;
    else if (baudrate == 9600) baud_ = CBR_9600;
    else if (baudrate == 14400) baud_ = CBR_14400;
    else if (baudrate == 19200) baud_ = CBR_19200;
    else if (baudrate == 38400) baud_ = CBR_38400;
    else if (baudrate == 57600) baud_ = CBR_57600;
    else if (baudrate == 115200) baud_ = CBR_115200;
    else if (baudrate == 128000) baud_ = CBR_128000;
    else if (baudrate == 256000) baud_ = CBR_256000;
    else {
        baud_ = baudrate;
        stdbaud = false;
    }
}

long ceSerial::GetBaudRate()
{
    return baud_;
}

long ceSerial::Open()
{
    if (IsOpened()) return 0;
#ifdef UNICODE
    wstring wtext(port_.begin(),port_.end());
#else
    std::string wtext = port_;
#endif
    hComm = CreateFile(wtext.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, //| FILE_FLAG_OVERLAPPED,
        0);
    if (hComm == INVALID_HANDLE_VALUE) {return -1;}

    if (PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0) {return -1;}//purge

    //get initial state
    DCB dcbOri = {0};
    bool fSuccess;
    fSuccess = GetCommState(hComm, &dcbOri);
    if (!fSuccess) {return -1;}

    DCB dcb1 = dcbOri;

    dcb1.BaudRate = baud_;

    if (parity_ == 'E') dcb1.Parity = EVENPARITY;
    else if (parity_ == 'O') dcb1.Parity = ODDPARITY;
    else if (parity_ == 'M') dcb1.Parity = MARKPARITY;
    else if (parity_ == 'S') dcb1.Parity = SPACEPARITY;
    else dcb1.Parity = NOPARITY;

    dcb1.ByteSize = (BYTE)dsize_;

    if(stopbits_ == 2) dcb1.StopBits = TWOSTOPBITS;
    else if (stopbits_ == 1.5) dcb1.StopBits = ONE5STOPBITS;
    else dcb1.StopBits = ONESTOPBIT;

    dcb1.fOutxCtsFlow = false;
    dcb1.fOutxDsrFlow = false;
    dcb1.fOutX = false;
    dcb1.fDtrControl = DTR_CONTROL_DISABLE;
    dcb1.fRtsControl = RTS_CONTROL_DISABLE;
    fSuccess = SetCommState(hComm, &dcb1);
    this->Delay(60);
    if (!fSuccess) {return -1;}

    fSuccess = GetCommState(hComm, &dcb1);
    if (!fSuccess) {return -1;}

    //osReader = { 0 };// Create the overlapped event.
    // Must be closed before exiting to avoid a handle leak.
    //osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    //if (osReader.hEvent == NULL) {return -1;}// Error creating overlapped event; abort.
    //fWaitingOnRead = FALSE;

    //osWrite = { 0 };
    //osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    //if (osWrite.hEvent == NULL) {return -1;}

    if (!GetCommTimeouts(hComm, &timeouts_ori)) { return -1; } // Error getting time-outs.
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = READ_TIMEOUT;
    timeouts.ReadTotalTimeoutMultiplier = 0;//15;
    timeouts.ReadTotalTimeoutConstant = 10;//100;
    timeouts.WriteTotalTimeoutMultiplier = 10;//15;
    timeouts.WriteTotalTimeoutConstant = 10;//100;
    if (!SetCommTimeouts(hComm, &timeouts)) { return -1;} // Error setting time-outs.
    return 0;
}

void ceSerial::SetDataSize(long nbits)
{
    if ((nbits < 5) || (nbits > 8)) nbits = 8;
        dsize_ = nbits;
}

long ceSerial::GetDataSize()
{
    return dsize_;
}

void ceSerial::Close()
{
    if (IsOpened())
    {
        SetCommTimeouts(hComm, &timeouts_ori);
        //CloseHandle(osReader.hEvent);
        //CloseHandle(osWrite.hEvent);
        CloseHandle(hComm);//close comm port
        hComm = INVALID_HANDLE_VALUE;
    }
}

bool ceSerial::IsOpened() {
    if(hComm == INVALID_HANDLE_VALUE)
        return false;
    else
        return true;
}

bool ceSerial::Write(uint16_t data)
{
    if (!IsOpened())
        return false;

    DWORD written = 0;
    return WriteFile(hComm, &data, 2, &written, 0);
}

bool ceSerial::Write(uint8_t data)
{
    if (!IsOpened())
        return false;

    DWORD written = 0;
    return WriteFile(hComm, &data, 1, &written, 0);
}

bool ceSerial::Write(char *data)
{
    if (!IsOpened())
    {
        return false;
    }
    BOOL fRes;
    DWORD dwWritten;
    long n = strlen(data);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;

    fRes = WriteFile(hComm, data, n, &dwWritten, 0);
//    // Issue write.
//    if (!WriteFile(hComm, data, n, &dwWritten, &osWrite)) {
//        // WriteFile failed, but it isn't delayed. Report error and abort.
//        if (GetLastError() != ERROR_IO_PENDING) {fRes = FALSE;}
//        else {// Write is pending.
//            if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) fRes = FALSE;
//            else fRes = TRUE;// Write operation completed successfully.
//        }
//    }
//    else fRes = TRUE;// WriteFile completed immediately.
    return fRes;
}

bool ceSerial::Write(char *data, long n)
{
    if (!IsOpened())
    {
        return false;
    }
    BOOL fRes;
    DWORD dwWritten;
    if (n < 0)
        n = 0;
    else
        if(n > 1024)
            n = 1024;

    fRes = WriteFile(hComm, data, n, &dwWritten, 0);
//    // Issue write.
//    if (!WriteFile(hComm, data, n, &dwWritten, &osWrite)) {
//        // WriteFile failed, but it isn't delayed. Report error and abort.
//        if (GetLastError() != ERROR_IO_PENDING) {fRes = FALSE;}
//        else {// Write is pending.
//            if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) fRes = FALSE;
//            else fRes = TRUE;// Write operation completed successfully.
//        }
//    }
//    else fRes = TRUE;// WriteFile completed immediately.
    return fRes;
}

bool ceSerial::WriteChar(char ch)
{
    char s[2];
    s[0] = ch;
    s[1] = 0;//null terminated
    return Write(s);
}

char ceSerial::ReadChar()
{
    if (!IsOpened())
        return 0;
    DWORD dwRead;
    if (!ReadFile(hComm, &rxchar_, 1, &dwRead, 0))
        rxchar_ = 0;

    return rxchar_;
}

char ceSerial::ReadChar(bool& success)
{
    success = false;
    if (!IsOpened())
        return 0;
    DWORD dwRead;

    success = ReadFile(hComm, &rxchar_, 1, &dwRead, 0);
    if (!success)
        rxchar_ = 0;

    return rxchar_;
}

bool ceSerial::SetRTS(bool value)
{
    bool r = false;
    if (IsOpened()) {
        if (value) {
            if (EscapeCommFunction(hComm, SETRTS)) r = true;
        }
        else {
            if (EscapeCommFunction(hComm, CLRRTS)) r = true;
        }
    }
    return r;
}

bool ceSerial::SetDTR(bool value)
{
    bool r = false;
    if (IsOpened()) {
        if (value) {
            if (EscapeCommFunction(hComm, SETDTR)) r = true;
        }
        else {
            if (EscapeCommFunction(hComm, CLRDTR)) r = true;
        }
    }
    return r;
}

bool ceSerial::GetCTS(bool& success)
{
    success = false;
    bool r = false;
    if (IsOpened()) {
        DWORD dwModemStatus;
        if (GetCommModemStatus(hComm, &dwModemStatus)){
            r = MS_CTS_ON & dwModemStatus;
            success = true;
        }
    }
    return r;
}

bool ceSerial::GetDSR(bool& success)
{
    success = false;
    bool r = false;
    if (IsOpened()) {
        DWORD dwModemStatus;
        if (GetCommModemStatus(hComm, &dwModemStatus)) {
            r = MS_DSR_ON & dwModemStatus;
            success = true;
        }
    }
    return r;
}

bool ceSerial::GetRI(bool& success)
{
    success = false;
    bool r = false;
    if (IsOpened()) {
        DWORD dwModemStatus;
        if (GetCommModemStatus(hComm, &dwModemStatus)) {
            r = MS_RING_ON & dwModemStatus;
            success = true;
        }
    }
    return r;
}

bool ceSerial::GetCD(bool& success)
{
    success = false;
    bool r = false;
    if (IsOpened()) {
        DWORD dwModemStatus;
        if (GetCommModemStatus(hComm, &dwModemStatus)) {
            r = MS_RLSD_ON & dwModemStatus;
            success = true;
        }
    }
    return r;
}

} //end namespace
#endif
