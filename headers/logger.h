#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <thread>
#include <ctime>
#include <type_traits>

enum class LogLevel {
    DEBUG_LEVEL = 0,
    INFO_LEVEL = 1,
    WARNING_LEVEL = 2,
    ERROR_LEVEL = 3,
    CRITICAL_LEVEL = 4,
    OFF_LEVEL = 5
};

class Logger {
public:
    // Получение экземпляра (Singleton)
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    // Настройка логирования
    void initialize(const std::string& filename = "log.txt",
                    LogLevel level = LogLevel::INFO_LEVEL,
                    bool console_output = true,
                    bool enable_function_logging = true) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_log_level = level;
        m_console_output = console_output;
        m_enable_function_logging = enable_function_logging;

        if (m_file.is_open()) {
            m_file.close();
        }

        if (!filename.empty()) {
            m_file.open(filename, std::ios::app);
            if (!m_file.is_open()) {
                std::cerr << "Ошибка открытия файла лога: " << filename << std::endl;
            }
        }
    }

    // ======== ОСНОВНЫЕ МЕТОДЫ ЛОГИРОВАНИЯ ========

    template<typename... Args>
    void debug(const Args&... args) {
        log(LogLevel::DEBUG_LEVEL, args...);
    }

    template<typename... Args>
    void info(const Args&... args) {
        log(LogLevel::INFO_LEVEL, args...);
    }

    template<typename... Args>
    void warning(const Args&... args) {
        log(LogLevel::WARNING_LEVEL, args...);
    }

    template<typename... Args>
    void error(const Args&... args) {
        log(LogLevel::ERROR_LEVEL, args...);
    }

    template<typename... Args>
    void critical(const Args&... args) {
        log(LogLevel::CRITICAL_LEVEL, args...);
    }

    // ======== СПЕЦИАЛЬНЫЕ МЕТОДЫ ДЛЯ ОТЛАДКИ ========

    // Простой вывод строки
    void logOut(const std::string& str) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open()) {
            m_file << str << std::endl;
            m_file.flush();
        }
        if (m_console_output) {
            std::cout << str << std::endl;
        }
    }

    // Вывод без перевода строки (для построения сложных сообщений)
    void logOutLine(const std::string& str) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open()) {
            m_file << str;
            m_file.flush();
        }
        if (m_console_output) {
            std::cout << str;
        }
    }

    // Логирование функции с параметрами
    template<typename... Args>
    void logFunctionCall(const char* funcName, Args... args) {
        if (!m_enable_function_logging) return;

        logOut("==============================================================");
        logOut(">>  ФУНКЦИЯ: " + std::string(funcName));
        logOut(">> Количество параметров: " + std::to_string(sizeof...(args)));

        if constexpr (sizeof...(args) > 0) {
            logOut(">  Параметры:");
            std::stringstream ss;
            ((ss << "    " << args), ...);
            logOut(ss.str());
        }
        logOut("==============================================================");
    }

    // Логирование параметров в HEX формате
    template<typename... Args>
    void logParametersHex(Args... args) {
        if (m_log_level < LogLevel::DEBUG_LEVEL) {
            return;
        }
        std::stringstream ss;
        ss << "[HEX] ";

        auto push = [&](auto value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, uint8_t> ||
                          std::is_same_v<T, unsigned char>) {
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(value) << " ";
            }
            else if constexpr (std::is_same_v<T, int8_t> ||
                               std::is_same_v<T, char>) {
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(static_cast<unsigned char>(value)) << " ";
            }
            else if constexpr (std::is_integral_v<T> && sizeof(T) <= 8) {
                ss << "0x" << std::hex << std::setw(sizeof(T) * 2) << std::setfill('0')
                << static_cast<uint64_t>(value) << " ";
            }
            else {
                // Для нечисловых типов просто выводим как есть
                ss << value << " ";
            }
        };

        (push(args), ...);

        logOut(ss.str());
    }

    // Логирование UART пакета (адаптировано из вашего кода)
    template<typename UartPacketType>
    void logOutUart(const UartPacketType& pack, const std::string& packet_name = "UART") {
        logOut("┌─────────────────────────────────────────────");
        logOut("│ Полученный пакет");
        logOut("├─────────────────────────────────────────────");
        logOut("│ Статус: " + std::to_string(pack.status_));
        logOut("│ Команда: " + std::to_string(pack.nameCommand_));
        logOut("│ CRC: " + std::to_string(pack.crc_));
        logOut("│ Параметры:");

        for(size_t i = 0; i < pack.parameters_.size(); ++i) {
            logOut("│   [" + std::to_string(i) + "] = 0x" +
                   toHexString(pack.parameters_[i]));
        }
        logOut("└─────────────────────────────────────────────");
    }

    // Логирование вектора байт в HEX формате
    void logHexDump(const uint8_t* data, size_t len, const std::string& label = "") {
        if (m_log_level < LogLevel::DEBUG_LEVEL) {
            return;
        }
        if (!label.empty()) {
            logOut(" " + label + " (" + std::to_string(len) + " байт)");
        }

        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < len; ++i) {
            if (i > 0 && i % 16 == 0) {
                logOut("  " + ss.str());
                ss.str("");
                ss.clear();
                ss << std::hex << std::setfill('0');
            }
            ss << std::setw(2) << static_cast<int>(data[i]) << " ";
        }

        if (!ss.str().empty()) {
            logOut("  " + ss.str());
        }
    }

    // Установка уровня логирования
    void set_log_level(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_log_level = level;
    }

    // Включение/отключение вывода в консоль
    void set_console_output(bool enabled) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_console_output = enabled;
    }

    // Включение/отключение логирования функций
    void set_function_logging(bool enabled) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_enable_function_logging = enabled;
    }

    // Получение текущего времени
    std::string currentDateTime() const {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
        return std::string(buf);
    }

private:
    Logger() = default;
    ~Logger() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }

    // Запрет копирования и перемещения
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // Вспомогательная функция для HEX преобразования
    std::string toHexString(uint8_t value) const {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(value);
        return ss.str();
    }

    std::ofstream m_file;
    LogLevel m_log_level = LogLevel::INFO_LEVEL;
    bool m_console_output = true;
    bool m_enable_function_logging = true;
    std::mutex m_mutex;

    // ======== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ========

    std::string get_current_time() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::string level_to_string(LogLevel level) const {
        switch (level) {
        case LogLevel::DEBUG_LEVEL:    return "DEBUG";
        case LogLevel::INFO_LEVEL:     return "INFO";
        case LogLevel::WARNING_LEVEL:  return "WARN";
        case LogLevel::ERROR_LEVEL:    return "ERROR";
        case LogLevel::CRITICAL_LEVEL: return "CRITICAL";
        default: return "UNKNOWN";
        }
    }

    template<typename... Args>
    void log(LogLevel level, const Args&... args) {
        if (level < m_log_level) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        std::stringstream message;
        message << get_current_time()
                << " [" << level_to_string(level) << "] "; //<< "[T" << std::this_thread::get_id() << "] "

        build_message(message, args...);

        if (m_file.is_open()) {
            m_file << message.str() << std::endl;
            m_file.flush();
        }

        if (m_console_output) {
            if (level >= LogLevel::ERROR_LEVEL) {
                std::cerr << message.str() << std::endl;
            } else {
                std::cout << message.str() << std::endl;
            }
        }
    }

    template<typename T, typename... Args>
    void build_message(std::stringstream& ss, const T& first, const Args&... rest) {
        ss << first;
        build_message(ss, rest...);
    }

    void build_message(std::stringstream&) {
        // Конец рекурсии
    }
};

// ======== МАКРОСЫ ДЛЯ УДОБСТВА ========

#define LOG_DEBUG(...)   Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...)    Logger::instance().info(__VA_ARGS__)
#define LOG_WARNING(...) Logger::instance().warning(__VA_ARGS__)
#define LOG_ERROR(...)   Logger::instance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance().critical(__VA_ARGS__)

// Специальные макросы для отладки функций
#define LOG_FUNCTION_CALL(...) Logger::instance().logFunctionCall(__FUNCTION__, ##__VA_ARGS__)

// Макрос для HEX вывода
#define LOG_HEX(...) Logger::instance().logParametersHex(__VA_ARGS__)

// Макрос для UART пакетов
#define LOG_UART(pack) Logger::instance().logOutUart(pack, #pack)