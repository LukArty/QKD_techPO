/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.

#include <conserial.h>


#include <cstddef>
#include <bitset>
using namespace std;
namespace hwe
{

Conserial::Conserial()
{
    #ifdef CE_WINDOWS
    com_.SetPortName(std::string("COM4"));
    #else
    com_.SetPortName(std::string("/dev/ttyStandQKD"));
    #endif
    com_.SetBaudRate(115200);
    com_.SetDataSize(8);
    com_.SetParity('N');
    com_.SetStopBits(1);
    com_.Open();
    com_.Close();
    com_.Open();

}

Conserial::~Conserial()
{ }

api:: InitResponse Conserial:: Init()
{
    DebugLogger debug(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    std::fstream ini_("./Angles.ini");
    if (!ini_.is_open()) { response = InitByPD();  }
    else {
        int n =6;
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
            WAngles<angle_t> anglesIni_;
            anglesIni_.aHalf_= stof (angle1_) ;
            anglesIni_.aQuart_= stof (angle2_);
            anglesIni_.bHalf_= stof (angle3_);
            anglesIni_.bQuart_= stof (angle4_);
            response =  InitByButtons(anglesIni_);} //Инициализация по концевикам
        else {response = InitByPD();}//Инициализация по фотодиодам
    }
    return debug.Return(response);
}

api::InitResponse Conserial::InitByPD()
{
    DebugLogger debug(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;
    uint16_t tempTimeOut_ = 900;
    uint16_t tempData = standOptions.timeoutTime_;
    standOptions.timeoutTime_ = tempTimeOut_;

    UartResponse pack;
    pack = Twiting(dict_.find("Init")->second, 0);

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[9];
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;
    standOptions.timeoutTime_ = tempData;
    return debug.Return(response); // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);
    api::InitResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("InitByButtons")->second, 4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

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
    return debug.Return(response); // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest(adc_t testId)
{
    DebugLogger debug(__FUNCTION__, 1, testId);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("RunSelfTest")->second, 1, testId);

    response.adcResponse_ = pack.parameters_[0]; // Возвращаем целое число
    response.errorCode_ = pack.status_; // Команда отработала корректно

    return debug.Return(response);
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
    DebugLogger debug(__FUNCTION__, 5, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_, power);
    api::SendMessageResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("SendMessage")->second, 5, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_, power);

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

    return debug.Return(response);
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout)
{
    DebugLogger debug(__FUNCTION__, 1, timeout);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if (timeout <= 0){
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return debug.Return(response);
    }
    else if (timeout >= 900){timeout = 900;}

    UartResponse pack;
    pack = Twiting(dict_.find("SetLaserState")->second,  1, timeout);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_ = response.adcResponse_ ;

    return debug.Return(response);
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
    DebugLogger debug(__FUNCTION__, 1, on);
    api::AdcResponse response; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return debug.Return(response);
    }

    UartResponse pack;
    pack = Twiting(dict_.find("SetLaserState")->second,  1, on);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;
    standOptions.laserState_ = response.adcResponse_;
    return debug.Return(response); // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    DebugLogger debug(__FUNCTION__, 1, power);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    if (power > standOptions.maxLaserPower_)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return debug.Return(response);
    }

    pack = Twiting(dict_.find("SetLaserPower")->second,  1, power);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return debug.Return(response); // Возвращаем значение, соответствующее установленному уровню
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
    DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);
    api::WAnglesResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("SetPlatesAngles")->second,  4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return debug.Return(response); // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
    DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);

    api::WAnglesResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("UpdateBaseAngles")->second,  4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return debug.Return(response); // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
    DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("ReadBaseAngles")->second, 0);

    WAngles<adc_t> steps = {pack.parameters_[0], pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return debug.Return(response);
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
    DebugLogger debug(__FUNCTION__, 1, numberUnit_);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("ReadEEPROM")->second, 1, numberUnit_);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
    DebugLogger debug(__FUNCTION__, 2, numberUnit_, param_);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("WriteEEPROM")->second, 2, numberUnit_, param_);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserState()
{
    DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLaserState")->second, 0);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserState_ = response.adcResponse_;
    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
    DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLaserPower")->second, 0);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return debug.Return(response); // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetCurPlatesAngles")->second, 0);

    // Получаем текущие углы поворота волновых пластин от МК
    WAngles<adc_t> steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetSignalLevel")->second, 0);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазер
    response.errorCode_ = pack.status_;

    standOptions.signalLevels_ = response.signal_;
    return debug.Return(response);
}

api::AngleResponse Conserial::GetRotateStep()
{
    DebugLogger debug(__FUNCTION__);
    api::AngleResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetRotateStep")->second, 0);

    // Получаем от МК количество шагов для поворота на 360 градусов
    uint16_t steps_ = pack.parameters_[0];
    if(steps_!=0){  standOptions.rotateStep_ = 360.0 / steps_;} // Считаем сколько градусов в одном шаге

    response.angle_= standOptions.rotateStep_;
    response.errorCode_ = pack.status_;

    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetLightNoises()
{
    DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLightNoises")->second, 0);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере
    response.errorCode_ = pack.status_;

    standOptions.lightNoises = response.signal_;
    return debug.Return(response);
}

api::AdcResponse Conserial::GetErrorCode()
{
    DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack;
    pack = Twiting(dict_.find("GetErrorCode")->second, 0);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return debug.Return(response);
}

api::AdcResponse Conserial::GetTimeout()
{
    DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack;
    pack = Twiting(dict_.find("GetTimeout")->second, 0);


    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_  = response.adcResponse_;

    return debug.Return(response);
}

api::InitResponse Conserial::GetInitParams(){

    DebugLogger debug(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;


    UartResponse pack;
    pack = Twiting(dict_.find("GetInitParams")->second, 0);


    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[9];
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;

    return debug.Return(response); // Возвращаем сформированный ответ

};

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа


    // получаем от МК начальные уровни засветки

    UartResponse pack;
    pack = Twiting(dict_.find("GetMaxLaserPower")->second, 0);

    response.errorCode_ = pack.status_;
    // Заполняем структуру
    response.signal_.h_ = pack.parameters_[0]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.signal_.v_ = pack.parameters_[1]; // <- начальная засветка детектора, принимающего вертикальную поляризацию

    return debug.Return(response);
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    DebugLogger debug(__FUNCTION__, 2, plateNumber, angle);
    api::AngleResponse response; // Структура для формирования ответа

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = 2; // // Принят некорректный входной параметр
        return debug.Return(response);
    }


    // Рассчитываем шаги...

    adc_t Step;
    Step = CalcStep(angle, standOptions.rotateStep_); //Подсчёт и округление шагов

    UartResponse pack;
    switch (plateNumber)
    {
    case 1:
        pack = Twiting(dict_.find("SetPlatesAngles")->second,  4,
                       Step, standOptions.curAngles_.aQuart_,
                       standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_);

        // Заполняем поля
        response.angle_ = ((float)pack.parameters_[0]) * standOptions.rotateStep_;
        response.errorCode_ = pack.status_;
        standOptions.curAngles_.aHalf_ = response.angle_;

        break;
    case 2:
        pack = Twiting(dict_.find("SetPlatesAngles")->second,  4,
                       standOptions.curAngles_.aHalf_, Step,
                       standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_);

        // Заполняем поля
        response.angle_ = ((float)pack.parameters_[1]) * standOptions.rotateStep_;
        response.errorCode_ = pack.status_;
        standOptions.curAngles_.aQuart_= response.angle_;
        break;
    case 3: pack = Twiting(dict_.find("SetPlatesAngles")->second,  4,
                       standOptions.curAngles_.aHalf_, standOptions.curAngles_.aQuart_,
                       Step, standOptions.curAngles_.bQuart_);

        // Заполняем поля
        response.angle_ = ((float)pack.parameters_[2]) * standOptions.rotateStep_;
        response.errorCode_ = pack.status_;
        standOptions.curAngles_.bHalf_ = response.angle_;
        break;
    case 4: pack = Twiting(dict_.find("SetPlatesAngles")->second,  4,
                       standOptions.curAngles_.aHalf_, standOptions.curAngles_.aQuart_,
                       standOptions.curAngles_.bHalf_, Step);

        // Заполняем поля
        response.angle_ = ((float)pack.parameters_[3]) * standOptions.rotateStep_;
        response.errorCode_ = pack.status_;
        standOptions.curAngles_.bQuart_ = response.angle_;
        break;
    }

    return debug.Return(response); // Возвращаем, чего там получилось установить
}
api::AdcResponse Conserial::GetMaxLaserPower()
{
    DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    // Заполняем поля для ответа
    response.adcResponse_ = standOptions.maxLaserPower_;
    response.errorCode_ = 0;
    return debug.Return(response); // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{
    DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    // Записываем полученное в структуру
    response.angles_ =  standOptions.startPlatesAngles_;
    response.errorCode_ = 0;
    // возвращаем структуру
    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    response.signal_ = standOptions.maxSignalLevels_;

    return debug.Return(response);
}

Conserial::UartResponse Conserial::Twiting (char commandName, int N,... ){
    DebugLogger debug(__FUNCTION__);
    UartResponse pack;
    // Проверка соединения
    if (!StandIsConected())
    {
        pack.status_= 1;
        return pack;
    }

    va_list temp_params;
    va_start(temp_params,N);
    uint16_t *params =new uint16_t [N];
    for (int i =0; i<N; i++){
        params[i]= va_arg(temp_params, unsigned int);
    }
    va_end(temp_params);

    int count = 0;
    while (count<3) {
        //SendUart      //Посылаем запрос МК
        switch (N) {
        case 0:
            SendUart(commandName, N );
            break;
        case 1:
            SendUart(commandName, N , params[0]);
            break;
        case 2:
            SendUart(commandName, N , params[0], params[1]);
            break;
        case 3:
            SendUart(commandName, N , params[0], params[1], params[2]);
            break;
        case 4:
            SendUart(commandName, N , params[0], params[1], params[2], params[3]);
            break;
        case 5:
            SendUart(commandName, N , params[0], params[1], params[2], params[3], params[4]);
            break;
        }

        //Чтение ответа
        pack = ParsePackege(standOptions.timeoutTime_);
        if (pack.status_==1){break;}
        ++count;
    }
    pack.status_= CheckStatus(pack.status_);
    delete [] params;
    return pack;
}

//Функция передачи по uart
uint16_t Conserial:: SendUart (char commandName,int N,...){
    DebugLogger debug(__FUNCTION__);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t end1 = 255;
    uint8_t end2 = 255;

    uint8_t solt = 0;
    uint8_t crc;

    va_list temp_params;
    va_start(temp_params,N);
    uint16_t *params =new uint16_t [N];
    for (int i =0; i<N; i++){
        params[i]= va_arg(temp_params, unsigned int);
    }
    va_end(temp_params);
    uint8_t temp_[64] = {(uint8_t) commandName,};
    int j = 0;
    for (int i = 1; i <= 2 * N; i= i+2){
        temp_[i] = params[j]>>8;
        temp_[i+1] = params[j];
        j++;
    };

    crc = Crc8((uint8_t*)&temp_,2*N+2);

    com_.Write(start1);
    com_.Write(start2);
    com_.Write((uint8_t )commandName);
    for (int s=1 ; s <= 2*N; s++){
        com_.Write((uint8_t ) temp_[s]);
    }
    com_.Write(solt);
    com_.Write(crc);
    com_.Write(end1);
    com_.Write(end2);

    delete [] params;
    return 1;
}

uint8_t Conserial::CheckStatus(uint8_t status){
    DebugLogger debug(__FUNCTION__);
    uint8_t errorCode = 3;
    switch (status) {
    case 1:
        errorCode = 0;
        break;//Успех
    case 2:
        cout<<"Количество принятых параметров превышает допустимый предел"<<endl;
        errorCode = 3;
        break;
    case 4:
        cout<<"Необнаружена метка конца пакета"<<endl;
        errorCode = 3;
        break;
    case 8:
        cout<<"Неизвестный ID  Команды"<<endl;
        errorCode = 3;
        break;
    case 16:
        cout<<"Несоответствие CRC"<<endl;
        errorCode = 3;
        break;
    case 17:
        errorCode = 1; //Проблема с подключением
        break;
    case 18:
        errorCode = 2; //Переданы неверные параметры на вход функции
        break;
    default: errorCode = 3; //Битый пакет (Не известно)
    }

    return errorCode;
}
Conserial::UartResponse Conserial::ParsePackege(unsigned int timeout){
    bool startPackFlag_=false;
    bool successReceipt_=false;
    bool flag_ = 0;
    bool a = 0;
    array <uint16_t, 201>  params  = {0};
    Conserial::UartResponse pack_;
    uint8_t currentByte_=0;
    timeout = timeout * 1000;


    if (!com_.IsOpened())
    {
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    clock_t end_time = clock() + timeout * (CLOCKS_PER_SEC/1000) ;
    while (clock()<end_time) {
        while (startPackFlag_ != true && clock()<end_time) {
            currentByte_ = com_.ReadChar(a);
            if(a){
                if (currentByte_ == 255){
                    flag_ = 1;
                }
                else{
                    if (flag_ == 1){
                        if (currentByte_ != 254)
                        {
                            flag_ = 0;
                        }
                        else{
                            startPackFlag_ = true;
                            break;
                        }
                    }
                    else {
                        flag_ = 0;
                    }
                }
            }
        }

        if (startPackFlag_ == true){
            int count = 9;
            int paramCnt =1;
            unsigned short p=0;
            while (p != 65535 && clock()<end_time) {
                currentByte_ = com_.ReadChar(a);
                if(a){
                    if (count>8){
                        pack_.nameCommand_=currentByte_;
                    }
                    else if (count>7){
                        pack_.status_=currentByte_;
                        if(pack_.status_!=1){
                            return {0,0,pack_.status_,{0,0,0,0,0,0,0,0,0,0}};
                        }
                    }
                    else{
                        p = p + currentByte_;
                        if (count>6){
                            p = p<<8;
                        }
                        else{
                            paramCnt++;
                            if (paramCnt > (int)params.size()+1){
                                return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
                            }
                            if (p==65535){
                                successReceipt_= true;
                                break;}
                            else{
                                params[0] = paramCnt-1;
                                params[paramCnt-1]= p;
                                p=0;
                                count=8;
                            }
                        }
                    }
                    count--;
                }
            }
        }
        else{
            cout<< " Ответа нет или он некорректный"<<endl;
            return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
        }
        if( successReceipt_ == true){
            break;
        }
    }

    // Формирование массива для подсчета CRC
    uint8_t temp_[64] = {pack_.nameCommand_,pack_.status_};
    int j = 1;
    for (int i = 2; i <= 2*params[0]; i= i+2){
        temp_[i] = params[j]>>8;
        temp_[i+1] = params[j];
        j++;
    };
    int crc = Crc8((uint8_t*)&temp_,2*(uint8_t)params[0]+2);
    //Подсчет CRC
    if(crc!=0){
        cout<< "WrongCheckSum"<<"\t" << crc <<endl;
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    //Формирование параметров
    for(int i=0; i<(int)params.size()-1; i++){
        if(i+1 != params[0])
            pack_.parameters_[i] = params[i+1];
        else {
            pack_.crc_ = params[i+1];
            break;
        }
    }
    return pack_ ;
}

// Функция подсчёта контрольной суммы
uint8_t Conserial::Crc8(uint8_t *pcBlock, uint8_t len)
{
    DebugLogger debug(__FUNCTION__);
    uint8_t crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];
    debug.Return();
    return crc;
}

uint16_t Conserial::CalcStep(angle_t angle, angle_t rotateStep){

    DebugLogger debug(__FUNCTION__, 2, angle, rotateStep);
    if (angle < 0){
        angle = angle + 360;
    }

    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота

    int Steps = round (angle / rotateStep); //Подсчёт и округление шагов
    return debug.Return(Steps);
}

WAngles<adc_t> Conserial::CalcSteps(WAngles<angle_t> angles){

    DebugLogger debug(__FUNCTION__, 2, angles, standOptions.rotateStep_);
    WAngles<adc_t> steps;
    steps.aHalf_ = CalcStep(angles.aHalf_,standOptions.rotateStep_);
    steps.aQuart_ = CalcStep(angles.aQuart_,standOptions.rotateStep_);
    steps.bHalf_ = CalcStep(angles.bHalf_,standOptions.rotateStep_);
    steps.bQuart_ = CalcStep(angles.bQuart_,standOptions.rotateStep_);
    return steps;
}

WAngles<angle_t> Conserial::CalcAngles(WAngles<adc_t> steps)
{
    DebugLogger debug(__FUNCTION__, 2, steps, standOptions.rotateStep_);
    WAngles<angle_t> angles;
    angles.aHalf_ = ((float)steps.aHalf_) * standOptions.rotateStep_;
    angles.aQuart_ = ((float)steps.aQuart_) * standOptions.rotateStep_;
    angles.bHalf_ = ((float)steps.bHalf_) * standOptions.rotateStep_;
    angles.bQuart_ = ((float)steps.bQuart_) * standOptions.rotateStep_;
    return angles;
}

bool Conserial::StandIsConected (){
    DebugLogger debug(__FUNCTION__);
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            return 0;
    }
    return 1;
};


}//namespace
