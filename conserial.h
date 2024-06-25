/// @file
/// @brief Заголовочный файл класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef CONSERIAL_H
#define CONSERIAL_H

#include <ceSerial.h>
#include <abstracthardwareapi.h>
#include <map>
#include <ctime>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdarg.h>

#include <cstddef>
#include <bitset>
#include <chrono>


//uncommit this define to disable logging for COM exchange
//#define NO_SERIAL_LOG

using namespace std;
namespace hwe
{

/// @brief Интерфейс для взаимодействия с аппаратной платформой.
class Conserial : public AbstractHardwareApi
{
public:
    struct versionFirmwareResponse{
        uint16_t major_ = 0;
        uint16_t minor_ = 0;
        uint16_t micro_ = 0;
        adc_t errorCode_ = 0;
    };
    struct versionProtocolResponse{
        uint16_t version_ = 0;
        uint16_t subversion_ = 0;
        adc_t errorCode_ = 0;
    };

    Conserial();
    virtual ~Conserial();
    /*!
    @brief Функция инициализации стенда
    Проверяет существует ли файл с начальными углами,
    если существует то проводит инициализацию
    по фотодетекторам InitByPD(), иначе по концевикам InitByButtons()
    @return Углы поворота пластин, начальная засветка фотодетекторов, максимальный уровень сигнала на фотодетекторах и максимальную мощность лазера
    */
    virtual api::InitResponse Init();
    /*!
    @brief Функция инициализации стенда по фотодетекторам
    @return Углы поворота пластин, начальная засветка фотодетекторов, максимальный уровень сигнала на фотодетекторах и максимальную мощность лазера и код ошибки
    */
    virtual api::InitResponse InitByPD();
    /*!
    @brief Функция инициализации стенда по фотодетекторам
    @param  [in] angles - Начальные углы стенда
    @return Углы поворота пластин, начальная засветка фотодетекторов, максимальный уровень сигнала на фотодетекторах и максимальную мощность лазера и код ошибки
    */
    virtual api::InitResponse InitByButtons(WAngles<angle_t> angles);

    /*!
    @brief Функция тестирования стенда
    @return Статус выполнения теста и код ошибки
    */
    virtual api::AdcResponse RunTest();
    /*!
    @brief Функция отправки битовой последовательности
    @param [in] angles - Углы поворотов волновых пластин для передачи сообщения.
    @param [in] power - Мощность лазера для передачи сообщения.
    @return Углы поворота пластин, начальная засветка фотодетекторов, максимальный уровень сигнала на фотодетекторах и максимальную мощность лазера и код ошибки
    */
    virtual api::SendMessageResponse Sendmessage(WAngles<angle_t> angles, adc_t power);
    /*!
    @brief Функция установки времени ожидания ответа от аппаратной платформы
    @param [in] timeout - Время ожидания в СЕКУНДАХ
    @return Установленное время ожидания и код ошибки
    */
    virtual api::AdcResponse SetTimeout(adc_t timeout);
    /*!
    @brief Функция включения и выключения лазера
    @param [in] on - Состояние лазера (1\0)
    @return Состояние лазера и код ошибки
    */
    virtual api::AdcResponse SetLaserState(adc_t on);
    /*!
    @brief Функция установки мощности лазера
    @param [in] power - Мощность лазера
    @return Мощность лазера и код ошибки
    */
    virtual api::AdcResponse SetLaserPower(adc_t power);
    /*!
    @brief Функция установки углов поворота пластин
    @param [in] angles - Углы поворотов волновых пластин для передачи сообщения.
    @return Установленные углы поворота пластин и код ошибки
    */
    virtual api::WAnglesResponse SetPlatesAngles(WAngles<angle_t> angles);
    /*!
    @brief Функция получения состояния лазера
    @return Состояние лазера и код ошибки
    */
    virtual api::AdcResponse GetLaserState();
    /*!
    @brief Функция получения мощности лазера
    @return Мощность лазера и код ошибки
    */
    virtual api::AdcResponse GetLaserPower();
    /*!
    @brief Функция получения параметров, полученных при инициализации стенда по фотодетекторам
    @return Углы поворота пластин, начальная засветка фотодетекторов, максимальный уровень сигнала на фотодетекторах и максимальную мощность лазера и код ошибки
    */
    api::InitResponse  GetInitParams();
    /*!
    @brief Функция получения максимального значения мощности лазера
    @return Максимальная мощность лазера и код ошибки
    */
    virtual api::AdcResponse GetMaxLaserPower();
    /*!
    @brief Функция получения начальных углов поворота пластин
    @return Углы поворота пластин и код ошибки
    */
    virtual api::WAnglesResponse GetStartPlatesAngles();
    /*!
    @brief Функция получения засветки на фотодетекторах при инициализации
    @return Засветка на фотодетекторах и код ошибки
    */
    virtual api::SLevelsResponse GetStartLightNoises();
    /*!
    @brief Функция получения максимального значения на фотодетекторах при инициализации
    @return Максимальное значение на фотодетекторах и код ошибки
    */
    virtual api::SLevelsResponse GetMaxSignalLevels();
    /*!
    @brief Функция установки угла поворота пластины
    @param [in] plateNumber - Номер пластины для поворота
    @param [in] angle - Угол поворота волновой пластины № plateNumber
    @return Установленные углы поворота пластин и код ошибки
    */
    virtual api::AngleResponse SetPlateAngle(adc_t plateNumber, angle_t angle);
    /*!
    @brief Функция получения текущих углов поворота пластин
    @return Углы поворота пластин и код ошибки
    */
    virtual api::WAnglesResponse GetPlatesAngles();
    /*!
    @brief Функция получения текущих значений на фотодетекторах
    @return Значения на фотодетекторах и код ошибки
    */
    virtual api::SLevelsResponse GetSignalLevels();
    /*!
    @brief Функция получения угла при повороте двигателя на один шаг
    @return Шаг двигателя (в углах) и код ошибки
    */
    virtual api::AngleResponse GetRotateStep();
    /*!
    @brief Функция получения текущей засветки на фотодетекторах
    @return Значения текущей засветки на фотодетекторах и код ошибки
    */
    virtual api::SLevelsResponse GetLightNoises();
    /*!
    @brief Функция получения текущего состояния аппаратной платформы
    @return Статус АП и код ошибки
    */
    api::AdcResponse GetHardwareState();
    /*!
    @brief Функция получения текущего состояния аппаратной платформы (Версия 1.0)
    @return Статус АП и код ошибки
    */
    virtual api::AdcResponse GetErrorCode();
    /*!
    @brief Функция получения времени ожидания ответа от аппаратной платформы
    @return Время ожидания и код ошибки
    */
    virtual api::AdcResponse GetTimeout();
    /*!
    @brief Функция обновления начальных углов в ПЗУ аппаратной платформы для инициализации по датчикам
    @param [in] angles - Углы поворотов волновых пластин для передачи сообщения.
    @return Углы поворотов пластин и код ошибки
    */
    virtual api::WAnglesResponse UpdateBaseAngle(WAngles<angle_t> angles);
    /*!
    @brief Функция получения начальных углов из ПЗУ аппаратной платформы для инициализации по датчикам
    @return Углы поворотов пластин и код ошибки
    */
    virtual api::WAnglesResponse ReadBaseAngles();
    /*!
    @brief Функция получения данных из ПЗУ аппаратной платформы
    @param [in] numberUnit_ - Номер ячейки памяти
    @return Данные из ячейки памяти и код ошибки
    */
    virtual api::AdcResponse ReadEEPROM(uint8_t numberUnit_);
    /*!
    @brief Функция получения данных из ПЗУ аппаратной платформы
    @param [in] numberUnit_ - Номер ячейки памяти
    @param [in] param_ - Данные для записи в ПЗУ
    @return Данные из ячейки памяти и код ошибки
    */
    virtual api::AdcResponse WriteEEPROM(uint8_t numberUnit_, uint16_t param_);
    /*!
    @brief Функция обновления прошивки аппаратной платформы
    @param [in] path - Путь до файла прошивки
    */
    void FirmwareUpdate (string path);
    /*!
    @brief Функция задания пароля для входа в технологический режим
    @param [in] passwd - Пароль для входа в тех. режим
    @return Статус операции и код ошибки
    */
    api::AdcResponse CreateConfigSecret(string passwd);
    /*!
    @brief Функция входа в технологический режим
    @param [in] passwd - Пароль для входа в тех. режим
    @return Статус операции и код ошибки
    */
    api::AdcResponse OpenConfigMode(string passwd);
    /*!
    @brief Функция выхода из технологического режима
    @return Статус операции и код ошибки
    */
    uint16_t CloseConfigMode();
    /*!
    @brief Функция получения текущего режима работы (технологический/штатный)
    @return 0 - штатный режим, 1 - технологический
    */
    uint16_t GetCurrentMode();

    /*!
    @brief Функция получения текущей версии протокола общения
    @return {X,Y,Z}
    */
    hwe::Conserial::versionProtocolResponse GetProtocolVersion ();
    /*!
    @brief Функция получения текущей версии прошивки АП
    @return {X,Y,Z}
    */
    hwe::Conserial::versionFirmwareResponse GetCurrentFirmwareVersion();
    /*!
    @brief Функция получения максимального количества передаваемых байтов
    @return Количество байт
    */
    uint16_t GetMaxPayloadSize();
    ///@brief Получение порта подключения стенда
    std::string GetComPortName()const;
    ///@brief Установка порта подключения стенда
    void SetComPortName(const char* port);

private:
    enum class VersionProtocol {unknown, protocol_1_0 = 2, protocol_1_2 = 3, protocol_1_5 = 4 };
    VersionProtocol version_protocol {VersionProtocol::protocol_1_5};

    /// @brief Структура версии прошивки АП
    struct versionFirmware{
        uint16_t major = 0;
        uint16_t minor = 0;
        uint16_t micro = 0;
    };
    /// @brief Структура версии протокола
    struct versionProtocol{
        uint16_t version = 0;
        uint16_t subversion = 0;
    };

    //Конфигурация
    versionFirmware versionFirmware = {1,0,0};
    versionProtocol versionProtocol = {1, 5};

    struct StandOptions{
        adc_t premissions = 0;
        adc_t laserState_ = 0;
        adc_t laserPower_ = 0;
        SLevels<hwe::adc_t> signalLevels_ = {0,0};
        WAngles<angle_t>curAngles_ = {0,0,0,0};
        SLevels<hwe::adc_t> lightNoises = {0,0};
        SLevels<hwe::adc_t> startLightNoises_= {0,0};
        WAngles<hwe::angle_t> startPlatesAngles_ = {0,0,0,0};
        SLevels<hwe::adc_t> maxSignalLevels_ = {0,0};
        adc_t timeoutTime_ = 2; //sec
        angle_t rotateStep_ = 0.3;
        adc_t maxLaserPower_ = 100;
        adc_t maxPayloadSize = 30;
    };

    Conserial::StandOptions standOptions; // Структура, хранящая текущее состояние стенда

    struct UartResponse{
        uint8_t status_= 0;
        uint8_t nameCommand_ = 0;
        uint8_t crc_= 0;
        uint16_t parameters_ [10] = {0,0,0,0,0,0,0,0,0,0};
        uint16_t payload = 0;
    };

    ce::ceSerial com_; // Обект класса для соединения с МК
    /// @brief Подсчет CRC
    uint8_t Crc8(uint8_t *pcBlock, uint8_t len);
    int ReadPacket(uint8_t *readBytes, int N);
    UartResponse ParsePacket();
    /// @brief Парсинг пакетов версии 1.2 и 1.5
    UartResponse ParsePackege(unsigned int timeout);
    /// @brief Парсинг пакетов версии 1.0
    UartResponse ParsePackege_1_0(unsigned int timeout);

    /// @brief Подсчет из угла в шаг
    uint16_t CalcStep(angle_t angle, angle_t rotateStep);
    /// @brief Подсчет из углов в шаги
    WAngles<adc_t> CalcSteps(WAngles<angle_t> angles);
    /// @brief Подсчет из углов в шаги
    WAngles<angle_t> CalcAngles(WAngles<adc_t> steps);
    /// @brief Отправка и получение ответов
    /// @param [in] commandName - ID команды
    /// @param [in] N - Количество передаваемых параметров
    /// @param ... - Параметры
    /// @return Распаршеный пакет
    UartResponse Twiting (char commandName, int N,... );
    /// @brief Отправка и получение ответов
    /// @param [in] commandName - ID команды
    /// @param [in] bytes - Массив передаваемых байтов
    /// @param [in] length - Количество передаваемых байтов
    /// @return Распаршеный пакет
    UartResponse Twiting (char commandName, uint8_t * bytes, uint16_t length);
    /// @brief Отправка запросов
    /// @param [in] commandName - ID команды
    /// @param [in] bytes - Массив передаваемых байтов
    /// @param [in] N - Количество передаваемых байтов
    /// @return 1
    uint16_t SendUart (char commandName, uint8_t * bytes, uint16_t N );

    uint16_t SendPacket (char commandName, uint8_t * bytes, uint16_t N );
    /// @brief Парсинг кодов ошибок с АП
    /// @return Статус
    uint8_t CheckStatus(uint8_t status);
    /// @brief Проверка подключения к АП
    bool StandIsConected ();

    void FindProtocolVersion();

    void ParamToBytes(uint8_t * bytes, int &quantityP, ...);


    //Loging
    std::ofstream out_;
    void logOut(string str);
    const string currentDateTime();
    void logOutUart(const UartResponse &pack);

    //Таблица для подсчёта CRC
    const uint8_t Crc8Table[256] = {
        0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
        0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
        0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
        0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
        0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
        0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
        0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
        0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
        0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
        0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
        0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
        0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
        0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
        0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
        0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
        0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
        0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
        0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
        0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
        0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
        0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
        0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
        0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
        0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
        0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
        0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
        0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
        0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
        0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
        0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
        0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
        0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
    };


    //Мапа название команды -- ключ команды
    const std::map <std::string, char> dict_ = {
        {"GetProtocolVersion", 0x10},
        {"GetCurrentFirmwareVersion", 0x11},
        {"GetMaxPayloadSize", 0x11},
        {"CreateConfigSecret", 0x30},
        {"OpenConfigMode", 0x31},
        {"CloseConfigMode", 0x32},
        {"GetCurrentMode", 0x33},
        {"Init", 0x41 }, //A
        {"SendMessage", 0x42 }, //B
        {"SetLaserState", 0x43 }, //C
        {"SetLaserPower", 0x44 }, //D
        {"SetTimeout", 0x46 },    //F
        {"GetHardwareState", 0x47 },  //G
        {"GetErrorCode", 0x47},
        {"GetLaserState",  0x48}, //H
        {"GetLaserPower",  0x49}, //I
        {"GetTimeout", 0x4A},     //J
        {"GetInitParams", 0x4B},  //K
        {"GetCurPlatesAngles", 0x4C}, //L
        {"GetSignalLevel", 0x4D}, //M
        {"GetRotateStep", 0x4E},  //N
        {"GetLightNoises", 0x50}, //P
        {"RunSelfTest", 0x53},  //S
        {"InitByButtons", 0x54},  //T
        {"SetPlatesAngles", 0x55}, //U
        {"ReadEEPROM", 0x56}, //V
        {"WriteEEPROM", 0x57}, //W
        {"UpdateBaseAngles", 0x58}, //X
        {"ReadBaseAngles", 0x59}  //Y
    };
};

} //namespace
#endif // CONSERIAL_H
