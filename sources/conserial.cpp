/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
/// @version 1.5
/// @copyright Copyright 2022 InfoTeCS.


#include <algorithm>
#include <conserial.h>



//#define NO_SERIAL_LOG

namespace hwe
{
/// @brief Интерфейс для взаимодействия с аппаратной платформой.
Conserial::Conserial()
{
#ifndef NO_SERIAL_LOG
    if (!out_.is_open())
    {
        out_.open("ceserial.log");
    }
#endif
#ifdef CE_WINDOWS
    com_.SetPort("COM3");
#else
    com_.SetPort("/dev/ttyStandQKD");
#endif
    com_.SetBaudRate(115200);
    com_.SetDataSize(8);
    com_.SetParity('N');
    com_.SetStopBits(1);
    com_.Open();

    standOptions.premissions = 0;
    standOptions.laserState_ = 0;
    standOptions.laserPower_ = 0;
    standOptions.signalLevels_ = {0,0};
    standOptions.curAngles_ = {0,0,0,0};
    standOptions.lightNoises = {0,0};
    standOptions.startLightNoises_= {0,0};
    standOptions.startPlatesAngles_ = {0,0,0,0};
    standOptions.maxSignalLevels_ = {0,0};
    standOptions.timeoutTime_ = 2000; //ms
    standOptions.rotateStep_ = 0.3;
    standOptions.maxLaserPower_ = 100;
    standOptions.maxPayloadSize = 30;
    FindProtocolVersion();

}

std::string Conserial::GetComPortName()const
{
    return com_.GetPort();
}

void Conserial::SetComPortName(const char* port)
{
    com_.SetPort(port);
}

Conserial::~Conserial()
{
    out_.close();
}

api:: InitResponse Conserial:: Init()
{
    logNameFunction(__FUNCTION__);
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
    logNameFunction(__FUNCTION__);
    api::InitResponse response = {{0,0},{0,0,0,0},0,{0,0},0}; // Структура для формирования ответа
    uint32_t tempData = standOptions.timeoutTime_; //для FW 1.0
    standOptions.timeoutTime_ = INIT_TIMEOUT_TIME;

    UartResponse pack = Twiting(dict_.at("Init"), nullptr, 0);

    if(pack.parameters_.size() == 9){
        // Заполняем поля структуры
        response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_.at(0)) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
        response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_.at(1)) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
        response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_.at(2)) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
        response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_.at(3)) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

        response.startLightNoises_.h_ = pack.parameters_.at(4); // <- начальная засветка детектора, принимающего горизонтальную поляризацию
        response.startLightNoises_.v_ = pack.parameters_.at(5); //<- начальная засветка детектора, принимающего вертикальную поляризацию

        response.maxSignalLevels_.h_ = pack.parameters_.at(6); // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.maxSignalLevels_.v_ = pack.parameters_.at(7); // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

        response.maxLaserPower_ = pack.parameters_.at(8);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_ = tempData;

    if(static_cast<int>(version_protocol)>3){
        standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
        standOptions.curAngles_ = standOptions.startPlatesAngles_;
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
        standOptions.timeoutTime_ = GetTimeout().adcResponse_;
    }
    return response; // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: "
           + to_string(angles.aHalf_)+
           +" "+to_string(angles.aQuart_)+
           +" "+to_string(angles.bHalf_)+
           +" "+to_string(angles.bQuart_));
    api::InitResponse response = {{0,0},{0,0,0,0},0,{0,0},0}; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    auto bytes = PackToBytes(steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    UartResponse pack = Twiting(dict_.at("InitByButtons"), bytes.data(), bytes.size());

    // Заполняем поля структуры
    if(pack.parameters_.size() == 9){
        // Заполняем поля структуры
        response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_.at(0)) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
        response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_.at(1)) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
        response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_.at(2)) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
        response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_.at(3)) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

        response.startLightNoises_.h_ = pack.parameters_.at(4); // <- начальная засветка детектора, принимающего горизонтальную поляризацию
        response.startLightNoises_.v_ = pack.parameters_.at(5); //<- начальная засветка детектора, принимающего вертикальную поляризацию

        response.maxSignalLevels_.h_ = pack.parameters_.at(6); // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.maxSignalLevels_.v_ = pack.parameters_.at(7); // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

        response.maxLaserPower_ = pack.parameters_.at(8);

        standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
        standOptions.curAngles_ = standOptions.startPlatesAngles_;
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    response.errorCode_ = pack.status_;


    return response; // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа
    uint32_t oldTimeout = standOptions.timeoutTime_;
    standOptions.timeoutTime_ = 10000;
    UartResponse pack = Twiting(dict_.at("RunSelfTest"), nullptr, 0);


    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    standOptions.timeoutTime_ = oldTimeout;
    response.errorCode_ = pack.status_; // Команда отработала корректно

    return response;
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: "
           + to_string(angles.aHalf_)+
           +" "+to_string(angles.aQuart_)+
           +" "+to_string(angles.bHalf_)+
           +" "+to_string(angles.bQuart_)
           +" "+ to_string(power));
    api::SendMessageResponse response= {{0,0,0,0},{0,0},{0,0},0}; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    auto bytes = PackToBytes(steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_, power);

    UartResponse pack = Twiting(dict_.at("SendMessage"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 8){
        // Заполняем поля
        response.newPlatesAngles_.aHalf_  = ((float)pack.parameters_[0]) * standOptions.rotateStep_; // <- полуволновая пластина "Алисы"     (1я пластинка)
        response.newPlatesAngles_.aQuart_ = ((float)pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
        response.newPlatesAngles_.bHalf_  = ((float)pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
        response.newPlatesAngles_.bQuart_ = ((float)pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

        response.currentLightNoises_.h_ = pack.parameters_[4]; // <- засветка детектора, принимающего горизонтальную поляризацию
        response.currentLightNoises_.v_ = pack.parameters_[5]; // <- засветка детектора, принимающего вертикальную поляризацию

        response.currentSignalLevels_.h_ = pack.parameters_[6]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.currentSignalLevels_.v_ = pack.parameters_[7]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере



        standOptions.curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов
        standOptions.lightNoises = response.currentLightNoises_;
        standOptions.signalLevels_ = response.currentSignalLevels_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;
    return response;
}


api::AdcResponse Conserial::SetTimeout(uint32_t timeout)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: " + to_string(timeout) + " мс");
    api::AdcResponse response ={0,0} ;

    if(standOptions.premissions!=1){
        logOut("Отказано в доступе (недостаточно прав)");
        response = {0, static_cast<uint16_t>(ErrorCode::AccessDenied)};
        return response;
    }

    if (timeout < 1000 || timeout >10000){
        logOut("Указано значение параметра вне диапазона (1000 < t < 10000)");
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // Принят некорректный входной параметр
        return response;
    }
    uint16_t minTimeout = timeout & 0xFFFF;

    auto bytes = PackToBytes(minTimeout);

    UartResponse pack = Twiting(dict_.at("SetTimeout"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
        standOptions.timeoutTime_ = response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;



    return response;
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: " + to_string(on));
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // Принят некорректный входной параметр
        return response;
    }

    auto bytes = PackToBytes(on);

    UartResponse pack = Twiting(dict_.at("SetLaserState"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);

        standOptions.laserState_ = response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;
    return response; // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: " + to_string(power));
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    if (power > standOptions.maxLaserPower_)
    {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // Принят некорректный входной параметр
        return response;
    }

    auto bytes = PackToBytes(power);
    UartResponse pack = Twiting(dict_.at("SetLaserPower"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);

        standOptions.laserPower_= response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;



    return response; // Возвращаем значение, соответствующее установленному уровню
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: "
           + to_string(angles.aHalf_)+
           +" "+to_string(angles.aQuart_)+
           +" "+to_string(angles.bHalf_)+
           +" "+to_string(angles.bQuart_));
    api::WAnglesResponse response = {{0,0,0,0},0}; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    auto bytes = PackToBytes(steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    UartResponse pack = Twiting(dict_.at("SetPlatesAngles"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 4){
        steps = {pack.parameters_.at(0),
                 pack.parameters_.at(1),
                 pack.parameters_.at(2),
                 pack.parameters_.at(3)};
        // Записываем полученное в структуру
        response.angles_ =  CalcAngles(steps);
        standOptions.curAngles_ = response.angles_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);


    response.errorCode_ = pack.status_;


    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: "
           + to_string(angles.aHalf_)+
           +" "+to_string(angles.aQuart_)+
           +" "+to_string(angles.bHalf_)+
           +" "+to_string(angles.bQuart_));
    api::WAnglesResponse response = {{0,0,0,0},0}; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        logOut("Отказано в доступе (недостаточно прав)");
        response = {{0,0,0,0}, 5};
        return response;
    }

    WAngles<adc_t> steps = CalcSteps(angles);

    auto bytes = PackToBytes(steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    UartResponse pack = Twiting(dict_.at("UpdateBaseAngles"), bytes.data(), bytes.size());

    if(pack.parameters_.size() == 4){
        steps = {pack.parameters_.at(0),
                 pack.parameters_.at(1),
                 pack.parameters_.at(2),
                 pack.parameters_.at(3)};
        // Записываем полученное в структуру
        response.angles_ =  CalcAngles(steps);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);


    response.errorCode_ = pack.status_;

    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
    logNameFunction(__FUNCTION__);
    api::WAnglesResponse response = {{0,0,0,0},0}; // Структура для формирования ответа


    UartResponse pack = Twiting(dict_.at("ReadBaseAngles"), nullptr, 0);

    WAngles<adc_t> steps = {0,0,0,0};
    if(pack.parameters_.size() == 4){
        steps = {pack.parameters_.at(0),
                 pack.parameters_.at(1),
                 pack.parameters_.at(2),
                 pack.parameters_.at(3)};
        // Записываем полученное в структуру
        response.angles_ =  CalcAngles(steps);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        logOut("Отказано в доступе (недостаточно прав)");
        response = {0, static_cast<uint16_t>(ErrorCode::AccessDenied)};
        return response;
    }

    auto bytes = PackToBytes(numberUnit_);

    UartResponse pack = Twiting(dict_.at("ReadEEPROM"), bytes.data(), bytes.size());

    // Заполняем поля для ответа
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        logOut("Отказано в доступе (недостаточно прав)");
        response = {0, static_cast<uint16_t>(ErrorCode::AccessDenied)};
        return response;
    }

    auto bytes = PackToBytes(numberUnit_, param_);

    UartResponse pack = Twiting(dict_.at("WriteEEPROM"), bytes.data(), bytes.size());

    // Заполняем поля для ответа
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserState()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    UartResponse pack = Twiting(dict_.at("GetLaserState"), nullptr, 0);

    // Заполняем поля для ответа
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
        standOptions.laserState_ = response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;


    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа


    UartResponse pack = Twiting(dict_.at("GetLaserPower"), nullptr, 0);


    // Заполняем поля для ответа
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0);
        standOptions.laserPower_= response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;


    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    logNameFunction(__FUNCTION__);
    api::WAnglesResponse response = {{0,0,0,0},0}; // Структура для формирования ответа

    UartResponse pack = Twiting(dict_.at("GetCurPlatesAngles"), nullptr, 0);

    // Получаем текущие углы поворота волновых пластин от МК
    WAngles<adc_t> steps = {0,0,0,0};
    if(pack.parameters_.size() == 4){
        steps = {pack.parameters_.at(0),
                 pack.parameters_.at(1),
                 pack.parameters_.at(2),
                 pack.parameters_.at(3)};

        response.angles_ =  CalcAngles(steps);
        standOptions.curAngles_ = response.angles_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    // Записываем полученное в структуру

    response.errorCode_ = pack.status_;


    return response;
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    logNameFunction(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack = Twiting(dict_.at("GetSignalLevel"), nullptr, 0);

    // Заполняем структуру для ответа
    if(pack.parameters_.size() == 2){
        response.signal_.h_ = pack.parameters_.at(0); // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.signal_.v_ = pack.parameters_.at(1); // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазер

        standOptions.signalLevels_ = response.signal_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    response.errorCode_ = pack.status_;


    return response;
}

api::AngleResponse Conserial::GetRotateStep()
{
    logNameFunction(__FUNCTION__);
    api::AngleResponse response; // Структура для формирования ответа


    UartResponse pack = Twiting(dict_.at("GetRotateStep"), nullptr, 0);

    // Получаем от МК количество шагов для поворота на 360 градусов
    uint16_t steps_ = 0;

    if(pack.parameters_.size() == 1){
        steps_ = pack.parameters_.at(0);
        if(steps_!=0){  standOptions.rotateStep_ = 360.0 / steps_;} // Считаем сколько градусов в одном шаге

        response.angle_= standOptions.rotateStep_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);


    response.errorCode_ = pack.status_;

    return response;
}

api::SLevelsResponse Conserial::GetLightNoises()
{
    logNameFunction(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack = Twiting(dict_.at("GetLightNoises"), nullptr, 0);

    // Заполняем структуру для ответа
    if(pack.parameters_.size() == 2){
        response.signal_.h_ = pack.parameters_.at(0); // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.signal_.v_ = pack.parameters_.at(1); // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазер

        standOptions.lightNoises = response.signal_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;


    return response;
}

api::AdcResponse Conserial::GetHardwareState(){
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Поле типа adc_t c ответом и код ошибки команды


    UartResponse pack = Twiting(dict_.at("GetHardwareState"), nullptr, 0);
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_.at(0); // Состояние АП

        if (response.adcResponse_ & 0){
            logOut("Аппаратная платформа в рабочем состоянии");
        }else
        {
            if ( response.adcResponse_ & (1<<1) )
                logOut("Не работает фотодетектор PDH");
            if ( response.adcResponse_ & (1<<2) )
                logOut("Не работает фотодетектор PDV");
            if (response.adcResponse_ & (1<<3) )
                logOut("Не работает лазер");
            if ( response.adcResponse_ & (1<<4) )
                logOut("Не работает первый двигатель");
            if ( response.adcResponse_ & (1<<5) )
                logOut("Не работает второй двигатель");
            if ( response.adcResponse_ & (1<<6) )
                logOut("Не работает третий двигатель");
            if (response.adcResponse_ & (1<<7))
                logOut("Не работает четвертый двигатель");
        }

    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;
    return response;
}

api::AdcResponse Conserial::GetErrorCode()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack = Twiting(dict_.at("GetLaserState"), nullptr, 0);
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_[0];
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::GetTimeout()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack = Twiting(dict_.at("GetTimeout"), nullptr, 0);

    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_[0];
        if (pack.parameters_.at(0) > 1000)
            standOptions.timeoutTime_  = response.adcResponse_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);
    response.errorCode_ = pack.status_;

    return response;
}


api::InitResponse Conserial::GetInitParams(){

    logNameFunction(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа

    UartResponse pack = Twiting( dict_.at("GetInitParams"), nullptr, 0);

    if(pack.parameters_.size() == 9){
        // Заполняем поля структуры
        response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
        response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
        response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
        response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

        response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
        response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

        response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
        response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

        response.maxLaserPower_ = pack.parameters_[8];



        standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    response.errorCode_ = pack.status_;

    return response; // Возвращаем сформированный ответ

}

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    logNameFunction(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Заполняем структуру
    response.signal_ = standOptions.startLightNoises_; // <- начальная засветка
    response.errorCode_ = static_cast<uint16_t>(ErrorCode::Success);
    return response;
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    logNameFunction(__FUNCTION__);
    logOut("Параметры: "+to_string(plateNumber)+ " " + to_string(angle));
    api::AngleResponse response; // Структура для формирования ответа
    api::WAnglesResponse tempResponse;
    WAngles <angle_t> angles {};

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // // Принят некорректный входной параметр
        return response;
    }

    switch (plateNumber)
    {
    case 1:
        angles = {angle,
                  standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_,
                  standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 2:
        angles = {standOptions.curAngles_.aHalf_,
                  angle,
                  standOptions.curAngles_.bHalf_,
                  standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aQuart_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 3:
        angles = {standOptions.curAngles_.aHalf_,
                  standOptions.curAngles_.aQuart_,
                  angle,
                  standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.bHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 4:
        angles = {standOptions.curAngles_.aHalf_,
                  standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_,
                  angle};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.bQuart_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    }

    return response; // Возвращаем, чего там получилось установить
}

api::AdcResponse Conserial::GetMaxLaserPower()
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0}; // Структура для формирования ответа

    // Заполняем поля для ответа
    response.adcResponse_ = standOptions.maxLaserPower_;
    response.errorCode_ = static_cast<uint16_t>(ErrorCode::Success);
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{

    logOut(__FUNCTION__ );
    api::WAnglesResponse response = {{0,0,0,0},0}; // Структура для формирования ответа

    // Записываем полученное в структуру
    response.angles_ =  standOptions.startPlatesAngles_;
    response.errorCode_ = static_cast<uint16_t>(ErrorCode::Success);
    // возвращаем структуру
    return response;
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    logNameFunction(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    response.signal_ = standOptions.maxSignalLevels_;
    response.errorCode_ = static_cast<uint16_t>(ErrorCode::Success);
    return response;
}

api::AdcResponse Conserial::CreateConfigSecret(string passwd){

    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0};

    if(standOptions.premissions!=1){
        logOut("Отказано в доступе (недостаточно прав)");
        response = {0,5};
        return response;
    }

    size_t original_len = passwd.size();
    if (original_len > 10 || passwd.empty())
    {
        logOut("Пароль слишком длинный!");
        response.adcResponse_ = 0;
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // Принят некорректный входной параметр
        return response;
    }

    // Вычисляем длину с выравниванием до чётной (padding нулевым байтом сзади)
    size_t padded_len = original_len;
    if (original_len % 2 != 0) {
        padded_len += 1;                        // делаем длину чётной
    }

    std::vector<uint8_t> passwordBytes(padded_len, 0);

    // Копируем пароль
    std::memcpy(passwordBytes.data(), passwd.data(), original_len);


    UartResponse pack;
    pack = Twiting(dict_.at("CreateConfigSecret"),
                   passwordBytes.data(),
                   passwordBytes.size());
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_[0];
        standOptions.premissions  = response.adcResponse_;
        response.errorCode_ = pack.status_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    return response;
}


api::AdcResponse Conserial::OpenConfigMode(string passwd)
{
    logNameFunction(__FUNCTION__);
    api::AdcResponse response = {0,0};
    if (versionFirmware.major <= 1 && versionFirmware.minor<5){
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

    size_t original_len = passwd.size();
    if (original_len > 10 || passwd.empty())
    {
        logOut("Пароль слишком длинный!");
        response.adcResponse_ = 0;
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidInput); // Принят некорректный входной параметр
        return response;
    }

    // Вычисляем длину с выравниванием до чётной (padding нулевым байтом сзади)
    size_t padded_len = original_len;
    if (original_len % 2 != 0) {
        padded_len += 1;                        // делаем длину чётной
    }

    std::vector<uint8_t> passwordBytes(padded_len, 0);

    // Копируем пароль
    std::memcpy(passwordBytes.data(), passwd.data(), original_len);

    UartResponse pack = Twiting(
        dict_.at("OpenConfigMode"),
        passwordBytes.data(),
        passwordBytes.size()
        );
    if(pack.parameters_.size() == 1){
        response.adcResponse_ = pack.parameters_[0];
        response.errorCode_   = pack.status_;
    }else
        response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);

    standOptions.premissions = response.adcResponse_;

    return response;
}


uint16_t Conserial::CloseConfigMode()
{
    logNameFunction(__FUNCTION__);
    standOptions.premissions  = 0;
    return 1;
}

uint16_t Conserial::GetCurrentMode()
{
    logNameFunction(__FUNCTION__);
    return standOptions.premissions;
}

uint16_t Conserial::GetMaxPayloadSize()
{
    logNameFunction(__FUNCTION__);

    UartResponse pack = Twiting(dict_.at("GetMaxPayloadSize"), nullptr, 0);
    if(!pack.parameters_.empty()){
        standOptions.maxPayloadSize = pack.parameters_[0];
    }

    return standOptions.maxPayloadSize;
}

api::versionProtocolResponse Conserial::GetProtocolVersion (){
    logNameFunction(__FUNCTION__);
    api::versionProtocolResponse response;

    if(static_cast<int>(version_protocol)>3){
        UartResponse pack = Twiting(dict_.at("GetProtocolVersion"), nullptr, 0);
        if(pack.parameters_.size() == 2){
            response.version_ = pack.parameters_.at(0);
            response.subversion_ = pack.parameters_.at(1);
            versionProtocol  = {response.version_, response.subversion_};
            response = {versionProtocol.version, versionProtocol.subversion}; //?
            response.errorCode_ = pack.status_;
        }else
            response.errorCode_ = static_cast<uint16_t>(ErrorCode::InvalidResponse);;
    }
    else if (static_cast<int>(version_protocol)== 1) {
        response = {1,0, static_cast<uint16_t>(ErrorCode::Success)};
    }else
        response = {0,0, static_cast<uint16_t>(ErrorCode::Success)};


    return response;
}

api::versionFirmwareResponse Conserial::GetCurrentFirmwareVersion(){
    logNameFunction(__FUNCTION__);
    api::versionFirmwareResponse response;
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:{
        UartResponse pack = Twiting(dict_.at("GetCurrentFirmwareVersion"), nullptr, 0);
        if(!pack.parameters_.empty()){
            response.major_ = pack.parameters_.at(0);
            response.minor_ = pack.parameters_.at(1);
            response.micro_ = pack.parameters_.at(2);

            versionFirmware  = {response.major_,response.minor_,response.micro_};
            response = {versionFirmware.major,versionFirmware.minor,versionFirmware.micro}; //?
        }
        response.errorCode_ = pack.status_;

        break;
    }
    default:
        if(version_protocol == VersionProtocol::protocol_1_0){
            response = {1, 0, static_cast<uint16_t>(ErrorCode::Success)};
        }
        break;
    }
    return response;
}

bool IsBootloaderFile(const string& path) {
    // Простая проверка: если в имени есть "optiboot", "bootloader", "boot"
    string lower = path;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.find("optiboot") != string::npos ||
           lower.find("bootloader") != string::npos ||
           lower.find("boot") != string::npos;
}


api::AdcResponse Conserial::FirmwareUpdate(std::string path)
{
    logNameFunction(__FUNCTION__);

    if (standOptions.premissions != 1) {
        logOut("Отказано в доступе (недостаточно прав)");
        return {0, static_cast<uint16_t>(ErrorCode::AccessDenied)};
    }
    com_.Close();

    /* ==== Открываем BIN ==== */
    FILE* bin = nullptr;

#ifdef _WIN32
    std::wstring wpath(path.begin(), path.end());
    bin = _wfopen(wpath.c_str(), L"rb");
#else
    bin = fopen(path.c_str(), "rb");
#endif

    if (!bin) {
        logOut("Не удалось открыть файл прошивки: " + path);
        return {0, static_cast<uint16_t>(ErrorCode::FirmwareFileNotRead)};
    }

    /* ==== Открываем FTDI ==== */
    FT_HANDLE ft;

    // как в main (2).c — первый FTDI в системе
    if (FT_Open(0, &ft) != FT_OK) {
        logOut("Не удалось открыть FTDI устройство");
        fclose(bin);
        return {0, static_cast<uint16_t>(ErrorCode::NoConnection)};
    }

    /* ==== Настройка порта ==== */
    FT_SetBaudRate(ft, 115200);
    FT_SetDataCharacteristics(ft, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    FT_SetTimeouts(ft, 1000, 1000);

    logOut("FTDI сконфигурирован: 115200, 8N1");

    /* ================= ВХОД В BOOTLOADER ================= */

    logOut("Вход в режим загрузчика...");

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
        logOut("Загрузчик не отвечает");
        FT_Close(ft);
        fclose(bin);
        return {0, static_cast<uint16_t>(ErrorCode::BootloaderNoResponse)};
    }

    logOut("Загрузчик активен");

    /* ================= СТИРАНИЕ FLASH ================= */

    logOut("Стирание FLASH...");
    if (!bl_mass_erase(ft)) {
        logOut("Ошибка стирания FLASH");
        FT_Close(ft);
        fclose(bin);
        return {0, static_cast<uint16_t>(ErrorCode::FlashEraseFailed)};
    }

    /* ================= ЗАПИСЬ ПРОШИВКИ ================= */

    logOut("Запись прошивки...");

    uint8_t buf[256];
    uint32_t addr = 0x08000000;
    size_t r;
    size_t total = 0;
    std::stringstream ss;

    while ((r = fread(buf, 1, sizeof(buf), bin)) > 0) {
        if (!bl_write(ft, addr, buf, (int)r)) {
            ss << std::hex << addr;
            logOut("Ошибка записи @0x" + ss.str());
            FT_Close(ft);
            fclose(bin);
            return {0, static_cast<uint16_t>(ErrorCode::FlashWriteFailed)};
        }

        addr += r;
        total += r;
    }

    logOut("Записано " + std::to_string(total) + " байт");

    /* ================= СБРОС ================= */

    logOut("Сброс устройства...");

    FT_ClrDtr(ft);
    sleep_ms(50);
    FT_ClrRts(ft);
    sleep_ms(50);
    FT_SetRts(ft);
    sleep_ms(50);

    FT_Close(ft);
    fclose(bin);

    logOut("Прошивка успешно завершена");

    return {1, static_cast<uint16_t>(ErrorCode::Success)};
}



void Conserial::FindProtocolVersion(){
    logOut("\t  ******** START " + (string) __FUNCTION__+ " ********");
    int notFound = 1;

    while(notFound !=0 && !(version_protocol == VersionProtocol::unknown)){
        notFound = GetLaserState().errorCode_;
        if(notFound !=0){
            switch (version_protocol) {
            case VersionProtocol::protocol_1_5:
                version_protocol = VersionProtocol::protocol_1_2;
                break;
            case VersionProtocol::protocol_1_2:
                version_protocol = VersionProtocol::protocol_1_0;
                break;
            default:
                version_protocol = VersionProtocol::unknown;
                break;
            }
        }
    }
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:
        versionProtocol  = {1, 5};
        logOut("Version: 1.5");
        break;
    case VersionProtocol::protocol_1_2:
        versionProtocol  = {1, 2};
        logOut("Version: 1.2");
        break;
    case VersionProtocol::protocol_1_0:
        versionProtocol  = {1, 0};
        logOut("Version: 1.0");
        break;
    default:
        versionProtocol  = {0, 0};
        logOut("Version: unknown");
        break;
    }
    logOut("\t ******** END " + (string)__FUNCTION__ + " ******** \n");
}

//          ****** ТРАНСПРОТ ******

Conserial::UartResponse Conserial::Twiting (uint8_t commandName,  uint8_t * bytes, uint16_t length){

    UartResponse pack;

    // Проверка соединения
    if (!StandIsConected())
    {
        pack.status_= 0;
    }else {

        for(int attempt = 0; attempt < 3; ++attempt)
        {
            SendPacket(commandName, bytes, length);
            try {
                pack = ParsePacket();
            } catch (...) {
                cerr << "ОШИБКА: Проблема считывания пакета с UART" << endl;
            }

            if (pack.status_ == 1)
                break;
        }
    }

    pack.status_= CheckStatus(pack.status_);
    return pack;
}


uint16_t Conserial:: SendPacket (uint8_t commandName,  uint8_t * bytes, uint16_t N){

    uint8_t crc = 0;

    std::vector<uint8_t> packData;
    std::vector<uint8_t> packet;

    /*Упаковка пакета*/
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:{
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
    case VersionProtocol::protocol_1_2:{
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
    case VersionProtocol::protocol_1_0:{
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
            logOutLine( to_string(b) + " | " );
        }
        logOut("");
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

    logOut( currentDateTime() + " -- Отправка пакета");

    for (auto b : packet)
    {
        com_.Write(b);
        logOutLine( to_string(b) + " | " );
    }
    logOut("");

    return 1;
}

std::vector<uint8_t> Conserial::ReadPacket(){

    bool success = 0, end_read = 0, start_read = 0;
    clock_t dedline = clock() + (standOptions.timeoutTime_/1000) * CLOCKS_PER_SEC ;
    uint8_t byte = 0;
    uint16_t sliding = 0;
    std::vector<uint8_t> readBytes;

    switch (version_protocol) {
    case VersionProtocol::unknown:
        break;
    default:
        logOut( "\n"+ currentDateTime()+ " -- Поиск начала пакета" );
        while (clock()< dedline && !end_read){
            byte = com_.ReadChar(success);
            if (!success)
                continue;

            sliding = (sliding << 8) |  byte;

            if (!start_read && byte !=0){
                logOutLine( to_string( byte) + " | " );
            }else{
                logOutLine( to_string( byte) + " | " );
            }
            if (!start_read) // Поиск начала пакета
            {
                if (sliding == 0xFFFE)
                {
                    start_read = true;
                    logOut( "\n======== Успешно найдено начало пакета ======== ");
                }
                continue;
            }

            //======== Считывание ========
            readBytes.push_back(byte);

            if (sliding == 0xFFFF)
            {
                end_read = true;
            }

            if (clock()>dedline){ //timeouted
                logOut("\n"+ currentDateTime() + " -- Вышел таймаут" );
                end_read = true;
                readBytes.resize(0);
            }
        }
        break;
    }

    return readBytes;
}

Conserial::UartResponse Conserial::ParsePacket(){
    Conserial::UartResponse pack_;

    vector <uint8_t> readedBytes = ReadPacket() ; // Reading

    /*Парсинг*/
    uint8_t crc = 255;

    if (!readedBytes.empty() && readedBytes.size()>2){ // Считаны байты с UART
        readedBytes.erase(readedBytes.cend()-2, readedBytes.cend()) ;

        switch (version_protocol) {
        case VersionProtocol::protocol_1_5:{
            pack_.payload = (uint16_t) readedBytes.at(0) << 8 | (uint16_t) readedBytes.at(1);
            pack_.nameCommand_ = readedBytes.at(2);
            pack_.status_ = readedBytes.at(3);
            pack_.crc_ = readedBytes.back();
            logOut(to_string((int)readedBytes.size()));
            for (int j = 4; j < (int)readedBytes.size()-3 ; j+=2) {
                pack_.parameters_.push_back((uint16_t) readedBytes.at(j) << 8 | (uint16_t) readedBytes.at(j+1));

            }

            crc = Crc8(readedBytes.data(), readedBytes.size());
            if(crc!=0){
                logOut("\n"+ currentDateTime() + "-- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                logOutUart(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        case VersionProtocol::protocol_1_2:{
            pack_.nameCommand_ = readedBytes.at(0);
            pack_.status_ = readedBytes.at(1);
            pack_.crc_ = readedBytes.back();


            for (int j = 2; j < (int)readedBytes.size(); j+=2) {
                pack_.parameters_.push_back((uint16_t) readedBytes.at(j) << 8 | (uint16_t) readedBytes.at(j+1));

            }

            crc = Crc8(readedBytes.data(), readedBytes.size());
            if(crc!=0){
                logOut("\n" + currentDateTime() + " -- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                logOutUart(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        case VersionProtocol::protocol_1_0:{
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
                logOut("\n" + currentDateTime() + "-- Ошибка: Несоответствие CRC \t" + to_string( crc) );
                logOutUart(pack_);
                pack_.status_ = static_cast<uint16_t>(ErrorCode::CrcMismatch);
            }
            break;
        }
        default:
            break;
        }

    }else{
        logOut("\n" + currentDateTime() + "-- Ошибка: Принятый пакет был сломан");
        pack_.status_ = static_cast<uint16_t>(ErrorCode::InvalidCommand);
    }

    logOut("\n======== Конец пакета ========");
    logOutUart(pack_);

    return pack_;
}

bool Conserial::StandIsConected (){
    if(!com_.IsOpened())
    {
        com_.Close();
        com_.Open();
        if(!com_.IsOpened())
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
        logOut("Успешное выполнение запроса");
    }else if(status == 0){
        errorCode = static_cast<uint16_t>(ErrorCode::NoConnection);
        logOut("Отсутствует соединение со стендом");
    }else if(status == 2){
        logOut("Количество принятых параметров превышает допустимый предел");
    }else if(status == 3){
        logOut("Необнаружена метка конца пакета");
    }else if(status == 4){
        logOut("Не удалось выполнить команду / Не известный ID команды ");
    }else if(status == 5){
        logOut("Отказано в доступе (недостаточно прав)");
    }else if(status == 6){
        logOut("Переданы неверные параметры на вход библиотечной функции");
    }else if(status == 7){
        logOut("Аппаратная платформа в аварийном состоянии");
    }else if(status == 8){
        logOut("Несоответствие CRC");
    }
    logOut("-> Код ошибки: " + to_string(errorCode));
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

//          ========  Step to Angle  & Angle to Step ========
uint16_t Conserial::CalcStep(angle_t angle, angle_t rotateStep){
    if (angle < 0){
        angle = angle + 360;
    }

    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота

    int Steps = round (angle / rotateStep); //Подсчёт и округление шагов
    return Steps;
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

//          ========  ЖУРНАЛИРОВАНИЕ ========

void Conserial::logNameFunction(const char * func){
    logOut("______________________________________________________________");
    logOut((string)func);
    logOut("______________________________________________________________");

}
void Conserial::logOut(std::string str)
{
#ifndef NO_SERIAL_LOG
    if (out_.is_open())
        out_ << str << std::endl;
#endif
}
void Conserial::logOutLine(std::string str)
{
#ifndef NO_SERIAL_LOG
    if (out_.is_open())
        out_ << str ;
#endif
}

const std::string Conserial::currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void Conserial::logOutUart(const UartResponse &pack)
{
    logOut("Whole packet info:");
    logOut("Pack.status = " + std::to_string(pack.status_));
    logOut("Pack.nameCommand = " + std::to_string(pack.nameCommand_));
    logOut("Pack.crc = " + std::to_string(pack.crc_));
    for(int i = 0; i < (int)pack.parameters_.size(); ++i)
        logOut("Pack.parameters[" + std::to_string(i) + "] = " + std::to_string(pack.parameters_[i]));
}



}//namespace
