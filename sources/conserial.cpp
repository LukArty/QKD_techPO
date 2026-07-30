/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
/// @version 1.5
/// @copyright Copyright 2022 InfoTeCS.


#include <conserial.h>


#define SERIAL_LOG

namespace hwe
{
/// @brief Интерфейс для взаимодействия с аппаратной платформой.
Conserial::Conserial(string port)
{
    com_ = std::make_unique<ce::ceSerial>();
#ifdef SERIAL_LOG
    Logger::instance().initialize("ceserial.log", LogLevel::INFO_LEVEL, true, true);
#else
    Logger::instance().initialize("ceserial.log", LogLevel::OFF_LEVEL, true, true);
#endif

#ifdef CE_WINDOWS
    if (port.empty())
        com_->SetPort("COM3");
    else
        com_->SetPort(port);
#else
    if (port.empty())
        com_.SetPort("/dev/ttyStandQKD");
    else
        com_.SetPort(port);
#endif
    com_->SetBaudRate(115200);
    com_->SetDataSize(8);
    com_->SetParity('N');
    com_->SetStopBits(1);
    com_->Open();

    standOptions.premissions = 0;
    standOptions.laserState_ = 0;
    standOptions.laserPower_ = 0;
    standOptions.signalLevels_ = {0,0};
    standOptions.curAngles_ = {0,0,0,0};
    standOptions.lightNoises = {0,0};
    standOptions.startLightNoises_= {0,0};
    standOptions.startPlatesAngles_ = {0,0,0,0};
    standOptions.maxSignalLevels_ = {0,0};
    standOptions.timeoutTime_ = 2000;
    standOptions.rotateStep_ = 0.3;
    standOptions.maxLaserPower_ = 100;
    standOptions.maxPayloadSize = 30;
    FindProtocolVersion();
}

std::string Conserial::GetComPortName()const
{
    return com_->GetPort();
}

void Conserial::SetComPortName(const std::string& port)
{
    LOG_FUNCTION_CALL(port);

    // Закрываем текущее соединение
    if (com_->IsOpened()) {
        com_->Close();
        LOG_DEBUG("Текущий порт закрыт");
    }

    // Устанавливаем новый порт
    if (!port.empty()) {
        #ifdef CE_WINDOWS
            com_->SetPort("\\\\.\\" + port);
        #else
            com_->SetPort(port);
        #endif
        LOG_DEBUG("Установлен порт: ", port);
    } else {
        // Если порт не указан - используем текущий
        LOG_DEBUG("Порт не указан, используем: ", com_->GetPort());
    }

    // Настраиваем параметры
    com_->SetBaudRate(115200);
    com_->SetDataSize(8);
    com_->SetParity('N');
    com_->SetStopBits(1);

    // Пытаемся открыть порт
    com_->Open();
    if (!com_->IsOpened()) {
        LOG_ERROR("Не удалось открыть порт: ", com_->GetPort());
    }else{
        LOG_INFO("Порт успешно открыт: ", com_->GetPort());
        FindProtocolVersion();
    }

}

Conserial::~Conserial(){
    if (com_ && com_->IsOpened()) {
        com_->Close();
        LOG_DEBUG("COM-порт закрыт");
    }
}

api:: InitResponse Conserial:: Init()
{
    LOG_FUNCTION_CALL();
    api::InitResponse response; // Структура для формирования ответа
    std::fstream ini_("./Angles.ini");
    if (!ini_.is_open()) { response = InitByPD();  }
    else {
        const int n =6;
        char temp_ [n];

        ini_.getline(temp_, n);//считываем первую строку где лежит угл для 1 пластины. Вид : 003.4
        temp_[3]='.';//Меняем разделитель на точку (На случай если запятая)
        string angle1_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle2_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle3_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle4_ = temp_;

        if (angle1_.length() > 4 && angle2_.length() > 4 && angle3_.length() > 4 && angle4_.length() > 4){ //Если в файл записаны углы
            WAngles <angle_t> anglesIni_{};
            anglesIni_.aHalf_= stof (angle1_) ;
            anglesIni_.aQuart_= stof (angle2_);
            anglesIni_.bHalf_= stof (angle3_);
            anglesIni_.bQuart_= stof (angle4_);
            response =  InitByButtons(anglesIni_);} //Инициализация по концевикам
        else {response = InitByPD();}//Инициализация по фотодиодам
    }
    return response;
}

api::InitResponse Conserial::InitByPD()
{
    LOG_FUNCTION_CALL();
    api::InitResponse response = {}; // Структура для формирования ответа
    uint32_t tempData = standOptions.timeoutTime_; //для FW 1.0
    standOptions.timeoutTime_ = INIT_TIMEOUT_TIME;

    response = SendCommand<api::InitResponse>("Init");

    standOptions.timeoutTime_ = tempData;

    if(response.errorCode_ == 0) {
        standOptions.startPlatesAngles_ = response.startPlatesAngles_;
        standOptions.curAngles_ = response.startPlatesAngles_;
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
    }

    if(v_protocol>ProtocolVersion::V1_2){ //!!!
        standOptions.timeoutTime_ = GetTimeout().adcResponse_;
    }
    return response; // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    LOG_FUNCTION_CALL(angles.aHalf_,angles.aQuart_,angles.bHalf_,angles.bQuart_ );


    WAngles<adc_t> steps = CalcSteps(angles);

    auto response = SendCommand<api::InitResponse>(        "InitByButtons",
                                          steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    if(response.errorCode_ == 0) {
        standOptions.startPlatesAngles_ = response.startPlatesAngles_;
        standOptions.curAngles_ = response.startPlatesAngles_;
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
    }
    return response;
}

api::AdcResponse Conserial::RunTest()
{

    uint32_t oldTimeout = standOptions.timeoutTime_;
    standOptions.timeoutTime_ = 10000;

    api::AdcResponse response = SendCommand<api::AdcResponse>("RunSelfTest");

    standOptions.timeoutTime_ = oldTimeout;

    return response;
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
    LOG_FUNCTION_CALL(angles.aHalf_,angles.aQuart_,angles.bHalf_,angles.bQuart_, power );

    WAngles<adc_t> steps = CalcSteps(angles);

    auto response = SendCommand<api::SendMessageResponse>(        "SendMessage",
                                                 steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_, power);

    if(response.errorCode_ == 0) {
        standOptions.curAngles_ = response.newPlatesAngles_;
        standOptions.lightNoises = response.currentLightNoises_;
        standOptions.signalLevels_ = response.currentSignalLevels_;
    }
    return response;
}

api::AdcResponse Conserial::SetTimeout(uint32_t timeout_ms)
{
    LOG_FUNCTION_CALL(timeout_ms, "мс" );
    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        if(timeout_ms >= 1000 && timeout_ms <65000) {
            response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput);
            return response;
        }

        uint16_t time_s = timeout_ms;

        response = SendCommand<api::AdcResponse>("SetTimeout", time_s);

        if(response.errorCode_ == 0) {
            standOptions.timeoutTime_ = response.adcResponse_;
        }
    }
    return response;
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
    LOG_FUNCTION_CALL(on);

    api::AdcResponse response = {};
    if(on != 0 && on != 1) {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput);
        return response;
    }

    response = SendCommand<api::AdcResponse>("SetLaserState", on);

    if(response.errorCode_ == 0) {
        standOptions.laserState_ = response.adcResponse_;
    }

    return response;
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    LOG_FUNCTION_CALL(power);
    api::AdcResponse response = {};
    if(power > standOptions.maxLaserPower_) {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput);
        return response;
    }

    response = SendCommand<api::AdcResponse>("SetLaserPower", power);

    if(response.errorCode_ == 0) {
        standOptions.laserPower_ = response.adcResponse_;
    }

    return response;
}

api::AdcResponse Conserial::DAC_SetLaserPower(adc_t power)
{
    LOG_FUNCTION_CALL(power);
    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        if(  power > 4095) {
            response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput);
            return response;
        }

        response = SendCommand<api::AdcResponse>("DAC_SetLaserPower", power);
    }
    return response;
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
    LOG_FUNCTION_CALL(angles.aHalf_,angles.aQuart_,angles.bHalf_,angles.bQuart_ );


    WAngles<adc_t> steps = CalcSteps(angles);
    auto response = SendCommand<api::WAnglesResponse>(        "SetPlatesAngles",
                                             steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);
    if(response.errorCode_ == 0) {
        standOptions.curAngles_ = response.angles_;
    }

    return response;
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
    LOG_FUNCTION_CALL(angles.aHalf_,angles.aQuart_,angles.bHalf_,angles.bQuart_ );

    api::WAnglesResponse response = {};
    if (CheckStandPremmissions<api::WAnglesResponse>(response)){

        WAngles<adc_t> steps = CalcSteps(angles);
        response = SendCommand<api::WAnglesResponse>(        "UpdateBaseAngles",
                                                     steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);
    }
    if(response.errorCode_ == 0) {
        standOptions.startPlatesAngles_ = response.angles_;
    }
    return response;
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
    LOG_FUNCTION_CALL();
    api::WAnglesResponse response = {};

    response = SendCommand<api::WAnglesResponse>("ReadBaseAngles");

    if(response.errorCode_ == 0) {
        standOptions.startPlatesAngles_ = response.angles_;
    }
    return response;
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
    LOG_FUNCTION_CALL();
    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        response = SendCommand<api::AdcResponse>("ReadEEPROM", numberUnit_);
    }
    return response;
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
    LOG_FUNCTION_CALL();

    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        response = SendCommand<api::AdcResponse>("WriteEEPROM", numberUnit_, param_);
    }

    return response;
}

api::AdcResponse Conserial::GetLaserState()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::AdcResponse>("GetLaserState");
}

api::AdcResponse Conserial::GetLaserPower()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::AdcResponse>("GetLaserPower");
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::WAnglesResponse>("GetCurPlatesAngles");
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::SLevelsResponse>("GetSignalLevel");
}

api::AngleResponse Conserial::GetRotateStep()
{
    LOG_FUNCTION_CALL();
    auto response = SendCommand<api::AngleResponse>("GetRotateStep");
    if (response.errorCode_==0 )
        standOptions.rotateStep_ = response.angle_;
    return response;

}

api::SLevelsResponse Conserial::GetLightNoises()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::SLevelsResponse>("GetLightNoises");
}

api::AdcResponse Conserial::GetHardwareState(){
    LOG_FUNCTION_CALL();
    api::AdcResponse response = SendCommand<api::AdcResponse>("GetHardwareState");

    if (response.adcResponse_ & 0){
        LOG_INFO("Аппаратная платформа в рабочем состоянии");
    }else
    {
        if ( response.adcResponse_ & (1<<1) )
            LOG_CRITICAL("Не работает фотодетектор PDH");
        if ( response.adcResponse_ & (1<<2) )
            LOG_CRITICAL("Не работает фотодетектор PDV");
        if (response.adcResponse_ & (1<<3) )
            LOG_CRITICAL("Не работает лазер");
        if ( response.adcResponse_ & (1<<4) )
            LOG_CRITICAL("Не работает первый двигатель");
        if ( response.adcResponse_ & (1<<5) )
            LOG_CRITICAL("Не работает второй двигатель");
        if ( response.adcResponse_ & (1<<6) )
            LOG_CRITICAL("Не работает третий двигатель");
        if (response.adcResponse_ & (1<<7))
            LOG_CRITICAL("Не работает четвертый двигатель");
    }
    return response;
}

api::AdcResponse Conserial::GetErrorCode()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::AdcResponse>("GetLaserState");
}

api::AdcResponse Conserial::GetTimeout()
{
    LOG_FUNCTION_CALL();
    auto response = SendCommand<api::AdcResponse>("GetTimeout");
    if (response.errorCode_==0){
        if (response.adcResponse_ != 0) {
            standOptions.timeoutTime_ = response.adcResponse_;
        }
        else{
            return {response.adcResponse_, static_cast<uint16_t>(ErrorCode::FlashDataEmpty)};
        }
    }
    return response;
}

api::InitResponse Conserial::GetInitParams(){

    LOG_FUNCTION_CALL();
    return SendCommand<api::InitResponse>("GetInitParams"); // Возвращаем сформированный ответ

}

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    LOG_FUNCTION_CALL();
    return {standOptions.startLightNoises_, static_cast<uint16_t>(ErrorCode::Success)};
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    LOG_FUNCTION_CALL(plateNumber, angle);
    // logOut("Параметры: "+to_string(plateNumber)+ " " + to_string(angle));

    api::AngleResponse response; // Структура для формирования ответа
    api::WAnglesResponse tempResponse;
    WAngles <angle_t> angles = standOptions.curAngles_;

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // // Принят некорректный входной параметр
        return response;
    }

    switch (plateNumber)
    {
    case 1:
        angles.aHalf_ = angle;
        tempResponse = SetPlatesAngles(angles);

        response.angle_ = tempResponse.angles_.aHalf_;
        break;
    case 2:
        angles.aQuart_ = angle;
        tempResponse = SetPlatesAngles(angles);

        response.angle_ = tempResponse.angles_.aQuart_;
        break;
    case 3:
        angles.bHalf_ = angle;
        tempResponse = SetPlatesAngles(angles);

        response.angle_ = tempResponse.angles_.bHalf_;
        break;
    case 4:
        angles.bQuart_ = angle;
        tempResponse = SetPlatesAngles(angles);

        response.angle_ = tempResponse.angles_.bQuart_;
        break;
    }

    response.errorCode_ = tempResponse.errorCode_;
    standOptions.curAngles_ = tempResponse.angles_;
    return response;
}

api::AdcResponse Conserial::GetMaxLaserPower()
{
    LOG_FUNCTION_CALL();
    return {standOptions.maxLaserPower_, static_cast<uint16_t>(ErrorCode::Success)};
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{
    LOG_FUNCTION_CALL();
    return {standOptions.startPlatesAngles_, static_cast<uint16_t>(ErrorCode::Success)};
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    LOG_FUNCTION_CALL();
    return {standOptions.maxSignalLevels_, static_cast<uint16_t>(ErrorCode::Success)};
}

std::vector<uint8_t> Conserial::PreparePasswordBytes(const string& passwd)
{
    size_t original_len = passwd.size();
    size_t padded_len = original_len;
    if (original_len % 2 != 0) {
        padded_len += 1;
    }

    std::vector<uint8_t> bytes(padded_len, 0);
    std::memcpy(bytes.data(), passwd.data(), original_len);
    return bytes;
}

api::AdcResponse Conserial::CreateConfigSecret(string passwd){

    LOG_FUNCTION_CALL();

    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        std::vector<uint8_t> passwordBytes=PreparePasswordBytes(passwd);

        response = SendCommand<api::AdcResponse>("CreateConfigSecret", passwordBytes);
    }

    return response;
}



api::AdcResponse Conserial::OpenConfigMode(string passwd)
{
    LOG_FUNCTION_CALL();
    api::AdcResponse response = {0,0};
    if (versionFirmware.major <= 1 && versionFirmware.micro<=5){
        if (passwd == "admin"){
            response.adcResponse_ = 1;
            response.errorCode_   = static_cast<uint16_t>(ErrorCode::Success);
        }
        else{
            response.adcResponse_ = 0;
            response.errorCode_   = static_cast<uint16_t>(ErrorCode::AccessDenied);
        }

        standOptions.premissions = response.adcResponse_;
        return response;
    }

    std::vector<uint8_t> passwordBytes=PreparePasswordBytes(passwd);
    return SendCommand<api::AdcResponse>("OpenConfigMode", passwordBytes);
}


uint16_t Conserial::CloseConfigMode()
{
    LOG_FUNCTION_CALL();
    standOptions.premissions  = 0;
    return 1;
}

uint16_t Conserial::GetCurrentMode()
{
    LOG_FUNCTION_CALL();
    return standOptions.premissions;
}

uint16_t Conserial::GetMaxPayloadSize()
{
    LOG_FUNCTION_CALL();
    return SendCommand<api::AdcResponse>("GetMaxPayloadSize").adcResponse_;
}

api::versionProtocolResponse Conserial::GetProtocolVersion (){
    LOG_FUNCTION_CALL();

    switch (v_protocol) {
    case ProtocolVersion::Unknown:
        return {0,0, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    case ProtocolVersion::V1_2:
        return  {1,2, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    case ProtocolVersion::V1_0:
        return  {1,0, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    default:
        return SendCommand<api::versionProtocolResponse>("GetProtocolVersion");
        break;
    }
}

api::versionFirmwareResponse Conserial::GetCurrentFirmwareVersion(){
    LOG_FUNCTION_CALL();
    switch (v_protocol) {
    case ProtocolVersion::Unknown:
        return {0,0,0, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    case ProtocolVersion::V1_2:
        return  {1,0,0, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    case ProtocolVersion::V1_0:
        return  {1,0,0, static_cast<uint16_t>(ErrorCode::Success)};
        break;
    default:
        return SendCommand<api::versionFirmwareResponse>("GetCurrentFirmwareVersion");
        break;
    }
}

void Conserial::SetBoardType(BoardType type){
    standOptions.boardType = type;
}

api::AdcResponse Conserial::FirmwareUpdate(std::string path){
    LOG_FUNCTION_CALL();

    api::AdcResponse response = {};
    switch (standOptions.boardType) {
    case BoardType::Arduino:
        response = FirmwareUpdate_Arduino(path);
        break;
    case BoardType::STM:
        response = FirmwareUpdate_STM(path);
        break;
    default:
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::AccessDenied);
        break;
    }
    return response;
}


api::AdcResponse Conserial::FirmwareUpdate_STM(std::string path)
{
    LOG_FUNCTION_CALL();

    api::AdcResponse response = {};
    if (CheckStandPremmissions<api::AdcResponse>(response)){
        com_->Close();

        /* ==== Открываем BIN ==== */
        FILE* bin = nullptr;

#ifdef _WIN32
        std::wstring wpath(path.begin(), path.end());
        bin = _wfopen(wpath.c_str(), L"rb");
#else
        bin = fopen(path.c_str(), "rb");
#endif

        if (!bin) {
            LOG_ERROR("Не удалось открыть файл прошивки: " + path);
            return {0, static_cast<uint16_t>(ErrorCode::FirmwareFileNotRead)};
        }

        /* ==== Открываем FTDI ==== */
        FT_HANDLE ft;

        // как в main (2).c — первый FTDI в системе
        if (FT_Open(0, &ft) != FT_OK) {
            LOG_ERROR("Не удалось открыть FTDI устройство");
            fclose(bin);
            return {0, static_cast<uint16_t>(ErrorCode::NoConnection)};
        }

        /* ==== Настройка порта ==== */
        FT_SetBaudRate(ft, 115200);
        FT_SetDataCharacteristics(ft, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
        FT_SetTimeouts(ft, 1000, 1000);

        LOG_ERROR("FTDI сконфигурирован: 115200, 8N1");

        /* ================= ВХОД В BOOTLOADER ================= */

        LOG_INFO("Вход в режим загрузчика...");

        FT_SetDtr(ft);
        sleep_ms(50);
        FT_ClrRts(ft);
        sleep_ms(50);
        FT_SetRts(ft);
        sleep_ms(50);

        /* ==== Инициализация STM32 ROM Bootloader ==== */
        uint8_t init = 0x7F;
        uart_write(ft, &init, 1);
        if (!wait_ack(ft, 1000)) {
            LOG_CRITICAL("Загрузчик не отвечает");
            FT_Close(ft);
            fclose(bin);
            return {0, static_cast<uint16_t>(ErrorCode::BootloaderNoResponse)};
        }

        LOG_INFO("Загрузчик активен");

        /* ================= СТИРАНИЕ FLASH ================= */

        LOG_INFO("Стирание FLASH...");
        if (!bl_mass_erase(ft)) {
            LOG_CRITICAL("Ошибка стирания FLASH");
            FT_Close(ft);
            fclose(bin);
            return {0, static_cast<uint16_t>(ErrorCode::FlashEraseFailed)};
        }

        /* ================= ЗАПИСЬ ПРОШИВКИ ================= */

        LOG_INFO("Запись прошивки...");

        uint8_t buf[256];
        uint32_t addr = 0x08000000;
        size_t r;
        size_t total = 0;
        std::stringstream ss;

        while ((r = fread(buf, 1, sizeof(buf), bin)) > 0) {
            if (!bl_write(ft, addr, buf, (int)r)) {
                ss << std::hex << addr;
                LOG_CRITICAL("Ошибка записи @0x" + ss.str());
                FT_Close(ft);
                fclose(bin);
                return {0, static_cast<uint16_t>(ErrorCode::FlashWriteFailed)};
            }

            addr += r;
            total += r;
        }

        LOG_INFO("Записано " + std::to_string(total) + " байт");

        /* ================= СБРОС ================= */

        LOG_INFO("Сброс устройства...");

        FT_ClrDtr(ft);
        sleep_ms(50);
        FT_ClrRts(ft);
        sleep_ms(50);
        FT_SetRts(ft);
        sleep_ms(50);

        FT_Close(ft);
        fclose(bin);

        LOG_INFO("Прошивка успешно завершена");
        com_->Open();
    }
    return {1, static_cast<uint16_t>(ErrorCode::Success)};
}


api::AdcResponse Conserial::FirmwareUpdate_Arduino(string path){
    LOG_FUNCTION_CALL();
    api::AdcResponse response{};

    if (!CheckStandPremmissions<api::AdcResponse>(response))
        return response;

    const std::string port = com_->GetPort();

    auto flash = [&](int baudRate) -> ErrorCode
    {
        com_->Close();
        std::string command =
            "avrdude -v "
            "-p atmega328p "
            "-c arduino "
            "-P " + port +
            " -b " + std::to_string(baudRate) +
            " -D "
            "-U flash:w:\"" + path + "\":i";

        LOG_DEBUG("Executing: " + command);

        int result = system(command.c_str());

        if (result == 0)
        {
            LOG_INFO("Успешная прошивка стенда (" + std::to_string(baudRate) + " baud).");
            return ErrorCode::Success;
        }

        LOG_CRITICAL("Upload failed (" + std::to_string(baudRate) +
               " baud), exit code = " + std::to_string(result));

        return ErrorCode::BootloaderNoResponse;
    };

    ErrorCode success = ErrorCode::BootloaderNoResponse;

    // Сначала пробуем новый загрузчик
    success = flash(115200);

    // Если не получилось — пробуем старый
    if (success != ErrorCode::Success)
    {
        LOG_DEBUG("Попытка прошивки через старый загрузчик...");
        success = flash(57600);
    }

    switch (success) {
    case ErrorCode::Success:
        FindProtocolVersion();
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::Success);
        break;
    default:
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::BootloaderNoResponse);
        LOG_ERROR("Невозможно прошить стенд. Загрузчик не отвечает");
        break;
    }
    return response;
}





void Conserial::FindProtocolVersion(){
    Logger::instance().logOut("\t  *********************************");
    LOG_FUNCTION_CALL();
    int notFound = 1;
    v_protocol = ProtocolVersion::V1_5;

    while(notFound !=0 && !(v_protocol == ProtocolVersion::Unknown)){
        notFound = GetLaserState().errorCode_;
        if(notFound !=0){
            switch (v_protocol) {
            case ProtocolVersion::V1_5:
                v_protocol = ProtocolVersion::V1_2;
                break;
            case ProtocolVersion::V1_2:
                v_protocol = ProtocolVersion::V1_0;
                break;
            default:
                v_protocol = ProtocolVersion::Unknown;
                break;
            }
        }
    }
    LOG_INFO("Версия протокола:" + v_protocol.getVersion().toString());
    Logger::instance().logOut("\t  ********************************* \n");
}

//          ****** ТРАНСПРОТ ******

Conserial::UartResponse Conserial::Twiting (uint8_t commandName,  uint8_t * bytes, uint16_t length){

    UartResponse pack;
    pack.status_ = static_cast<uint16_t>(ErrorCode::Success);

    // Проверка соединения
    if (!com_->IsOpened())
    {
        com_-> Open();
        if (!com_->IsOpened())
        {
        pack.status_= static_cast<uint16_t>(ErrorCode::NoConnection);
        LOG_ERROR("Порт не открыт");
        return pack;
        }
    }
    // Попытки отправки
    const int MAX_ATTEMPTS = 3;
    for(int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
    {
        LOG_INFO("Попытка отправки #" + std::to_string(attempt + 1));

        // Отправка пакета
        if(!SendPacket(commandName, bytes, length)){
            LOG_ERROR("Ошибка: Не удалось отправить пакет");
            pack.status_ = static_cast<uint16_t>(ErrorCode::InternalError);
            continue;
        }

        // Чтение ответа
        try {
            pack = ParsePacket();
        } catch (...) {
            LOG_DEBUG("Исключение при парсинге !");
            pack.status_ = static_cast<uint16_t>(ErrorCode::InternalError);
            continue;
        }

        if (pack.status_ == 1)
            break;
    }

    return pack;
}


uint16_t Conserial:: SendPacket (uint8_t commandName,  uint8_t * bytes, uint16_t N){

    uint8_t crc = 0;

    std::vector<uint8_t> packData;
    std::vector<uint8_t> packet;

    /*Упаковка пакета*/
    switch (v_protocol) {
    case ProtocolVersion::V1_5:{
        /*
         * st0 | st1 | pld0 | pld1 | cmd | bytes | solt |  crc | end0 | end1
         * 255 | 254 | 0 | x | 67 | x | 255 | 255
        */
        packData.reserve(N+4); // pld0 + pld1 + cN + [bytes] + solt
        packData.push_back((N+2) >> 8); //pld0
        packData.push_back((N+2) & 0xFF); //pld1
        packData.push_back(commandName); //cN
        if (bytes && N > 0)
        {
            packData.insert(packData.end(), bytes, bytes + N);
        }
        packData.push_back(0x00); //solt

        crc = Crc8(packData.data(), packData.size());


        packet.reserve(packData.size() + 5);

        packet.push_back(0xFF);
        packet.push_back(0xFE);

        packet.insert(packet.end(), packData.begin(), packData.end());

        packet.push_back(crc);  //crc
        packet.push_back(0xFF);
        packet.push_back(0xFF);
        break;
    }
    case ProtocolVersion::V1_2:{
        /*
         * st0 | st1 | cmd | bytes | solt |  crc | end0 | end1
         * 255 | 254 | 67 | 0 | x | 255 | 255
        */
        packData.reserve(N+2); // cN + [bytes] + solt

        packData.push_back(commandName); //cN

        if (bytes && N > 0)
        {
            packData.insert(packData.end(), bytes, bytes + N);
        }
        packData.push_back(0x00); //solt

        crc = Crc8(packData.data(), packData.size());

        packet.push_back(0xFF); //st0
        packet.push_back(0xFE); //st1

        packet.insert(packet.end(), packData.begin(), packData.end()); // cN + [bytes] + solt

        packet.push_back(crc);  //crc
        packet.push_back(0xFF); //end0
        packet.push_back(0xFF); //end1
        break;
    }
    case ProtocolVersion::V1_0:{
        /*
         * st0 | st1 | status | cmd | bytes | crc | end0 | end1
         * 255 | 254 | 0 | 67 | 37 | 255 | 255
        */
        packData.reserve(N+2); //  status + cN + [bytes]


        packData.push_back(commandName); //cN


        if (bytes && N > 0)
        {
            packData.insert(packData.end(), bytes, bytes + N);
        }
        packData.push_back(0x00); //status


        crc = Crc8(packData.data(), packData.size());


        for (auto b : packData)
        {
            Logger::instance().logOutLine( to_string(b) + " | " );
        }
        Logger::instance().logOut("");
        packet.push_back(0xFF); //st0
        packet.push_back(0xFE); //st1
        packet.push_back(0x00); //status
        packet.push_back(commandName); //cN
        packet.push_back(crc);
        if(packData.size()>2){
            packet.insert(packet.end(), packData.begin()+1, packData.end()-1); // bytes
        }
        packet.push_back(0xFF); //end0
        packet.push_back(0xFF); //end1
        break;
    }
    default:
        return 0;
    }

    Logger::instance().logOut( Logger::instance().currentDateTime() + " -- Отправка пакета");

    for (auto b : packet)
    {
        com_->Write(b);
        LOG_HEX(b);
    }
    Logger::instance().logOut("");

    return 1;
}

std::vector<uint8_t> Conserial::ReadPacket(){

    bool success = 0, end_read = 0, start_read = 0;
    clock_t dedline = clock() + standOptions.timeoutTime_ * CLOCKS_PER_SEC / 1000 ; //! МС
    uint8_t byte = 0;
    uint16_t sliding = 0;
    std::vector<uint8_t> readBytes;

    switch (v_protocol) {
    case ProtocolVersion::Unknown:
        break;
    default:
        Logger::instance().logOut("\n"+ Logger::instance().currentDateTime() + " -- Поиск начала пакета" );
        while (clock()< dedline && !end_read){
            byte = com_->ReadChar(success);
            LOG_HEX(byte);
            if (!success)
                continue;

            sliding = (sliding << 8) |  byte;

            if (!start_read) // Поиск начала пакета
            {
                if (sliding == 0xFFFE)
                {
                    start_read = true;
                    LOG_INFO("\n======== Успешно найдено начало пакета ======== ");
                    sliding = 0;
                    readBytes.clear();
                }
                continue;
            }
            //======== Считывание ========
            readBytes.push_back(byte);

            if (sliding == 0xFFFF)
            {
                // Вечный костыль на случай CRC = FF
                byte = com_->ReadChar(success);
                LOG_HEX(byte);
                if (byte == 0xFF && success){
                    readBytes.push_back(byte);
                }
                end_read = true;
                if (readBytes.size() >= 2){
                    readBytes.pop_back(); // удалить предыдущий 0xFF
                    readBytes.pop_back();
                }
                // cout << ""<<endl;
            }

            if (clock()>dedline){ //timeouted
                LOG_WARNING("\n"+ Logger::instance().currentDateTime() + " -- Вышел таймаут" );
                end_read = true;
                readBytes.resize(0);
            }
        }
        break;
    }
    // for (auto var : readBytes) {
    //     cout << int (var) << " ";
    // }
    // logOut("");
    return readBytes;
}

Conserial::UartResponse Conserial::ParsePacket(){
    Conserial::UartResponse pack_;

    vector <uint8_t> readedBytes = ReadPacket() ; // Reading

    /*Парсинг*/
    uint8_t crc = 255;

    if (!readedBytes.empty() && readedBytes.size()>2){ // Считаны байты с UART

        switch (v_protocol) {
        case ProtocolVersion::V1_5:{
            pack_.payload = (uint16_t) readedBytes.at(0) << 8 | (uint16_t) readedBytes.at(1);
            pack_.nameCommand_ = readedBytes.at(2);
            pack_.status_ = readedBytes.at(3);
            pack_.crc_ = readedBytes.back();
            LOG_DEBUG(to_string((int)readedBytes.size()));
            for (int j = 4; j < (int)readedBytes.size()-3 ; j+=2) {
                pack_.parameters_.push_back((uint16_t) readedBytes.at(j) << 8 | (uint16_t) readedBytes.at(j+1));

            }

            crc = Crc8(readedBytes.data(), readedBytes.size());
            if(crc!=0){
                LOG_ERROR("\n"+ Logger::instance().currentDateTime() + "-- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                LOG_UART(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        case ProtocolVersion::V1_2:{
            pack_.nameCommand_ = readedBytes.at(0);
            pack_.status_ = readedBytes.at(1);
            pack_.crc_ = readedBytes.back();


            for (int j = 2; j < (int)readedBytes.size(); j+=2) {
                pack_.parameters_.push_back((uint16_t) readedBytes.at(j) << 8 | (uint16_t) readedBytes.at(j+1));

            }

            crc = Crc8(readedBytes.data(), readedBytes.size());
            if(crc!=0){
                LOG_ERROR("\n" + Logger::instance().currentDateTime() + " -- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                LOG_UART(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        case ProtocolVersion::V1_0:{
            pack_.status_ = readedBytes.at(0);
            pack_.nameCommand_ = readedBytes.at(1);
            pack_.crc_ = readedBytes.at(2);

            for (int j = 3; j < (int)readedBytes.size(); j+=2) {
                pack_.parameters_.push_back((uint16_t) readedBytes.at(j) << 8 | (uint16_t) readedBytes.at(j+1));
            }


            uint16_t temp = pack_.nameCommand_;
            for (int i = 0; i < (int)pack_.parameters_.size(); ++i) {
                temp+= pack_.parameters_.at(i);
            }

            crc = Crc8((uint8_t *)&temp, sizeof(temp));

            if(!(crc == pack_.crc_)){
                LOG_ERROR("\n" + Logger::instance().currentDateTime() + "-- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                LOG_UART(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        default:
            break;
        }

    }else{
        LOG_ERROR("\n" + Logger::instance().currentDateTime() + "-- Ошибка: Принятый пакет был сломан");
        pack_.status_ = static_cast<uint16_t>(ErrorCode::MissingFrameEnd);
    }

    Logger::instance().logOut("\n ======== Конец пакета ========");
    LOG_UART(pack_);

    return pack_;
}

bool Conserial::StandIsConected (){
    if(!com_->IsOpened())
    {
        com_->Close();
        com_->Open();
        if(!com_->IsOpened())
            return 0;
        else{
            FindProtocolVersion();
        }
    }
    return 1;
};

uint16_t Conserial::CheckStatus(uint16_t status){
    uint16_t errorCode = status;

    if (status == 1){
        errorCode = static_cast<uint16_t>(ErrorCode::Success);
        LOG_INFO("Успешное выполнение запроса");
    }

    else if(status == 0){
        errorCode = static_cast<uint16_t>(ErrorCode::NoConnection);
        LOG_ERROR("Отсутствует соединение со стендом");
    }else if(status == 2){
        LOG_ERROR("Количество принятых параметров превышает допустимый предел");
    }else if(status == 3){
        LOG_ERROR("Необнаружена метка конца пакета");
    }else if(status == 4){
        LOG_ERROR("Не удалось выполнить команду / Не известный ID команды ");
    }else if(status == 5){
        LOG_ERROR("Отказано в доступе (недостаточно прав)");
    }else if(status == 6){
        LOG_ERROR("Переданы неверные параметры на вход библиотечной функции");
    }else if(status == 7){
        LOG_CRITICAL("Аппаратная платформа в аварийном состоянии");
    }else if(status == 8){
        LOG_ERROR("Несоответствие CRC");
    }
    LOG_INFO("-> Код ошибки: " + to_string(errorCode));
    return errorCode;
}

// Функция подсчёта контрольной суммы
uint8_t Conserial::Crc8(uint8_t *pcBlock, uint8_t len)
{
    uint8_t crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];
    return crc;
}

// std::vector<uint8_t> Conserial::PackToBytes(const std::vector<uint8_t>& vec) {
//     return vec;
// }

//          ========  Step to Angle  & Angle to Step ========
uint16_t Conserial::CalcStep(angle_t angle, angle_t rotateStep){


    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота
    if (angle < 0){
        angle = angle + 360;
    }
    int Step = round (angle / rotateStep); //Подсчёт и округление шагов
    return Step;
}

WAngles<adc_t> Conserial::CalcSteps(WAngles<angle_t> angles){

    WAngles<adc_t> steps;
    steps.aHalf_ = CalcStep(angles.aHalf_,standOptions.rotateStep_);
    steps.aQuart_ = CalcStep(angles.aQuart_,standOptions.rotateStep_);
    steps.bHalf_ = CalcStep(angles.bHalf_,standOptions.rotateStep_);
    steps.bQuart_ = CalcStep(angles.bQuart_,standOptions.rotateStep_);
    return steps;
}

WAngles<angle_t> Conserial::CalcAngles(WAngles<adc_t> steps)
{
    WAngles<angle_t> angles;
    angles.aHalf_ = ((float)steps.aHalf_) * standOptions.rotateStep_;
    angles.aQuart_ = ((float)steps.aQuart_) * standOptions.rotateStep_;
    angles.bHalf_ = ((float)steps.bHalf_) * standOptions.rotateStep_;
    angles.bQuart_ = ((float)steps.bQuart_) * standOptions.rotateStep_;
    return angles;
}

std::vector<std::string> Conserial::GetFTDIComPorts() {

    //com_->Close();
    std::vector<std::string> comPorts;  // Вектор для хранения найденных портов
    FT_STATUS ftStatus;                  // Статус выполнения операций FTDI
    DWORD numDevices = 0;                // Количество найденных устройств

    // Создаем список устройств
    ftStatus = FT_CreateDeviceInfoList(&numDevices);
    if (ftStatus != FT_OK) {
        std::cerr << "Ошибка создания списка устройств" << std::endl;
        return comPorts;
    }

    // Проверяем, есть ли устройства
    if (numDevices == 0) {
        std::cout << "FTDI устройства не найдены" << std::endl;
        return comPorts;
    }

    // Перебираем каждое устройство
    for (DWORD i = 0; i < numDevices; i++) {
        FT_HANDLE ftHandle = nullptr;

        // Открываем устройство для получения COM-порта
        ftStatus = FT_Open(i, &ftHandle);
        if (ftStatus == FT_OK) {
            LONG comPortNumber = 0;

            // Получаем номер COM-порта
            ftStatus = FT_GetComPortNumber(ftHandle, &comPortNumber);

            std::cout << "FT_GetComPortNumber: status = "
                      << ftStatus
                      << ", COM = "
                      << comPortNumber
                      << std::endl;

            // Если порт найден (не -1), добавляем его в список
            if (ftStatus == FT_OK && comPortNumber != -1) {
                std::string comPortName = "COM" + std::to_string(comPortNumber);
                comPorts.push_back(comPortName);
            }

            // Закрываем устройство
            FT_Close(ftHandle);
        }
    }

    for (const auto& el : comPorts) {
        std::cout << el << " ";
    }

    return comPorts;
}


}//namespace
