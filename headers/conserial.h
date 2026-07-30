/// @file
/// @brief Заголовочный файл класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.
#pragma once

#ifndef CONSERIAL_H
#define CONSERIAL_H

#include <ceSerial.h>
#include <abstracthardwareapi.h>


#include <map>
#include <ctime>
#include <string>
#include <cstring>
#include <stdarg.h>
#include <vector>
// #include <algorithm>

#include <ftd2xx.h>
#include <bootloader.h>
#include <logger.h>



//uncommit this define to disable logging for COM exchange
//#define NO_SERIAL_LOG

using namespace std;
namespace hwe
{

// Общая структура версии
template<typename T>
struct Version {
    T major = 0;
    T minor = 0;
    T micro = 0;

    Version() = default;
    Version(T maj, T min) : major(maj), minor(min), micro(0) {}
    Version(T maj, T min, T mic) : major(maj), minor(min), micro(mic) {}

    // Методы сравнения
    bool operator==(const Version& other) const {
        return major == other.major
            && minor == other.minor
            && micro == other.micro;
    }

    bool operator<(const Version& other) const {
        if (major < other.major) return true;
        if (major > other.major) return false;
        if (minor < other.minor) return true;
        if (minor > other.minor) return false;
        return micro < other.micro;
    }
    bool operator>(const Version& other) const {
        if (major > other.major) return true;
        if (major < other.major) return false;
        if (minor > other.minor) return true;
        if (minor < other.minor) return false;
        return micro > other.micro;
    }

    // Строковое представление
    std::string toString() const {
        if (micro == 0) {
            return std::to_string(major) + "." + std::to_string(minor);
        }
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(micro);
    }
};

// Класс протокола с использованием общей версии
class ProtocolVersion {
public:
    enum Value {
        Unknown = 1,
        V1_0 = 2,
        V1_2 = 3,
        V1_5 = 4
    };

    ProtocolVersion(Value value = Value::V1_5) : value_(value) {}

    Value getValue() const { return value_; }
    operator Value() const { return value_; }

    Version <uint16_t> getVersion() const {
        switch(value_) {
        case Unknown: return {0, 0};
        case V1_0:    return {1, 0};
        case V1_2:    return {1, 2};
        case V1_5:    return {1, 5};
        default:      return {0, 0};
        }
    }

private:
    Value value_;
};


/// @brief Интерфейс для взаимодействия с аппаратной платформой.
class Conserial : public AbstractHardwareApi
{
public:
    enum class ErrorCode : uint16_t {
        // Ошибки общения с АП
        Success         = 0,    // Успешное выполнение запроса
        NoConnection    = 1,    // Отсутствует соединение со стендом
        InvalidResponse = 2,    // Количество принятых параметров превышает допустимый предел
        MissingFrameEnd = 3,    // Необнаружена метка конца пакета
        InvalidCommand  = 4,    // Не удалось выполнить команду / Не известный ID команды
        AccessDenied    = 5,    // Отказано в доступе (недостаточно прав)
        InvalidInput    = 6,    // Переданы неверные параметры на вход библиотечной функции
        HardwareFault   = 7,    // Аппаратная платформа в аварийном состоянии
        CrcMismatch     = 8,    // Несоответствие CRC

        // Ошибки процесса обновления прошивки (Firmware Update)
        FirmwareFileNotRead   = 9,   // Не считан файл прошивки
        BootloaderNoResponse  = 10,  // Не отвечает бутлоадер
        FlashEraseFailed      = 11,  // Ошибка стирания памяти мк
        FlashWriteFailed      = 12,  // Ошибка записи
        FlashDataEmpty        = 13,  // Нет данных в мк

        // Другие ошибки
        InternalError = 200, // Внутренние ошибки

    };

    enum class BoardType : uint16_t{
        Arduino = 0,
        STM = 1
    };

    Conserial(string port = "");
    void FindProtocolVersion();
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
    @param [in] timeout - Время ожидания в мс
    @return Установленное время ожидания и код ошибки
    */
    virtual api::AdcResponse SetTimeout(uint32_t timeout_ms);
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
    @brief Функция установки мощности лазера в кодах ЦАП
    @param [in] power - Мощность лазера в кодах ЦАП
    @return Мощность лазера в кодах ЦАП и код ошибки
    */
    virtual api::AdcResponse DAC_SetLaserPower(adc_t power);
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
    api::AdcResponse FirmwareUpdate (string path);
    /*!
    @brief Функция обновления прошивки аппаратной платформы
    @param [in] path - Путь до файла прошивки
    */
    api::AdcResponse FirmwareUpdate_Arduino (string path);
    /*!
    @brief Функция обновления прошивки аппаратной платформы
    @param [in] path - Путь до файла прошивки
    */
    api::AdcResponse FirmwareUpdate_STM (string path);
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
    api::versionProtocolResponse GetProtocolVersion ();
    /*!
    @brief Функция получения текущей версии прошивки АП
    @return {X,Y,Z}
    */
    api::versionFirmwareResponse GetCurrentFirmwareVersion();
    /*!
    @brief Функция получения максимального количества передаваемых байтов
    @return Количество байт
    */
    uint16_t GetMaxPayloadSize();
    ///@brief Получение порта подключения стенда
    std::string GetComPortName()const;
    ///@brief Установка порта подключения стенда
    void SetComPortName(const std::string& port);
    ///@brief Установка порта подключения стенда
    void SetBoardType(BoardType type);


    std::vector<std::string> GetFTDIComPorts();

private:

    /// @breif Значение для ожидания ответа при инициализации
    const uint32_t INIT_TIMEOUT_TIME = 900000;


    // enum class VersionProtocol  { unknown = 1,
    //                              protocol_1_0 = 2,
    //                              protocol_1_2 = 3,
    //                              protocol_1_5 = 4 };
    // VersionProtocol version_protocol {VersionProtocol::protocol_1_5};

    // /// @brief Структура версии прошивки АП
    // struct versionFirmware{
    //     uint16_t major = 0;
    //     uint16_t minor = 0;
    //     uint16_t micro = 0;
    // };
    // /// @brief Структура версии протокола
    // struct versionProtocol{
    //     uint16_t version = 0;
    //     uint16_t subversion = 0;
    // };
    /// @brief Структура для хранения текущей конфигурации стенда
    struct StandOptions{
        BoardType boardType = BoardType::STM;
        adc_t premissions = 0;
        adc_t laserState_ = 0;
        adc_t laserPower_ = 0;
        SLevels<hwe::adc_t> signalLevels_ = {0,0};
        WAngles<angle_t>curAngles_ = {0,0,0,0};
        SLevels<hwe::adc_t> lightNoises = {0,0};
        SLevels<hwe::adc_t> startLightNoises_= {0,0};
        WAngles<hwe::angle_t> startPlatesAngles_ = {0,0,0,0};
        SLevels<hwe::adc_t> maxSignalLevels_ = {0,0};
        uint32_t timeoutTime_ = 2000; //ms
        angle_t rotateStep_ = 0.3;
        adc_t maxLaserPower_ = 100;
        adc_t maxPayloadSize = 30;
    };
    /// @brief Структура для парсинга транспортных пакетов
    struct UartResponse{
        uint8_t status_= 0;
        uint8_t nameCommand_ = 0;
        uint8_t crc_= 0;
        vector <uint16_t> parameters_;
        uint16_t payload = 0;
    };

    //Конфигурация
    ProtocolVersion v_protocol;
    Version<uint16_t> versionFirmware = {1,0,0};

    Conserial::StandOptions standOptions; // Структура, хранящая текущее состояние стенда


    std::unique_ptr<ce::ceSerial> com_;; // УКАЗАТЕЛЬ класса для соединения с МК

    /// @brief Подсчет CRC
    uint8_t Crc8(uint8_t *pcBlock, uint8_t len);

    /// @brief Чтение пакетов с UART
    std::vector<uint8_t> ReadPacket();

    /// @brief Парсинг пакетов
    UartResponse ParsePacket();

    /// @brief Подсчет из угла в шаг
    uint16_t CalcStep(angle_t angle, angle_t rotateStep);

    /// @brief Подсчет из углов в шаги
    WAngles<adc_t> CalcSteps(WAngles<angle_t> angles);

    /// @brief Подсчет из углов в шаги
    WAngles<angle_t> CalcAngles(WAngles<adc_t> steps);

    /// @brief Отправка и получение ответов
    /// @param [in] commandName - ID команды
    /// @param [in] bytes - Массив передаваемых байтов
    /// @param [in] length - Количество передаваемых байтов
    /// @return Распаршеный пакет
    UartResponse Twiting (uint8_t commandName,  uint8_t * bytes = nullptr, uint16_t length = 0);

    /// @brief Отправка запросов
    /// @param [in] commandName - ID команды
    /// @param [in] bytes - Массив передаваемых байтов
    /// @param [in] N - Количество передаваемых байтов
    /// @return 1
    uint16_t SendPacket (uint8_t commandName, uint8_t  * bytes, uint16_t N );

    /// @brief Парсинг кодов ошибок с АП
    /// @return Статус
    uint16_t CheckStatus(uint16_t status);

    /// @brief Проверка подключения к АП
    bool StandIsConected ();

    std::vector<uint8_t> PreparePasswordBytes(const string& passwd);

    template<typename... Args>
    std::vector<uint8_t> PackToBytes(Args... args)
    {
        std::vector<uint8_t> bytes;

        auto push = [&](auto value)
        {
            using T = std::decay_t<decltype(value)>;

            // Проверяем, является ли тип вектором
            if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                // Если это вектор байт - просто копируем его
                bytes.insert(bytes.end(), value.begin(), value.end());
            }
            else if constexpr (std::is_same_v<T, std::vector<char>>) {
                // Если вектор char - конвертируем в uint8_t
                for (auto c : value) {
                    bytes.push_back(static_cast<uint8_t>(c));
                }
            }
            else if constexpr (std::is_arithmetic_v<T>) {
                using U = std::make_unsigned_t<T>;
                U v = static_cast<U>(value);

                // Записываем байты от старшего к младшему (Big Endian)
                for (int i = sizeof(U) - 1; i >= 0; --i) {
                    bytes.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFF));
                }
            }
            else {
                // Для остальных типов - пробуем привести к строке
                static_assert(sizeof(T) == 0, "Невозможно преобразовать в байт массив");
            }
        };

        (push(args), ...);
        return bytes;
    }

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
    const std::map <std::string, uint8_t> dict_ = {
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
        {"DAC_SetLaserPower", 0x45}, //E
        {"SetTimeout", 0x46 },    //F
        {"GetHardwareState", 0x47 },  //G
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

    //_____________________________________________________________________________________________
    // Шаблоны
    //_____________________________________________________________________________________________

    template<typename ResponseType>
    bool CheckStandPremmissions(ResponseType &response){
        if(standOptions.premissions!=1){
            LOG_WARNING("Отказано в доступе (недостаточно прав)");
            response.errorCode_ = static_cast<uint16_t>(ErrorCode::AccessDenied);
            return false;
        }
        return true;
    }

    // Упрощенный базовый шаблон только для отправки команд
    template<typename ResponseType, typename... Args>
    ResponseType SendCommand(const std::string& cmd, Args... args) {

        LOG_DEBUG("Отправленная команда: " + cmd);

        ResponseType response{};
        auto bytes = PackToBytes(args...);
        UartResponse pack{};

        try {
            uint8_t cmdByte = dict_.at(cmd);
            pack = Twiting(cmdByte, bytes.data(), bytes.size());
        } catch (const std::out_of_range& e) {
            pack.status_ = static_cast<uint16_t>(ErrorCode::InvalidCommand);
            response.errorCode_ = pack.status_;
            return response;
        }

        if(pack.status_ != 1) {
            response.errorCode_ = CheckStatus(pack.status_);
            return response;
        }

        size_t expectedParams = GetExpectedParams<ResponseType>();
        if(pack.parameters_.size() != expectedParams) {
            response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
            return response;
        }

        FillResponse(response, pack);
        response.errorCode_ = CheckStatus(pack.status_);
        return response;
    }

    // Упрощенный FillResponse - только заполнение ответа
    template<typename ResponseType>
    void FillResponse(ResponseType& resp, const UartResponse& pack) {
        LOG_DEBUG("FillResponse: pack.parameters_.size() = " + to_string(pack.parameters_.size()));

        if constexpr (std::is_same_v<ResponseType, api::AngleResponse>) {
            if (pack.parameters_.at(0) > 0)
                resp.angle_ = 360.0 / pack.parameters_.at(0);
            else
                resp.angle_ = 0;
        }
        if constexpr (std::is_same_v<ResponseType, api::AdcResponse>) {
            resp.adcResponse_ = pack.parameters_.at(0);
        }
        else if constexpr (std::is_same_v<ResponseType, api::versionProtocolResponse>) {
            resp.version_ = pack.parameters_.at(0);
            resp.subversion_ = pack.parameters_.at(1);
        }
        else if constexpr (std::is_same_v<ResponseType, api::versionFirmwareResponse>) {
            resp.major_ = pack.parameters_.at(0);
            resp.minor_ = pack.parameters_.at(1);
            resp.micro_ = pack.parameters_.at(2);
        }
        else if constexpr (std::is_same_v<ResponseType, api::SLevelsResponse>) {
            resp.signal_.h_ = pack.parameters_.at(0);
            resp.signal_.v_ = pack.parameters_.at(1);
        }
        else if constexpr (std::is_same_v<ResponseType, api::WAnglesResponse>) {
            WAngles<adc_t> steps = {
                pack.parameters_.at(0),
                pack.parameters_.at(1),
                pack.parameters_.at(2),
                pack.parameters_.at(3)
            };
            resp.angles_ = CalcAngles(steps);
        }
        else if constexpr (std::is_same_v<ResponseType, api::InitResponse>) {
            resp.startPlatesAngles_ = CalcAngles({
                pack.parameters_.at(0), pack.parameters_.at(1),
                pack.parameters_.at(2), pack.parameters_.at(3)
            });
            resp.startLightNoises_ = {pack.parameters_.at(4), pack.parameters_.at(5)};
            resp.maxSignalLevels_ = {pack.parameters_.at(6), pack.parameters_.at(7)};
            resp.maxLaserPower_ = pack.parameters_.at(8);
        }
        else if constexpr (std::is_same_v<ResponseType, api::SendMessageResponse>) {
            resp.newPlatesAngles_ = CalcAngles({
                pack.parameters_.at(0), pack.parameters_.at(1),
                pack.parameters_.at(2), pack.parameters_.at(3)
            });
            resp.currentLightNoises_ = {pack.parameters_.at(4), pack.parameters_.at(5)};
            resp.currentSignalLevels_ = {pack.parameters_.at(6), pack.parameters_.at(7)};
        }
    }
    /**
 * @brief Определяет ожидаемое количество параметров для типа ответа
 *
 * Используется для валидации полученного пакета - если количество параметров
 * не совпадает с ожидаемым, возвращается ошибка InvalidResponse.
 *
 * @tparam T Тип ответа
 * @return constexpr size_t Ожидаемое количество параметров
 *
 * @note Соответствие типов и количества параметров:
 * - AdcResponse, AngleResponse: 1 параметр
 * - SLevelsResponse, versionProtocolResponse: 2 параметра
 * - versionFirmwareResponse: 3 параметра
 * - WAnglesResponse: 4 параметра
 * - SendMessageResponse: 8 параметров
 * - InitResponse: 9 параметров
 */
    template<typename T>
    constexpr size_t GetExpectedParams() {
        if constexpr (std::is_same_v<T, api::AdcResponse> ||
                      std::is_same_v<T, api::AngleResponse>) {
            return 1;
        } else if constexpr (std::is_same_v<T, api::SLevelsResponse> ||
                             std::is_same_v<T, api::versionProtocolResponse> ) {
            return 2;
        } else if constexpr (std::is_same_v<T, api::versionFirmwareResponse>) {
            return 3;
        } else if constexpr (std::is_same_v<T, api::WAnglesResponse>) {
            return 4;
        } else if constexpr (std::is_same_v<T, api::InitResponse>) {
            return 9;
        } else if constexpr (std::is_same_v<T, api::SendMessageResponse>) {
            return 8;
        }
        return 0;
    }
};




} //namespace
#endif // CONSERIAL_H
