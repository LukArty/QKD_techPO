/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
/// @version 1.5
/// @copyright Copyright 2022 InfoTeCS.

#include <conserial.h>
//#define NO_SERIAL_LOG

namespace hwe
{
/// @brief Интерфейс для взаимодействия с аппаратной платформой.
Conserial::Conserial()
{
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
    com_.Close();
    com_.Open();

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
    logOut(__FUNCTION__);
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
    logOut(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;
    uint16_t tempTimeOut_ = 900;
    uint16_t tempData = standOptions.timeoutTime_;
    standOptions.timeoutTime_ = tempTimeOut_;


    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("Init")->second, bytes, N);

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
    response.errorCode_ = pack.status_;
    if(version_protocol != VersionProtocol::protocol_1_0 && version_protocol != VersionProtocol::unknown){
        standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
        standOptions.curAngles_ = standOptions.startPlatesAngles_;
        standOptions.startLightNoises_ = response.startLightNoises_;
        standOptions.maxSignalLevels_ = response.maxSignalLevels_;
        standOptions.maxLaserPower_ = response.maxLaserPower_;
        standOptions.timeoutTime_ = tempData;
    }
    return response; // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(angles.aHalf_)+ to_string(angles.aQuart_)
           + to_string(angles.bHalf_)+ to_string(angles.bQuart_));
    api::InitResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);


    int N = 4;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);
    UartResponse pack = Twiting(dict_.find("InitByButtons")->second, bytes, N);

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
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.curAngles_ = standOptions.startPlatesAngles_;
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;
    return response; // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest()
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("RunSelfTest")->second, bytes, N);


    response.adcResponse_ = pack.parameters_[0]; // Возвращаем целое число
    response.errorCode_ = pack.status_; // Команда отработала корректно

    return response;
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(angles.aHalf_)+ to_string(angles.aQuart_)
           + to_string(angles.bHalf_)+ to_string(angles.bQuart_) + to_string(power));
    api::SendMessageResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    int N = 5;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_, power);
    UartResponse pack = Twiting(dict_.find("SendMessage")->second, bytes, N);

    // Заполняем поля
    response.newPlatesAngles_.aHalf_  = ((float)pack.parameters_[0]) * standOptions.rotateStep_; // <- полуволновая пластина "Алисы"     (1я пластинка)
    response.newPlatesAngles_.aQuart_ = ((float)pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.newPlatesAngles_.bHalf_  = ((float)pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.newPlatesAngles_.bQuart_ = ((float)pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.currentLightNoises_.h_ = pack.parameters_[4]; // <- засветка детектора, принимающего горизонтальную поляризацию
    response.currentLightNoises_.v_ = pack.parameters_[5]; // <- засветка детектора, принимающего вертикальную поляризацию

    response.currentSignalLevels_.h_ = pack.parameters_[6]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.currentSignalLevels_.v_ = pack.parameters_[7]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов
    standOptions.lightNoises = response.currentLightNoises_;
    standOptions.signalLevels_ = response.currentSignalLevels_;

    return response;
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(timeout));
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if(standOptions.premissions!=1){
        response = {0,257};
        return response;
    }

    if (timeout <= 0){
        response.errorCode_ = 256; // Принят некорректный входной параметр
        return response;
    }
    else if (timeout >= 900){timeout = 900;}

    int N = 1;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, timeout);
    UartResponse pack = Twiting(dict_.find("SetTimeout")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_ = response.adcResponse_ ;

    return response;
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(on));
    api::AdcResponse response; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        response.errorCode_ = 256; // Принят некорректный входной параметр
        return response;
    }

    int N = 1;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, on);
    UartResponse pack = Twiting(dict_.find("SetLaserState")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;
    standOptions.laserState_ = response.adcResponse_;
    return response; // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(power));
    api::AdcResponse response; // Структура для формирования ответа

    if (power > standOptions.maxLaserPower_)
    {
        response.errorCode_ = 256; // Принят некорректный входной параметр
        return response;
    }

    int N = 1;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, power);
    UartResponse pack = Twiting(dict_.find("SetLaserPower")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];

    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return response; // Возвращаем значение, соответствующее установленному уровню
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(angles.aHalf_)+ to_string(angles.aQuart_)
           + to_string(angles.bHalf_)+ to_string(angles.bQuart_));
    api::WAnglesResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    int N = 4;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);
    UartResponse pack = Twiting(dict_.find("SetPlatesAngles")->second, bytes, N);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(angles.aHalf_)+ to_string(angles.aQuart_)
           + to_string(angles.bHalf_)+ to_string(angles.bQuart_));
    api::WAnglesResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {{0,0,0,0}, 257};
        return response;
    }

    WAngles<adc_t> steps = CalcSteps(angles);

    int N = 4;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);
    UartResponse pack = Twiting(dict_.find("UpdateBaseAngles")->second, bytes, N);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
    logOut(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("ReadBaseAngles")->second, bytes, N);

    WAngles<adc_t> steps = {pack.parameters_[0], pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {0, 257};
        return response;
    }

    int N = 1;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, numberUnit_);
    UartResponse pack = Twiting(dict_.find("ReadEEPROM")->second, bytes, N);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {0, 257};
        return response;
    }

    int N = 2;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N, numberUnit_, param_);
    UartResponse pack = Twiting(dict_.find("WriteEEPROM")->second, bytes, N);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserState()
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetLaserState")->second, bytes, N);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserState_ = response.adcResponse_;
    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetLaserPower")->second, bytes, N);


    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    logOut(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetCurPlatesAngles")->second, bytes, N);


    // Получаем текущие углы поворота волновых пластин от МК
    WAngles<adc_t> steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return response;
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    logOut(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetSignalLevel")->second, bytes, N);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазер
    response.errorCode_ = pack.status_;

    standOptions.signalLevels_ = response.signal_;
    return response;
}

api::AngleResponse Conserial::GetRotateStep()
{
    logOut(__FUNCTION__);
    api::AngleResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetRotateStep")->second, bytes, N);

    // Получаем от МК количество шагов для поворота на 360 градусов
    uint16_t steps_ = pack.parameters_[0];
    if(steps_!=0){  standOptions.rotateStep_ = 360.0 / steps_;} // Считаем сколько градусов в одном шаге

    response.angle_= standOptions.rotateStep_;
    response.errorCode_ = pack.status_;

    return response;
}

api::SLevelsResponse Conserial::GetLightNoises()
{
    logOut(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetLightNoises")->second, bytes, N);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере
    response.errorCode_ = pack.status_;

    standOptions.lightNoises = response.signal_;
    return response;
}

api::AdcResponse Conserial::GetHardwareState(){
    logOut(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetHardwareState")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::GetErrorCode()
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetErrorCode")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::GetTimeout()
{
    logOut(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetTimeout")->second, bytes, N);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_  = response.adcResponse_;

    return response;
}

api::InitResponse Conserial::GetInitParams(){

    logOut(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;


    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetInitParams")->second, bytes, N);


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
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;

    return response; // Возвращаем сформированный ответ

}

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    logOut(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Заполняем структуру
    response.signal_ = standOptions.startLightNoises_; // <- начальная засветка
    response.errorCode_ = 0;
    return response;
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    logOut(__FUNCTION__);
    logOut("Параметры: " + to_string(angle));
    api::AngleResponse response; // Структура для формирования ответа
    api::WAnglesResponse tempResponse;
    WAngles <angle_t> angles {};

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = 256; // // Принят некорректный входной параметр
        return response;
    }

    switch (plateNumber)
    {
    case 1:
        angles = {angle,standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 2:
        angles = {standOptions.curAngles_.aHalf_, angle,
                  standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aQuart_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 3:
        angles = {standOptions.curAngles_.aHalf_,standOptions.curAngles_.aQuart_,
                  angle, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.bHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 4:
        angles = {standOptions.curAngles_.aHalf_,standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_, angle};
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
    logOut(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    // Заполняем поля для ответа
    response.adcResponse_ = standOptions.maxLaserPower_;
    response.errorCode_ = 0;
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{

    logOut(__FUNCTION__ );
    api::WAnglesResponse response; // Структура для формирования ответа

    // Записываем полученное в структуру
    response.angles_ =  standOptions.startPlatesAngles_;
    response.errorCode_ = 0;
    // возвращаем структуру
    return response;
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    logOut(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    response.signal_ = standOptions.maxSignalLevels_;
    response.errorCode_ = 0;
    return response;
}

api::AdcResponse Conserial::CreateConfigSecret(string passwd){

    logOut(__FUNCTION__);
    api::AdcResponse response;

    if(standOptions.premissions!=1){
        response = {0, 257};
        return response;
    }



    int str_length = passwd.size();
    if(str_length>20)
    {
        response.errorCode_ = 256; // Принят некорректный входной параметр
        return response;
    }
    //Старшие байты добиваются нулями в случае короткого пароля
    while (str_length<20 ){
        passwd=(char) 0x00 + passwd;
        str_length = passwd.size();
    }


    UartResponse pack;
    pack = Twiting(dict_.find("CreateConfigSecret")->second, &(uint8_t &) passwd[0], str_length);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.premissions  = response.adcResponse_;

    return response;
}

api::AdcResponse Conserial::OpenConfigMode(string passwd){

    logOut(__FUNCTION__);
    api::AdcResponse response;

    int str_length = passwd.size();
    if(str_length>20)
    {
        response.errorCode_ = 5; // Принят некорректный входной параметр
        return response;
    }
    while (str_length<20 ){
        passwd=(char) 0x00 + passwd;
        str_length = passwd.size();
    }

    UartResponse pack;
    pack = Twiting(dict_.find("OpenConfigMode")->second, &(uint8_t &) passwd[0], str_length);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.premissions  = response.adcResponse_;

    return response;
}

uint16_t Conserial::CloseConfigMode()
{
    logOut(__FUNCTION__);
    standOptions.premissions  = 0;
    return 1;
}

uint16_t Conserial::GetCurrentMode()
{
    logOut(__FUNCTION__);
    return standOptions.premissions;
}

uint16_t Conserial::GetMaxPayloadSize()
{
    logOut(__FUNCTION__);
    int N = 0;
    uint8_t *bytes = {new uint8_t[N]};
    ParamToBytes(bytes, N);
    UartResponse pack = Twiting(dict_.find("GetMaxPayloadSize")->second, bytes, N);

    standOptions.maxPayloadSize = pack.parameters_[0];
    return standOptions.maxPayloadSize;
}

hwe::Conserial::versionProtocolResponse Conserial::GetProtocolVersion (){
    logOut(__FUNCTION__);
    hwe::Conserial::versionProtocolResponse response;
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:{
        response = {1,5,0};
        break;
    }
    case VersionProtocol::protocol_1_2:{
        response = {1,2,0};
        break;
    }
    case VersionProtocol::protocol_1_0:{
        response = {1,0,0};

        break;
    }
    default:
        break;
    }

    return response;
}

hwe::Conserial::versionFirmwareResponse Conserial::GetCurrentFirmwareVersion(){
    logOut(__FUNCTION__);
    hwe::Conserial::versionFirmwareResponse response;
    UartResponse pack;
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:{
        int N = 0;
        uint8_t *bytes = {new uint8_t[N]};
        ParamToBytes(bytes, N);
        UartResponse pack = Twiting(dict_.find("GetCurrentFirmwareVersion")->second, bytes, N);
        response.major_ = pack.parameters_[0];
        response.minor_ = pack.parameters_[1];
        response.micro_ = pack.parameters_[2];
        response.errorCode_ = pack.status_;
        versionFirmware  = {response.major_,response.minor_,response.micro_};
        response = {versionFirmware.major,versionFirmware.minor,versionFirmware.micro};
        break;
    }
    default:
        if(version_protocol == VersionProtocol::protocol_1_0){
            response = {1,0,0};
        }
        break;
    }
    return response;
}

void Conserial::FirmwareUpdate (string path){
    logOut(__FUNCTION__);
    string command ="avrdude -v -p atmega328p -c arduino -P " + com_.GetPort() +" -b 115200 -D -U flash:w:\"" + path + "\":i";
    const char * mainCommand= command.c_str();
    if(system(mainCommand)){
        FindProtocolVersion();
    }
    logOut("\n");
}


void Conserial::FindProtocolVersion(){
    logOut("\t  **** START " + (string) __FUNCTION__+ " ****");
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
        logOut("Version: 1.5");
        break;
    case VersionProtocol::protocol_1_2:
        logOut("Version: 1.2");
        break;
    case VersionProtocol::protocol_1_0:
        logOut("Version: 1.0");
        break;
    default:
        logOut("Version: unknown");
        break;
    }
    logOut("\t **** END " + (string)__FUNCTION__ + " **** \n");
}

//          ****** ТРАНСПРОТ ******
Conserial::UartResponse Conserial::Twiting (char commandName, uint8_t * bytes, uint16_t length){

    UartResponse pack;
    // Проверка соединения
    if (!StandIsConected())
    {
        pack.status_= 0;
    }else {

        int count = 0;
        while (count < 3 && pack.status_ != 1) {
            //Посылаем запрос МК
            SendPacket(commandName, bytes, length);

            pack = ParsePacket();
            ++count;
        }
    }

    pack.status_= CheckStatus(pack.status_);
    return pack;
}

void Conserial::ParamToBytes(uint8_t * bytes,  int &quantityP, ...){
    va_list temp_params;
    va_start(temp_params,quantityP);
    uint16_t * params =new uint16_t [quantityP];

    for (int i = 0; i < quantityP; i++){
        params[i]= va_arg(temp_params, unsigned int);
    }
    va_end(temp_params);

    int j = 0 ;
    quantityP *=  2;
    for (int i = 0; i < quantityP; i= i+2){
        bytes[i] = (params[j]>>8);
        bytes[i+1] = params[j];
        j++;
    }

    delete [] params;
}

uint16_t Conserial:: SendPacket (char commandName, uint8_t * bytes, uint16_t N){

    //const int maxBytesWithoutParam = 9; // st0 + st1 + pld0 + pld1 + cN + solt + crc + end0 + end1
    uint8_t crc = 0;

    uint8_t temp_[128] = {uint8_t(commandName)};
    uint8_t packingMessage [64] {};
    int n = 0;
    /*Packing pack*/
    switch (version_protocol) {
    case VersionProtocol::protocol_1_5:{
        temp_[0] = uint8_t((N+2)>>8);
        temp_[1] = uint8_t(N+2);
        temp_[2] = commandName;
        for (int i = 3; i <=  N+2; i++){
            temp_[i] = bytes[i-3];
        }
        crc = Crc8((uint8_t *) &temp_, N+4);//bytes +pld0+pld1+ cN +solt

        packingMessage[0] = 0xFF; //st0
        packingMessage[1] = 0xFE; //st1
        n = 2;
        int j = 0;
        while (n < N+5) {
            packingMessage[n] = temp_[j];
            n++; j++;
        }
        packingMessage[n++] = 0x00; //solt
        packingMessage[n++] = crc; //crc
        packingMessage[n++] = 0xFF; //end1
        packingMessage[n++] = 0xFF; //end2

        break;
    }
    case VersionProtocol::protocol_1_2:{
        for (int i = 1; i <=  N; i++){
            temp_[i] = bytes[i-1];
        }
        crc = Crc8((uint8_t *) &temp_, N+2);//bytes + cN +solt

        packingMessage[0] = 0xFF; //st0
        packingMessage[1] = 0xFE; //st1
        n = 2;
        int j = 0;
        while (n < N+3) {
            packingMessage[n] = temp_[j];
            n++; j++;
        }
        packingMessage[n++] = 0x00; //solt
        packingMessage[n++] = crc; //crc
        packingMessage[n++] = 0xFF; //end1
        packingMessage[n++] = 0xFF; //end2
        break;
    }
    case VersionProtocol::protocol_1_0:{
        for (int i = 1; i <=  N; i++){
            temp_[i] = bytes[i-1];
        }
        crc = Crc8((uint8_t *) &temp_, N+2);//bytes + cN +solt

        packingMessage[0] = 0xFF; //st0
        packingMessage[1] = 0xFE; //st1
        n = 2;
        packingMessage[n++] = 0x00; //solt (status)
        packingMessage[n++] = uint8_t(commandName); //name
        packingMessage[n++] = crc; //crc


        int j = 0;
        while (/*n <= N+4*/j < N) {
            packingMessage[n++] = temp_[1+j];
            j++;
        }

        packingMessage[n++] = 0xFF; //end1
        packingMessage[n++] = 0xFF; //end2

        break;
    }
    default:
        break;
    }

    logOut("Sending message" + currentDateTime());
    for (int i = 0 ; i < n ; i++){
        com_.Write(packingMessage[i]);
        logOut("send byte:" + to_string(packingMessage[i]));
    }
    //delete [] packingMessage;
    return 1;
}

int Conserial::ReadPacket(uint8_t *readBytes, int N ){

    bool success = 0, end_read = 0, start_read = 0;
    int readed  = 0;
    clock_t end_time = clock() + standOptions.timeoutTime_* 1000 * (CLOCKS_PER_SEC/1000) ;
    uint8_t byte = 0;
    uint16_t temp_2b = 0;

    switch (version_protocol) {
    case VersionProtocol::unknown:
        break;
    default:
        logOut("Read_com started to find packet at " + currentDateTime());
        while (clock()< end_time && !end_read){
            byte = com_.ReadChar(success);
            if (success){
                logOut("read byte:" + to_string(byte));
                if (start_read){ // Reading all bytes from package with end bytes;
                    readBytes[readed++] = byte;
                }
                temp_2b = (temp_2b << 8) | (uint16_t) byte;
                if (temp_2b == 0xFFFE && !start_read){ // Starting read
                    logOut("Started read packet (marker successfull)");
                    start_read = true;
                }else if (start_read && temp_2b == 0xFFFF){ // Ending read
                    logOut("Ended read packet (marker successfull)");
                    end_read = 1;
                }
                if (clock()>end_time){ //timeouted
                    logOut("Read timeouted at " + currentDateTime());
                    end_read = 1;
                    readed = 0;
                }
                if (readed > N){ // much date
                    logOut("Was read much date");
                    end_read = 1;
                    readed = 0;
                }
            }
        }
        break;
    }

    return readed;
}

Conserial::UartResponse Conserial::ParsePacket(){
    Conserial::UartResponse pack_;
    if (!com_.IsOpened())
    {
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    uint8_t buffer[200]{}; // Array readed

    int readedBytes = ReadPacket(buffer, 200) ; // Reading
    buffer[--readedBytes] = 0; //Clean first end byte
    buffer[--readedBytes] = 0; //Clean second end byte

    /*Parsing*/

    uint8_t crc = 255;
    if ( readedBytes > 0){ //Success read
        switch (version_protocol) {
        case VersionProtocol::protocol_1_5:{
            pack_.payload = (uint16_t) buffer[0] << 8 | (uint16_t) buffer[1];
            pack_.nameCommand_ = buffer[2];
            pack_.status_ = buffer[3];
            pack_.crc_ = buffer[readedBytes-1];

            int j = 4;
            for (int i = 0; i < 10 && j < readedBytes - 2 ; ++i) {
                pack_.parameters_[i]=(uint16_t) buffer[j] << 8 | (uint16_t) buffer[j+1];
                j+=2;
            }

            crc = Crc8(buffer, readedBytes);
            if(crc!=0){
                logOut("WrongCheckSum \t" + to_string(crc));
                logOutUart(pack_);
                pack_.status_ = 3;
            }
            break;
        }
        case VersionProtocol::protocol_1_2:{
            pack_.nameCommand_ = buffer[0];
            pack_.status_ = buffer[1];
            pack_.crc_ = buffer[readedBytes-1];

            int j = 2;
            for (int i = 0; i < 10 && readedBytes - j > 1 && readedBytes - j < readedBytes - 2; ++i) {
                pack_.parameters_[i]=(uint16_t) buffer[j] << 8 | (uint16_t) buffer[j+1];
                j+=2;
            }

            crc = Crc8(buffer, readedBytes);
            if(crc!=0){
                logOut("WrongCheckSum \t" + to_string(crc));
                logOutUart(pack_);
                pack_.status_ = 3;
            }
            break;
        }
        case VersionProtocol::protocol_1_0:{
            pack_.status_ = buffer[0];
            pack_.nameCommand_ = buffer[1];
            pack_.crc_ = buffer[2];
            int j = 3;
            for (int i = 0; i < 10 && readedBytes - j > 1; ++i) {
                pack_.parameters_[i]=(uint16_t) buffer[j] << 8 | (uint16_t) buffer[j+1];
                j += 2;
            }

            uint16_t temp = pack_.nameCommand_ + pack_.parameters_[0] + pack_.parameters_[1]
                            + pack_.parameters_[2] + pack_.parameters_[3] + pack_.parameters_[4]
                            + pack_.parameters_[5] + pack_.parameters_[6] + pack_.parameters_[7]
                            + pack_.parameters_[8]+ pack_.parameters_[9];

            crc = Crc8((uint8_t *)&temp, sizeof(temp));

            if(!(crc == pack_.crc_)){
                logOut("WrongCheckSum \t" + to_string(crc));
                logOutUart(pack_);
                pack_.status_ = 3;
            }
            break;
        }
        default:
            break;
        }

    }else{ //Not read or not fully read
        logOut("Read packet broken");
        pack_.status_ = 3;
    }

    logOut("Coverted  final packet");
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
        errorCode = 0;
    }else
    {
        if ( !(status ^ 2) ){
            logOut("Количество принятых параметров превышает допустимый предел");}
        if ( !(status ^ 4) ){
            logOut("Необнаружена метка конца пакета");}
        if ( !(status ^ 16) ){
            logOut("Несоответствие CRC");}
        if ( !(status ^ 32) ){
            logOut("Не удалось выполнить команду");}
        if ( !(status ^ 64) ){
            logOut("Аппаратная платформа в аварийном состоянии");}

        if ( !(status ^ 0)){
            errorCode = 1;
            logOut("Проблема с подключением");
        }
        if ( !(status ^ 256)){
            logOut("Переданы неверные параметры на вход библиотечной функции ");
        }
        if ( !(status ^ 257)){
            logOut("Отказано в доступе (недостаточно прав)");
        }
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

//          ****** Step to Angle  & Angle to Step ******
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

//          ****** ЖУРНАЛИРОВАНИЕ ******
void Conserial::logOut(std::string str)
{
#ifndef NO_SERIAL_LOG
    if (!out_.is_open())
    {
        out_.open("ceserial.log");
    }
    if (out_.is_open())
        out_ << str << std::endl;
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
    for(int i = 0; i < 10; ++i)
        logOut("Pack.parameters[" + std::to_string(i) + "] = " + std::to_string(pack.parameters_[i]));
}



}//namespace
