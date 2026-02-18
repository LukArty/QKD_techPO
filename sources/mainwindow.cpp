#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <QSettings>
#include <QVariant>
#include <QFile>
#include <ctime>
#include <QFileInfo>
#include <QTextStream>
#include <QIntValidator>
//#include <iostream>
#include <string>
#include <unistd.h>
#include <conserial.h>
#include <stdlib.h>
#include <helpwindow.h>
#include <random>
#include <QDir>

using namespace std;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mutx_str=false;
    mutx=false;

    ///Начальные параметры
    ParamAngles();


    /// ввод только целых чисел
    ui->Interval->setValidator(new QIntValidator(this));
    ui->LaserPowerValue->setValidator(new QIntValidator(this));
    ui->QuantityTest->setValidator(new QIntValidator(this));
    ui->AutoLine->setValidator(new QIntValidator(this));
    ui->Quantity_test->setValidator(new QIntValidator(this));

    ///Неактивные кнопки, вкладки и текстовые поля
    ui ->EvaBasis->setEnabled(false);
    ui ->EvaBit->setEnabled(false);
    ui ->InitByButtons->setEnabled(false);
    ui ->InitBut->setEnabled(false);
    ui ->Timer_->setEnabled(false);
    ui->tabWidget->setTabEnabled(3,false);

    ///Значения по умолчанию
    ui->radio_ElectionPD->setDown(true);
    ui ->Interval -> setText("180"); //интервал по-умолчанию
    ui->graph->xAxis->setRange(0,180);
    ui->graph->yAxis->setRange(0,3000);
    ui->progressBar->setValue(0);
    ui->Timer_->setInputMask("00:00");
    ui->Timer_->setText("01:00");

    ///скрытие кнопок и текстовые поля
    ui->Error_key->hide();
    ui->Eva_key->hide();
    ui->Protocol_test->hide();
    ui->Quantity_test->hide();

    ///Запуск функций через Enter
    connect(ui->LaserPowerValue, SIGNAL(returnPressed()), this, SLOT(on_SetLaserPowerBut_clicked()));
    connect(ui->AutoLine, SIGNAL(returnPressed()), this, SLOT(on_Auto_Filing_clicked()));
    connect(ui->TimeoutValue, SIGNAL(returnPressed()), this, SLOT(on_SetTimeoutBut_clicked()));
    connect(ui->Port, SIGNAL(returnPressed()), this, SLOT(on_SetComPortName_clicked()));

    ///Переключения между вкладками
    // Инициализируем объект
    keyCNTR1 = new QShortcut(this);
    keyCNTR2 = new QShortcut(this);
    keyCNTR3 = new QShortcut(this);
    keyCNTR4 = new QShortcut(this);
    keyAdmin = new QShortcut(this);

    // Устанавливаем сочетание клавиш
    keyCNTR1->setKey((Qt::CTRL) | Qt::Key_1);
    keyCNTR2->setKey(Qt::CTRL | Qt::Key_2);
    keyCNTR3->setKey(Qt::CTRL | Qt::Key_3);
    keyCNTR4->setKey(Qt::CTRL | Qt::Key_4);
    keyAdmin ->setKey(Qt::SHIFT | Qt::Key_A | Qt::ALT);

    //обработчик нажатия клавиши
    connect(keyCNTR1, SIGNAL(activated()), this, SLOT(slotShortcutCtrl1()));
    connect(keyCNTR2, SIGNAL(activated()), this, SLOT(slotShortcutCtrl2()));
    connect(keyCNTR3, SIGNAL(activated()), this, SLOT(slotShortcutCtrl3()));
    connect(keyCNTR4, SIGNAL(activated()), this, SLOT(slotShortcutCtrl4()));
    connect(keyAdmin, SIGNAL(activated()), this, SLOT(slotShortcutAdmin()));

    this->test_protocol.setFileName("./test_protocol.txt");
    ui->Console_2->setReadOnly(false);  // Разблокировать
}

/// @brief обработчик нажатия клавишиae
void MainWindow::slotShortcutCtrl1()
{
    ui->tabWidget->setCurrentIndex(0);
}
void MainWindow::slotShortcutCtrl2()
{
    ui->tabWidget->setCurrentIndex(1);
}
void MainWindow::slotShortcutCtrl3()
{
    ui->tabWidget->setCurrentIndex(2);
}
void MainWindow::slotShortcutCtrl4()
{
    if(ui->radio_admin->isChecked()){
        ui->tabWidget->setCurrentIndex(3); }
}
void MainWindow::slotShortcutAdmin()
{
    ui->radio_admin->setDown(true);
    ui->radio_admin->click();
}

MainWindow::~MainWindow()
{
    //после закрытия приложения
    stand_.SetLaserPower(0); // установка мощности лазера в 0
    stand_.SetLaserState(0); //перевод лазер в состояние выкл
    delete ui;

}

void MainWindow::ConsoleLog(QString text)
{
    ui->CommandConsole->append(text);
}

void MainWindow::ConsoleLog(QString text, bool bad)
{
    if(bad){
        QTextCharFormat default_format = ui->CommandConsole->currentCharFormat();
        QTextCharFormat bad_format = ui->CommandConsole->currentCharFormat();
        bad_format.setForeground(QBrush(Qt::red));
        ui->CommandConsole->setCurrentCharFormat(bad_format);
        ui->CommandConsole->append(text);
        ui->CommandConsole->setCurrentCharFormat(default_format);
    }
    else ui->CommandConsole->append(text);
}


/// @brief Функция для вывода текущих значений параметров стенда
void MainWindow:: ParamAngles(){
    api::WAnglesResponse response;
    response = stand_.GetPlatesAngles();
    ui->angle_1->setText(QString::number (response.angles_.aHalf_));
    ui->angle_2->setText(QString::number (response.angles_.aQuart_));
    ui->angle_3->setText(QString::number(response.angles_.bHalf_));
    ui->angle_4->setText(QString::number (response.angles_.bQuart_));

    api::AdcResponse response1;
    response1 = stand_.GetLaserState();
    if(response1.adcResponse_ == 1){ui->laser_st->setText("ВКЛ");}
    else{ ui->laser_st->setText("ВЫКЛ");}

    response1 = stand_.GetLaserPower();
    ui->laser_pw->setText(QString::number (response1.adcResponse_));
}

/// @brief Функция выполняет инициализацию по концовикам если есть ini-файл, иначе инициализацию по ФД
void MainWindow::on_InitBut_clicked()
{
    mutx = true;
    QFile fileOut("./Angles.ini");
    QString angles1,angles2, angles3, angles4;
    api::InitResponse response;
    //читаем значения углов с файла
    if (fileOut.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&fileOut);
        QStringList angles;
        while(!stream.atEnd())
        {
            angles << stream.readLine();
        }
        float i = angles[0].toFloat();
        angles1 = QString::number(i);
        i = angles[1].toFloat();
        angles2 = QString::number(i);
        i = angles[2].toFloat();
        angles3= QString::number(i);
        i = angles[3].toFloat();
        angles4 = QString::number(i);
        ui ->InitAngles1 -> setText(angles1);
        ui ->InitAngles2 -> setText(angles2);
        ui ->InitAngles3 -> setText(angles3);
        ui ->InitAngles4 -> setText(angles4);

        //инициализация по консовикам
        response = stand_.InitByButtons({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
        fileOut.close();

        if (response.errorCode_ == 0){
            ConsoleLog("Выполнена команда Init.");
            ConsoleLog("Установленны значения:");
            ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aQuart_));
            ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.startPlatesAngles_.bHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.startPlatesAngles_.bQuart_));

            ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.startLightNoises_.h_));
            ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.startLightNoises_.v_));

            ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
            ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

            ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
        }
        else {
            ConsoleLog("Команда Init не выполена");
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
        }
    }
    else{
        api::InitResponse response;
        response = stand_.InitByPD();
        if (response.errorCode_ == 0){
            ConsoleLog("Выполнена команда Init.");
            ConsoleLog("Установленны значения:");
            ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aQuart_));
            ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.startPlatesAngles_.bHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.startPlatesAngles_.bQuart_));

            ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.startLightNoises_.h_));
            ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.startLightNoises_.v_));

            ConsoleLog("Уровень максимального сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
            ConsoleLog("Уровень максимального сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

            ConsoleLog("Максимальная мощность лазера: "+ QString::number (response.maxLaserPower_));
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        }
        else {
            ConsoleLog("Команда Init не выполена");
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
        }
    }
    mutx = false;
}


void MainWindow::on_GetHardwareState_clicked(){
    mutx = true;
    api::AdcResponse response;
    response = stand_.GetHardwareState();
    ConsoleLog("Выполнена команда GetHardwareState");

    if (response.errorCode_ == 0) {
        QString statusText;
        if (response.adcResponse_ == 0){
            statusText = ("Аппаратная платформа в рабочем состоянии");
            ConsoleLog("Код состояния: 0 - " + statusText);
        }
        else
        {
            if ( response.adcResponse_ & (1<<1) ) {
                statusText = (" Не работает фотодетектор PDH");
                ConsoleLog("Код состояния: 1 - " + statusText);
            }

            if ( response.adcResponse_ & (1<<2) ){
                statusText = (" Не работает фотодетектор PDV");
                ConsoleLog("Код состояния: 2 - " + statusText);
            }
            if (response.adcResponse_ & (1<<3) ){
                statusText = (" Не работает лазер");
                ConsoleLog("Код состояния: 3 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<4) ){
                statusText = (" Не работает первый двигатель");
                ConsoleLog("Код состояния: 4 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<5) ){
                statusText =(" Не работает второй двигатель");
                ConsoleLog("Код состояния: 5 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<6) ){
                statusText = (" Не работает третий двигатель");
                ConsoleLog("Код состояния: 6 - " + statusText);
            }
            if (response.adcResponse_ & (1<<7)){
                statusText = (" Не работает четвертый двигатель");
                ConsoleLog("Код состояния: 7 - " + statusText);
            }
        }
        ConsoleLog("Код ошибки: " + QString::number(response.errorCode_));

    } else {
        ConsoleLog("Код ошибки: " + QString::number(response.errorCode_), 1);
    }

    mutx = false;
}

/// @brief Запускает самодиагностику аппаратной платформы
void MainWindow::on_RunSelfTestBut_clicked()
{
    mutx = true;
    api::AdcResponse response;
    response = stand_.RunTest();
    ConsoleLog("Выполнена команда RunSelfTest");

    if (response.errorCode_ == 0) {
        QString statusText;
        if (response.adcResponse_ == 0){
            statusText = ("Аппаратная платформа в рабочем состоянии");
            ConsoleLog("Код состояния: 0 - " + statusText);
        }
        else
        {
            if ( response.adcResponse_ & (1<<1) ) {
                statusText = (" Не работает фотодетектор PDH");
                ConsoleLog("Код состояния: 1 - " + statusText);
            }

            if ( response.adcResponse_ & (1<<2) ){
                statusText = (" Не работает фотодетектор PDV");
                ConsoleLog("Код состояния: 2 - " + statusText);
            }
            if (response.adcResponse_ & (1<<3) ){
                statusText = (" Не работает лазер");
                ConsoleLog("Код состояния: 3 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<4) ){
                statusText = (" Не работает первый двигатель");
                ConsoleLog("Код состояния: 4 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<5) ){
                statusText =(" Не работает второй двигатель");
                ConsoleLog("Код состояния: 5 - " + statusText);
            }
            if ( response.adcResponse_ & (1<<6) ){
                statusText = (" Не работает третий двигатель");
                ConsoleLog("Код состояния: 6 - " + statusText);
            }
            if (response.adcResponse_ & (1<<7)){
                statusText = (" Не работает четвертый двигатель");
                ConsoleLog("Код состояния: 7 - " + statusText);
            }
        }
        ConsoleLog("Код ошибки: " + QString::number(response.errorCode_));

    } else {
        ConsoleLog("Код ошибки: " + QString::number(response.errorCode_), 1);
    }
    ParamAngles();
    mutx = false;
}

/// @brief Функция определения кода ошибки
void MainWindow::on_GetErrorCodeBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    response = stand_.GetErrorCode();
    ConsoleLog("Выполнена команда GetErrorCode");
    if (response.errorCode_ == 0){

        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);}
    mutx=false;
}

/// @brief Функция определения состояния лазера
void MainWindow::on_GetLaserStateBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    response = stand_.GetLaserState();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetLaserState");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        if(response.adcResponse_ == 1){ui->laser_st->setText("ВКЛ");}
        else{ ui->laser_st->setText("ВЫКЛ");}
    }
    else {
        ConsoleLog("Команда GetLaserState не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция включения и выключения лазера
void MainWindow::on_SetLaserStateBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    api::AdcResponse response1;
    response1 = stand_.GetLaserState();

    if (response1.adcResponse_ == 0)
    {
        response = stand_.SetLaserState(1);
        ui->laser_st->setText("ВКЛ");
    }
    else
    {
        response = stand_.SetLaserState(0);
        ui->laser_st->setText("ВЫКЛ");
    }
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда SetLaserState");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ConsoleLog("Команда SetLaserState не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
        ui->laser_st->setText("ВЫКЛ");
    }
    mutx= false;
}

/// @brief установка мощности лазера
void MainWindow::on_SetLaserPowerBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    QString value_ = ui ->LaserPowerValue -> text();
    response = stand_.SetLaserPower(value_.toUInt());
    if (response.errorCode_ == 0){
        ui->laser_pw->setText(QString::number (response.adcResponse_));
        ConsoleLog("Выполнена команда SetLaserPower");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Значение: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ConsoleLog("Команда SetLaserPower не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция получения текущего значения мощности лазера
void MainWindow::on_GetLaserPowerBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    response = stand_.GetLaserPower();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetLaserPower");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Мощность лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ui->laser_pw->setText(QString::number (response.adcResponse_));
    }
    else {
        ConsoleLog("Команда GetLaserPower не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief определения значения минимального шага двигателя
void MainWindow::on_GetRotateStepBut_clicked()
{
    mutx=true;
    api::AngleResponse response;
    response = stand_.GetRotateStep();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetRotateStep");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Значение шага: "+ QString::number (response.angle_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ConsoleLog("Команда GetRotateStep не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция установка значений углов пластин поворота
void MainWindow::on_SetPlateAngleBut_clicked()
{
    mutx=true;
    api::WAnglesResponse response;
    QString angles1 = ui ->InitAngles1 -> text();
    QString angles2 = ui ->InitAngles2 -> text();
    QString angles3 = ui ->InitAngles3 -> text();
    QString angles4 = ui ->InitAngles4 -> text();
    response = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});

    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда SetPlatesAngles");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда SetPlatesAngles не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция получения текущих значений углов пластин поворота
void MainWindow::on_GetCurPlatesAnglesBut_clicked()
{
    mutx=true;
    api::WAnglesResponse response;
    response = stand_.GetPlatesAngles();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetPlatesAngles");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда GetPlatesAngles не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция установка значения таймаута
void MainWindow::on_SetTimeoutBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    QString value_ = ui -> TimeoutValue-> text();
    if(value_ == " "){
        ConsoleLog("Команда SetTimeout не выполнена");
        ConsoleLog("Не указано значение таймаута");
    }
    response = stand_.SetTimeout(value_.toUInt());
    if(response.errorCode_ == 0){
        ui->Console_2->append("Выполнена команда SetTimeout");
        ui->Console_2->append("Установленные значения:");
        ui->Console_2->append("Значение таймаута: "+ QString::number (response.adcResponse_));
        ui->Console_2->append("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ui->Console_2->append("Команда SetTimeout не выполнена");
        ui->Console_2->append("Код ошибки: "+ QString::number (response.errorCode_));
    }
    mutx=false;
}

/// @brief  получения текущее значение таймаута
void MainWindow::on_GetTimeoutBut_clicked()
{
    mutx=true;
    api::AdcResponse response;
    response = stand_.GetTimeout();
    if(response.errorCode_ == 0){
        ui->Console_2->append("Выполнена команда GetTimeout");
        ui->Console_2->append("Полученные значения:");
        ui->Console_2->append("Значение таймаута: "+ QString::number (response.adcResponse_));
        ui->Console_2->append("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ui->Console_2->append("Команда GetTimeout не выполнена");
        ui->Console_2->append("Код ошибки: "+ QString::number (response.errorCode_));
    }
    mutx=false;
}

/// @brief Функция получения текущих значений на фотодетекторах
void MainWindow::on_GetSignalLevelBut_clicked()
{
    mutx=true;
    api::SLevelsResponse response;
    response = stand_.GetSignalLevels();
    if(response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetSignalLevel");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.signal_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.signal_.v_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ConsoleLog("Команда GetSignalLevel не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Функция получения текущей засветки на фотодетекторах
void MainWindow::on_GetLightNoisesBut_clicked()
{
    mutx=true;
    api::SLevelsResponse response;
    response = stand_.GetLightNoises();
    if(response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetLightNoises");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.signal_.h_));
        ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.signal_.v_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {
        ConsoleLog("Команда GetLightNoises не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}


/// @brief Функция отправки битовой последовательности
void MainWindow::on_SendMessageBut_clicked()
{
    api::SendMessageResponse response;
    api::AdcResponse response_1;
    response_1 = stand_.GetLaserPower();
    int Power = response_1.adcResponse_;
    if(response_1.adcResponse_ == 0){ stand_.SetLaserPower(60); Power = 60; }

    api::AdcResponse response_2;
    response_2 = stand_.GetLaserState();
    if(response_2.adcResponse_ == 0){ stand_.SetLaserState(1);}

    QString angles1 = ui ->InitAngles1 -> text();
    QString angles2 = ui ->InitAngles2 -> text();
    QString angles3 = ui ->InitAngles3 -> text();
    QString angles4 = ui ->InitAngles4 -> text();
    response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles3.toFloat(),angles4.toFloat()}, Power);

    if(response.errorCode_ == 0){
        ConsoleLog("Выполнена команда SendMessage");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.newPlatesAngles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.newPlatesAngles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.newPlatesAngles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.newPlatesAngles_.bQuart_));

        ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.currentLightNoises_.h_));
        ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.currentLightNoises_.v_));

        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.currentSignalLevels_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.currentSignalLevels_.v_));

        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда SendMessage не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
}

/// @brief запись углов ini-файл
void MainWindow::on_WriteIni_clicked()
{
    ConsoleLog("Выполнена команда WriteIni");
    QString angles1 = ui ->InitAngles1 -> text();
    QString angles2 = ui ->InitAngles2 -> text();
    QString angles3 = ui ->InitAngles3 -> text();
    QString angles4 = ui ->InitAngles4 -> text();

    QFile fileOut("./Angles.ini");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);

    angles1 += "\n";
    while(angles1.length() <= 5){ angles1  = "0" + angles1; }
    QTextStream writeStream (&fileOut);
    writeStream << angles1;

    angles2 += "\n";
    while(angles2.length() <= 5){ angles2  = "0" + angles2; }
    QTextStream writeStream2 (&fileOut);
    writeStream2 << angles2;

    angles3 += "\n";
    while(angles3.length() <= 5){ angles3  = "0" + angles3; }
    QTextStream writeStream3 (&fileOut);
    writeStream3 << angles3;

    angles4 += "\n";
    while(angles4.length() <= 5){ angles4  = "0" + angles4; }
    QTextStream writeStream4 (&fileOut);
    writeStream4 << angles4;
    fileOut.close();
}

/// @brief инициализация по датчикам
void MainWindow::on_InitByButtons_clicked()
{
    mutx=true;
    QFile fileOut("./Angles.ini");
    QString angles1,angles2, angles3, angles4;
    api::InitResponse response;

    if (fileOut.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&fileOut);
        QStringList angles;
        while(!stream.atEnd())
        {
            angles << stream.readLine();
        }
        float i = angles[0].toFloat();
        angles1 = QString::number(i);
        i = angles[1].toFloat();
        angles2 = QString::number(i);
        i = angles[2].toFloat();
        angles3= QString::number(i);
        i = angles[3].toFloat();
        angles4 = QString::number(i);
        ui ->InitAngles1 -> setText(angles1);
        ui ->InitAngles2 -> setText(angles2);
        ui ->InitAngles3 -> setText(angles3);
        ui ->InitAngles4 -> setText(angles4);
        response = stand_.InitByButtons({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
        fileOut.close();
    }
    else{
        angles1 = "0";
        angles2 = "0";
        angles3 = "0";
        angles4 = "0";
        response = stand_.InitByButtons({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
    }

    if(response.errorCode_ == 0){
        ConsoleLog("Выполнена команда InitByButtons");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.startPlatesAngles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.startPlatesAngles_.bQuart_));

        ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.startLightNoises_.h_));
        ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.startLightNoises_.v_));

        ConsoleLog("Максимальный уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
        ConsoleLog("Максимальный уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

        ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда InitByButtons не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief считывание углов из ini-файла
void MainWindow::on_ReadIni_clicked()
{
    ConsoleLog("Выполнена команда ReadIni");
    QFile fileOut("./Angles.ini");
    if(fileOut.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&fileOut);
        QStringList angles;
        while(!stream.atEnd())
        {
            angles << stream.readLine();
        }
        float i = angles[0].toFloat();
        angles[0] = QString::number(i);
        i = angles[1].toFloat();
        angles[1] = QString::number(i);
        i = angles[2].toFloat();
        angles[2] = QString::number(i);
        i = angles[3].toFloat();
        angles[3] = QString::number(i);
        ui ->InitAngles1 -> setText(angles[0]);
        ui ->InitAngles2 -> setText(angles[1]);
        ui ->InitAngles3 -> setText(angles[2]);
        ui ->InitAngles4 -> setText(angles[3]);
    }
    fileOut.close();
}

/// @brief инициализация по ФД
void MainWindow::InitByPD(float aHalf_, float aQuart_, float bHalf_, float bQuart_, int h_, int v_, int h_max, int v_max, int Power, int errorCode_){
    ConsoleLog("Выполнена команда InitByPD");
    ConsoleLog("Установленные значения:");
    ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (aHalf_));
    ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (aQuart_));
    ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(bHalf_));
    ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (bQuart_));

    ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (h_));
    ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (v_));

    ConsoleLog("Уровень максимального сигнала на первом фотодетекторе: "+ QString::number (h_max));
    ConsoleLog("Уровень максимального сигнала на втором фотодетекторе: "+ QString::number (v_max));

    ConsoleLog("Максимальная мощность лазера: "+ QString::number (Power));
    ConsoleLog("Код ошибки: "+ QString::number (errorCode_));
    ParamAngles();
    mutx_str=false;
}
void MainWindow::on_InitByPD_clicked()
{
    if(mutx==true){killLoop();}
    if(mutx_str==false){
        api::AdcResponse response;
        response = stand_.GetErrorCode();
        if(response.errorCode_ == 0){
            mutx_str=true;
            pStreamWork = new StreamWork( &stand_);
            pStreamWork->moveToThread(&pMyThread);

            connect(&pMyThread,SIGNAL(started()),pStreamWork,SLOT(InitByPD())); //выполнение протокола
            connect(pStreamWork,SIGNAL(finished1(float, float, float,float, int, int, int, int, int, int)),this,SLOT(InitByPD(float, float, float,float, int, int, int, int, int, int)));

            connect(pStreamWork, &StreamWork::finished1, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
            connect(pStreamWork, SIGNAL(finished1(float, float, float,float, int, int, int, int, int, int)), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
            //connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(terminate())); // когда закончит работу поток, удаляем и его
            pMyThread.start();
        }
        else {
            ConsoleLog("Команда InitByPD не выполнена");
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
        }
    }
}

/// @brief импульсный режим
void MainWindow::on_PulseLaser_clicked()
{
    mutx = true;
    //очистка предыдущего графика
    ui->graph->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика
    ui->graph->addGraph();
    ui->graph->graph(0)->setPen(QPen(Qt::blue));
    ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->graph->addGraph();
    ui->graph->graph(1)->setPen(QPen(Qt::red));
    ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->graph->xAxis->setLabel(" ");
    ui->graph->yAxis->setLabel("Уровень сигнала");

    Flag_ = false; // флаг для остваноки
    api::AdcResponse response;
    api::SLevelsResponse response1;
    api::AdcResponse response_er;
    response_er = stand_.GetErrorCode();
    int cout = 0;
    int y1_max = 0,y1_ = 0, y1_min = 1000000;
    int y2_max = 0,y2_ = 0, y2_min = 1000000;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    if(response_er.errorCode_ == 0){
        while (Flag_ == false)
        {
            // вкл/выкл лазера
            if(cout%2){response = stand_.SetLaserState(1); QThread::msleep(200);}
            else{response = stand_.SetLaserState(0);}
            QThread::msleep(200);
            response1 = stand_.GetSignalLevels();
            x.push_back(cout++);
            y1_ = response1.signal_.h_;
            y2_ = response1.signal_.v_;
            y1.push_back(y1_);
            y2.push_back(y2_);

            //определние мак и мин сигнала
            if(y1_ > y1_max){
                y1_max =y1_;
            }
            if (y1_ < y1_min){
                y1_min =y1_;
            }

            if(y2_ > y2_max){
                y2_max =y2_;
            }
            if (y2_ < y2_min){
                y2_min =y2_;
            }

            //вывод результатов в окно
            PDH_max = QString::number(y1_max);
            PDH_min = QString::number(y1_min);
            PDV_max = QString::number(y2_max);
            PDV_min = QString::number(y2_min);
            ui ->PDH_max -> setText(PDH_max);
            ui ->PDH_min -> setText(PDH_min);
            ui ->PDV_max -> setText(PDV_max);
            ui ->PDV_min -> setText(PDV_min);
            ui ->Cur_PDH -> setText(QString::number(y1_));
            ui ->Cur_PDV -> setText(QString::number(y2_));

            if (y1_max>y2_max) {ui->graph->yAxis->setRange(0 ,y1_max + 10);}
            else{ ui->graph->yAxis->setRange(0 ,y2_max + 1);}
            ui->graph->xAxis->setRange(0 ,cout+10);
            ui->graph->graph(0)->setData(x,y1);
            ui->graph->graph(1)->setData(x,y2);
            ui->graph->replot();

            //остановка цикла по кнопке
            QApplication::processEvents();
            connect( ui->StopLaser, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
        }
        response = stand_.SetLaserState(1);
    }
    else {
        ConsoleLog("Импульсный режим не включен");
        ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Сканирование по 1 пластине
/*void MainWindow::on_ScanAngles1_clicked()
{
    if(mutx_str==false){
        mutx=true;
        //очистка предыдущего графика
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();

        //отрисовка графика
        int interval = ui ->Interval-> text().toInt();
        if(interval == 0){interval=180;ui ->Interval-> setText("180");}
        ui->graph->xAxis->setRange(0,interval);
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->xAxis->setLabel("Угол поворота пластины");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(interval/18.0); // шаг тика должен быть 10,0 или 20,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        api::SLevelsResponse response; //для проведния SendMessage
        api::AdcResponse response_1; //для определения мощности лазера
        //считывание углов и проверка кратности
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);

        double h = (ui ->Step_scan-> value());//значение шага
        response_1 = stand_.GetLaserPower();
        //double Power = response_1.adcResponse_;
        Flag_ = false;

        int y1_max = 0,y1_ = 0, y1_min = 100000;
        int y2_max = 0,y2_ = 0, y2_min = 100000;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        QFile fileOut("./ScanAngles1.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        api::AdcResponse response_er;
        response_er = stand_.GetErrorCode();
        if(response_er.errorCode_ == 0){
            for (float angles=0; angles <= interval;angles +=h)
            {
                if( Flag_ == false){
                    stand_.SetPlatesAngles({angles, angles2, angles3, angles4});
                    response = stand_.GetSignalLevels();
                    x.push_back(angles);
                    y1_ = response.signal_.h_;
                    y2_ = response.signal_.v_;
                    y1.push_back(y1_);
                    y2.push_back(y2_);

                    writeStream << ("Angels: " + (QString::number(angles)).toUtf8() + "\n");
                    writeStream <<("PDH: " + (QString::number(y1_)).toUtf8()+ "\n");
                    writeStream << ("PDV: " + (QString::number(y2_)).toUtf8() + "\n");


                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if (y1_ < y1_min){
                        y1_min =y1_;
                    }

                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                    if (y2_ < y2_min){
                        y2_min =y2_;
                    }

                    PDH_max = QString::number(y1_max);
                    PDH_min = QString::number(y1_min);
                    PDV_max = QString::number(y2_max);
                    PDV_min = QString::number(y2_min);
                    ui ->PDH_max -> setText(PDH_max);
                    ui ->PDH_min -> setText(PDH_min);
                    ui ->PDV_max -> setText(PDV_max);
                    ui ->PDV_min -> setText(PDV_min);
                    ui ->Cur_PDH -> setText(QString::number(y1_));
                    ui ->Cur_PDV -> setText(QString::number(y2_));

                    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}
                    ui->graph->graph(0)->setData(x,y1);
                    ui->graph->graph(1)->setData(x,y2);
                    ui->graph->replot();

                    QApplication::processEvents();
                    connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                    connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                }
            }
            api::SLevelsResponse response_3;
            stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
            response_3 = stand_.GetSignalLevels();
            ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
            ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
            ParamAngles();
        }
        else {
            ConsoleLog("Сканирование не выполнено");
            ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
        }
        mutx=false;
    }
}*/


void MainWindow::on_ScanAngles1_clicked()
{
    if(mutx_str==false){
        mutx=true;
        //очистка предыдущего графика
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();

        //отрисовка графика
        int interval = ui ->Interval-> text().toInt();
        if(interval == 0){interval=180; ui ->Interval-> setText("180");}
        ui->graph->xAxis->setRange(0,interval);
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->xAxis->setLabel("Угол поворота пластины");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(interval/18.0); // шаг тика должен быть 10,0 или 20,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        api::SendMessageResponse response; //для проведния SendMessage
        api::AdcResponse response_1; //для определения мощности лазера
        //считывание углов и проверка кратности
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);

        double h = (ui ->Step_scan-> value());//значение шага
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;

        int y1_max = 0,y1_ = 0, y1_min = 100000;
        int y2_max = 0,y2_ = 0, y2_min = 100000;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        QFile fileOut("./ScanAngles1.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        api::AdcResponse response_er;
        response_er = stand_.GetErrorCode();
        if(response_er.errorCode_ == 0){
            for (float angles=0; angles <= interval;angles +=h)
            {
                if( Flag_ == false){
                    response = stand_.Sendmessage({angles,angles2,angles3,angles4},Power);

                    x.push_back(angles);
                    y1_ = response.currentSignalLevels_.h_;
                    y2_ = response.currentSignalLevels_.v_;
                    y1.push_back(y1_);
                    y2.push_back(y2_);

                    writeStream << ("Angels: " + (QString::number(angles)).toUtf8() + "\n");
                    writeStream <<("PDH: " + (QString::number(y1_)).toUtf8()+ "\n");
                    writeStream << ("PDV: " + (QString::number(y2_)).toUtf8() + "\n");


                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if (y1_ < y1_min){
                        y1_min =y1_;
                    }

                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                    if (y2_ < y2_min){
                        y2_min =y2_;
                    }

                    PDH_max = QString::number(y1_max);
                    PDH_min = QString::number(y1_min);
                    PDV_max = QString::number(y2_max);
                    PDV_min = QString::number(y2_min);
                    ui ->PDH_max -> setText(PDH_max);
                    ui ->PDH_min -> setText(PDH_min);
                    ui ->PDV_max -> setText(PDV_max);
                    ui ->PDV_min -> setText(PDV_min);
                    ui ->Cur_PDH -> setText(QString::number(y1_));
                    ui ->Cur_PDV -> setText(QString::number(y2_));

                    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}
                    ui->graph->graph(0)->setData(x,y1);
                    ui->graph->graph(1)->setData(x,y2);
                    ui->graph->replot();

                    QApplication::processEvents();
                    connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                    connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                }
            }
            api::SLevelsResponse response_3;
            stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
            response_3 = stand_.GetSignalLevels();
            ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
            ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
            ParamAngles();
        }
        else {
            ConsoleLog("Сканирование не выполнено");
            ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
        }
        mutx=false;
    }
}

/// @brief Сканирование по 2 пластине
void MainWindow::on_ScanAngles2_clicked()
{
    if(mutx_str==false){
        mutx=true;
        //очистка предыдущего графика
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();

        //отрисовка графика
        int interval = ui ->Interval-> text().toInt();
        if(interval == 0){interval=180; ui ->Interval-> setText("180");}
        ui->graph->xAxis->setRange(0,interval);
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->xAxis->setLabel("Угол поворота пластины");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(interval/18.0); // шаг тика должен быть 10,0 или 20,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        api::SendMessageResponse response; //для проведния SendMessage
        api::AdcResponse response_1; //для определения мощности лазера
        //считывание углов и проверка кратности
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);

        double h = (ui ->Step_scan-> value());//значение шага
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;

        int y1_max = 0,y1_ = 0, y1_min = 100000;
        int y2_max = 0,y2_ = 0, y2_min = 100000;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        QFile fileOut("./ScanAngles1.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        api::AdcResponse response_er;
        response_er = stand_.GetErrorCode();
        if(response_er.errorCode_ == 0){
            for (float angles=0; angles <= interval;angles +=h)
            {
                if( Flag_ == false){
                    response = stand_.Sendmessage({angles1,angles,angles3,angles4},Power);

                    x.push_back(angles);
                    y1_ = response.currentSignalLevels_.h_;
                    y2_ = response.currentSignalLevels_.v_;
                    y1.push_back(y1_);
                    y2.push_back(y2_);

                    writeStream << ("Angels: " + (QString::number(angles)).toUtf8() + "\n");
                    writeStream <<("PDH: " + (QString::number(y1_)).toUtf8()+ "\n");
                    writeStream << ("PDV: " + (QString::number(y2_)).toUtf8() + "\n");


                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if (y1_ < y1_min){
                        y1_min =y1_;
                    }

                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                    if (y2_ < y2_min){
                        y2_min =y2_;
                    }

                    PDH_max = QString::number(y1_max);
                    PDH_min = QString::number(y1_min);
                    PDV_max = QString::number(y2_max);
                    PDV_min = QString::number(y2_min);
                    ui ->PDH_max -> setText(PDH_max);
                    ui ->PDH_min -> setText(PDH_min);
                    ui ->PDV_max -> setText(PDV_max);
                    ui ->PDV_min -> setText(PDV_min);
                    ui ->Cur_PDH -> setText(QString::number(y1_));
                    ui ->Cur_PDV -> setText(QString::number(y2_));

                    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}
                    ui->graph->graph(0)->setData(x,y1);
                    ui->graph->graph(1)->setData(x,y2);
                    ui->graph->replot();

                    QApplication::processEvents();
                    connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                    connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                }
            }
            api::SLevelsResponse response_3;
            stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
            response_3 = stand_.GetSignalLevels();
            ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
            ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
            ParamAngles();
        }
        else {
            ConsoleLog("Сканирование не выполнено");
            ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
        }
        mutx=false;
    }
}

/// @brief Сканирование по 3 пластине
void MainWindow::on_ScanAngles3_clicked()
{
    if(mutx_str==false){
        mutx=true;
        //очистка предыдущего графика
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();

        //отрисовка графика
        int interval = ui ->Interval-> text().toInt();
        if(interval == 0){interval=180; ui ->Interval-> setText("180");}
        ui->graph->xAxis->setRange(0,interval);
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->xAxis->setLabel("Угол поворота пластины");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(interval/18.0); // шаг тика должен быть 10,0 или 20,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        api::SendMessageResponse response; //для проведния SendMessage
        api::AdcResponse response_1; //для определения мощности лазера
        //считывание углов и проверка кратности
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);

        double h = (ui ->Step_scan-> value());//значение шага
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;

        int y1_max = 0,y1_ = 0, y1_min = 100000;
        int y2_max = 0,y2_ = 0, y2_min = 100000;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        QFile fileOut("./ScanAngles1.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        api::AdcResponse response_er;
        response_er = stand_.GetErrorCode();
        if(response_er.errorCode_ == 0){
            for (float angles=0; angles <= interval;angles +=h)
            {
                if( Flag_ == false){
                    response = stand_.Sendmessage({angles1,angles2,angles,angles4},Power);

                    x.push_back(angles);
                    y1_ = response.currentSignalLevels_.h_;
                    y2_ = response.currentSignalLevels_.v_;
                    y1.push_back(y1_);
                    y2.push_back(y2_);

                    writeStream << ("Angels: " + (QString::number(angles)).toUtf8() + "\n");
                    writeStream <<("PDH: " + (QString::number(y1_)).toUtf8()+ "\n");
                    writeStream << ("PDV: " + (QString::number(y2_)).toUtf8() + "\n");


                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if (y1_ < y1_min){
                        y1_min =y1_;
                    }

                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                    if (y2_ < y2_min){
                        y2_min =y2_;
                    }

                    PDH_max = QString::number(y1_max);
                    PDH_min = QString::number(y1_min);
                    PDV_max = QString::number(y2_max);
                    PDV_min = QString::number(y2_min);
                    ui ->PDH_max -> setText(PDH_max);
                    ui ->PDH_min -> setText(PDH_min);
                    ui ->PDV_max -> setText(PDV_max);
                    ui ->PDV_min -> setText(PDV_min);
                    ui ->Cur_PDH -> setText(QString::number(y1_));
                    ui ->Cur_PDV -> setText(QString::number(y2_));

                    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}
                    ui->graph->graph(0)->setData(x,y1);
                    ui->graph->graph(1)->setData(x,y2);
                    ui->graph->replot();

                    QApplication::processEvents();
                    connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                    connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                }
            }
            api::SLevelsResponse response_3;
            stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
            response_3 = stand_.GetSignalLevels();
            ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
            ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
            ParamAngles();
        }
        else {
            ConsoleLog("Сканирование не выполнено");
            ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
        }
        mutx=false;
    }
}

/// @brief Сканирование по 4 пластине
void MainWindow::on_ScanAngles4_clicked()
{
    if(mutx_str==false){
        mutx=true;
        //очистка предыдущего графика
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();

        //отрисовка графика
        int interval = ui ->Interval-> text().toInt();
        if(interval == 0){interval=180; ui ->Interval-> setText("180");}
        ui->graph->xAxis->setRange(0,interval);
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->xAxis->setLabel("Угол поворота пластины");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(interval/18.0); // шаг тика должен быть 10,0 или 20,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        api::SendMessageResponse response; //для проведния SendMessage
        api::AdcResponse response_1; //для определения мощности лазера
        //считывание углов и проверка кратности
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);

        double h = (ui ->Step_scan-> value());//значение шага
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;

        int y1_max = 0,y1_ = 0, y1_min = 100000;
        int y2_max = 0,y2_ = 0, y2_min = 100000;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        QFile fileOut("./ScanAngles1.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        api::AdcResponse response_er;
        response_er = stand_.GetErrorCode();
        if(response_er.errorCode_ == 0){
            for (float angles=0; angles <= interval;angles +=h)
            {
                if( Flag_ == false){
                    response = stand_.Sendmessage({angles1,angles2,angles3,angles},Power);

                    x.push_back(angles);
                    y1_ = response.currentSignalLevels_.h_;
                    y2_ = response.currentSignalLevels_.v_;
                    y1.push_back(y1_);
                    y2.push_back(y2_);

                    writeStream << ("Angels: " + (QString::number(angles)).toUtf8() + "\n");
                    writeStream <<("PDH: " + (QString::number(y1_)).toUtf8()+ "\n");
                    writeStream << ("PDV: " + (QString::number(y2_)).toUtf8() + "\n");


                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if (y1_ < y1_min){
                        y1_min =y1_;
                    }

                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                    if (y2_ < y2_min){
                        y2_min =y2_;
                    }

                    PDH_max = QString::number(y1_max);
                    PDH_min = QString::number(y1_min);
                    PDV_max = QString::number(y2_max);
                    PDV_min = QString::number(y2_min);
                    ui ->PDH_max -> setText(PDH_max);
                    ui ->PDH_min -> setText(PDH_min);
                    ui ->PDV_max -> setText(PDV_max);
                    ui ->PDV_min -> setText(PDV_min);
                    ui ->Cur_PDH -> setText(QString::number(y1_));
                    ui ->Cur_PDV -> setText(QString::number(y2_));

                    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}
                    ui->graph->graph(0)->setData(x,y1);
                    ui->graph->graph(1)->setData(x,y2);
                    ui->graph->replot();

                    QApplication::processEvents();
                    connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                    connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                }
            }
            api::SLevelsResponse response_3;
            stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
            response_3 = stand_.GetSignalLevels();
            ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
            ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
            ParamAngles();
        }
        else {
            ConsoleLog("Сканирование не выполнено");
            ConsoleLog("Код ошибки: "+ QString::number (response_er.errorCode_), 1);
        }
        mutx=false;
    }
}

/// @brief Мониторинг по фодотедекторам (постоянное снятие показателей с фд)
void MainWindow::on_MonitoringPD_clicked()
{
    if(mutx_str==false){
        mutx=true;
        timer2 = new QTimer(this);
        timer = new QTimer(this);
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();
        number=0;

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        ui->graph->xAxis->setLabel("");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        QSharedPointer<QCPAxisTicker> autoTicker(new QCPAxisTicker);
        ui->graph->xAxis->setTicker(autoTicker);

        QString time = ui -> Timer_-> text();
        QString per;

        for(int i = 0; i<time.size();i++){
            if(i==0|| i==1){
                per += time[i];
                m=per.toInt();
            }
            else if(i==3|| i==4){
                per += time[i];
                s=per.toInt();
            }
            else{per="";}
        }
        api::SLevelsResponse response;
        api::AdcResponse response_1;
        api::AdcResponse response_2;
        Flag_ = false;

        int cout = 0;
        float y1_max = 0,y1_ = 0, y1_min =100000, pdh=0;
        float y2_max = 0,y2_ = 0, y2_min =100000, pdv=0;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        if(response.errorCode_ == 0){
            if(ui->timer_check->isChecked()){
                connect(timer, &QTimer::timeout, this, &MainWindow::killLoop);
                timer->start(((m*60)+s)*1000);
                connect(timer2, &QTimer::timeout, this, &MainWindow::slotTimerAlarm);
                timer2->start(1000);
                ui ->MonitoringPD->setEnabled(false);
                ui ->timer_check->setEnabled(false);
            }
            while (Flag_ == false)
            {
                response = stand_.GetSignalLevels();
                x.push_back(cout++);
                y1_ = response.signal_.h_;
                y2_ = response.signal_.v_;
                y1.push_back(y1_);
                y2.push_back(y2_);

                /*if(y1_ == 0 || y2_ == 0){
                response = stand_.GetSignalLevels();
                y1_ = response.signal_.h_;
                y2_ = response.signal_.v_;
                y1.push_back(y1_);
                y2.push_back(y2_);
            }*/

                pdh=pdh+response.signal_.h_;;
                pdv=pdv+response.signal_.v_;

                response_1 = stand_.GetLaserPower();

                if(y1_ > y1_max){
                    y1_max =y1_;
                }
                if (y1_ < y1_min){
                    y1_min =y1_;
                }

                if(y2_ > y2_max){
                    y2_max =y2_;
                }
                if (y2_ < y2_min){
                    y2_min =y2_;
                }

                PDH_max = QString::number(y1_max);
                PDH_min = QString::number(y1_min);
                PDV_max = QString::number(y2_max);
                PDV_min = QString::number(y2_min);
                ui ->PDH_max -> setText(PDH_max);
                ui ->PDH_min -> setText(PDH_min);
                ui ->PDV_max -> setText(PDV_max);
                ui ->PDV_min -> setText(PDV_min);
                ui ->Cur_PDH -> setText(QString::number(y1_));
                ui ->Cur_PDV -> setText(QString::number(y2_));

                if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}

                ui->graph->xAxis->setRange(0 ,cout + 10);
                ui->graph->graph(0)->setData(x,y1);
                ui->graph->graph(1)->setData(x,y2);
                ui->graph->replot();
                ui->max_PDH->setText(QString::number (((y1_max-(pdh/cout))*100)/(pdh/cout)));
                ui->min_PDH->setText(QString::number ((((pdh/cout)-y1_min)*100)/(pdh/cout)));
                ui->max_PDV->setText(QString::number (((y2_max-(pdv/cout))*100)/(pdv/cout)));
                ui->min_PDV->setText(QString::number ((((pdv/cout)-y2_min)*100)/(pdv/cout)));
                QApplication::processEvents();

                connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->MonitoringSend, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->MonitorNoises, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles1, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles2, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles3, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles4, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
            if(ui->timer_check->isChecked()){
                timer->stop();
                timer2->stop();
                ui->Timer_->setText(time);
            }
            ConsoleLog("Среднее знаение для PDH: " + QString::number (pdh/cout));
            ConsoleLog("Отклонение от максимума PDH: " + QString::number (((y1_max-(pdh/cout))*100)/(pdh/cout))+ " %");
            ConsoleLog("Отклонение от минимума PDH: " + QString::number ((((pdh/cout)-y1_min)*100)/(pdh/cout))+ " %");
            ConsoleLog("Среднее знаение для PDV: " + QString::number (pdv/cout));
            ConsoleLog("Отклонение от максимума PDV: " + QString::number (((y2_max-(pdv/cout))*100)/(pdv/cout))+ " %");
            ConsoleLog("Отклонение от минимума PDV: " + QString::number ((((pdv/cout)-y2_min)*100)/(pdv/cout))+ " %");
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);}
        mutx = false;
    }

}
/// @brief обновление таймера
void MainWindow::slotTimerAlarm()
{
    if(m>=1 && s == 0){
        s=60;
        m--;
    }
    s--;
    if(s < 10 && m < 10){ui->Timer_->setText("0"+QString::number(m) +":"+"0"+QString::number(s));}
    else if(m < 10 && s >= 10){ui->Timer_->setText("0"+QString::number(m) +":"+QString::number(s));}
    else if(s < 10 && m >= 10){ui->Timer_->setText(QString::number(m) +":"+"0"+QString::number(s));}
    else{ui->Timer_->setText(QString::number(m) +":"+QString::number(s));}
}

/// @brief параметры после инициализации
void MainWindow::on_GetInitParams_clicked()
{
    mutx=true;
    api::InitResponse response;
    response = stand_.GetInitParams();

    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда GetInitParams:");
        ConsoleLog("Полученные значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.startPlatesAngles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.startPlatesAngles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.startPlatesAngles_.bQuart_));

        ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.startLightNoises_.h_));
        ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.startLightNoises_.v_));

        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

        ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда GetInitParams не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
    mutx=false;
}

/// @brief Тестирование светоделителя
void MainWindow::on_PDRatio_clicked()
{
    if(mutx==false && mutx_str==false){
        float angles1 = ui->angle_1->text().toFloat(); // значения для Sendmessage
        float angles2 =  ui->angle_1->text().toFloat();
        float angles3 =  ui->angle_1->text().toFloat();
        float angles4 =  ui->angle_1->text().toFloat();
        float ratio_1, ratio_2;

        api::SendMessageResponse response_1;
        api::AdcResponse response_2;
        float h = 0.9; //шаг
        float y1_=0, y2_=0;
        float sum_PDH=0, sum_PDV=0;
        response_2 = stand_.GetLaserPower();
        double Power = response_2.adcResponse_;
        int qua = ui ->QuantityTest -> text().toInt();//количество тестов
        //выполняем тесты
        if(qua <= 0){qua=1; ui ->QuantityTest -> setText("1");}
        for (int i = 1; i <= qua; i++){
            for (float n = angles1; n <= angles1+180; n +=h)
            {
                response_1  = stand_.Sendmessage({n, angles2, angles3, angles4}, Power);
                y1_ = response_1.currentSignalLevels_.h_;
                y2_ = response_1.currentSignalLevels_.v_;
                sum_PDH += y1_;
                sum_PDV += y2_;
            }
            ratio_1= (sum_PDH*100)/(sum_PDH + sum_PDV);
            ratio_2= (sum_PDV*100)/(sum_PDH + sum_PDV);
            ConsoleLog("Номер теста: "+ QString::number (i));
            ConsoleLog("Сумма всех значений полученых на PDH: "+ QString::number (sum_PDH));
            ConsoleLog("Сумма всех значений полученых на PDV: "+ QString::number (sum_PDV));
            ConsoleLog("Соотношение мощностей: "+ QString::number (ratio_1)+" %" +"   "+ QString::number (ratio_2)+" %");
            sum_PDH=0;
            sum_PDV=0;
        }
    }
}

/// @brief тестирование лазера
void MainWindow::LaserTest(float y1_, float y2_,int Power)
{
    ui->laser_pw->setText(QString::number (Power));
    x.push_back(Power);
    y1.push_back(y1_);
    y2.push_back(y2_);
    //определение мак и мин сигнала
    if(y1_ > y1_max){
        y1_max =y1_;
    }
    if (y1_ < y1_min){
        y1_min =y1_;
    }

    if(y2_ > y2_max){
        y2_max =y2_;
    }
    if (y2_ < y2_min){
        y2_min =y2_;
    }

    //вывод значений
    ui ->PDH_max -> setText(QString::number(y1_max));
    ui ->PDH_min -> setText(QString::number(y1_min));
    ui ->PDV_max -> setText(QString::number(y2_max));
    ui ->PDV_min -> setText(QString::number(y2_min));
    ui ->Cur_PDH -> setText(QString::number(y1_));
    ui ->Cur_PDV -> setText(QString::number(y2_));

    if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
    else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}

    ui->graph->xAxis->setRange(0 ,Power);
    ui->graph->graph(0)->setData(x,y1);
    ui->graph->graph(1)->setData(x,y2);
    ui->graph->replot();

    if(Power == 100){ConsoleLog("Тестирование закончено!");mutx_str=false;}
}
/// @brief тестирование лазера
void MainWindow::on_LaserTest_clicked()
{
    if( mutx_str==false){
        //очистка поля под график
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();
        number =0;

        //отрисовка графика
        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
        ui->graph->xAxis->setLabel("Мощность лазера");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->graph->xAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(5.0); // шаг тика должен быть 5,0
        fixedTicker->setScaleStrategy(QCPAxisTickerFixed::ssNone);

        y1_max = 0, y1_min =100000;
        y2_max = 0, y2_min =100000;
        mutx_str=true;

        stand_.SetLaserState(1);
        ui->laser_st->setText("ВКЛ");

        api::AdcResponse response;
        response = stand_.GetErrorCode();
        if(response.errorCode_ == 0){
            pStreamWork = new StreamWork(&stand_);
            pStreamWork->moveToThread(&pMyThread);
            connect(&pMyThread,SIGNAL(started()),pStreamWork,SLOT(LaserTest())); //выполнение протокола
            connect(pStreamWork,SIGNAL(emitdate(float,float,int)),this,SLOT(LaserTest(float,float,int)));
            connect(pStreamWork, &StreamWork::finished, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
            connect(pStreamWork, SIGNAL(finished()), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
            connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(quit())); // когда закончит работу поток, удаляем и его
            pMyThread.start();
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);}
    }
}

/// @brief проверка введенных углов на кратрость шагу двигателя
float MainWindow:: AngleCheck (float angle, float step)
{
    angle = (angle*10);
    int an = angle;
    int h = step*10;
    if(an % h != 0){
        while (an % h != 0) {
            an += 1;
            angle +=1;
        }
        return angle/10;
    }
    else{return angle/10;}
}

/// @brief Методы для вывода обновления значений
void MainWindow::update(int i, int size,QStringList bit, QStringList Combit, QStringList blankbit, QStringList keybit, double error){
    //Progress
    ui->Progress->clear();
    ui ->Progress->setText(QString::number(i+1)+" из "+QString::number(size));
    ui->progressBar->setValue(((i+1)*100)/size);
    //Вывод сырой строки
    ui->RawLine->clear();
    ui ->RawLine -> setText(bit.join(""));
    //Сравнение базисов
    ui->Comparison->clear();
    ui ->Comparison -> setText(Combit.join(""));
    //чистая строка
    ui->BlankLine->clear();
    ui ->BlankLine -> setText(blankbit.join(""));
    //Вывод ключа
    ui->key->clear();
    ui->LengthKey->clear();
    ui ->key -> setText(keybit.join(""));
    ui ->LengthKey->setText(QString::number(keybit.join("").size()));
    //вывод процента ошибок
    ui->error_pr->clear();
    ui ->error_pr -> setText(QString::number(error)+ "%");
}
void MainWindow::update(int i, int size,QStringList bit,QStringList bit_e, QStringList Combit, QStringList blankbit, QStringList keybit, double error){
    //Progress
    ui->Progress->clear();
    ui ->Progress->setText(QString::number(i+1)+" из "+QString::number(size));
    ui->progressBar->setValue(((i+1)*100)/size);
    //Вывод сырой строки
    ui->RawLine->clear();
    ui ->EvaBit -> setText(bit_e.join(""));
    ui ->RawLine -> setText(bit.join(""));
    //Сравнение базисов
    ui->Comparison->clear();
    ui ->Comparison -> setText(Combit.join(""));
    //чистая строка
    ui->BlankLine->clear();
    ui ->BlankLine -> setText(blankbit.join(""));
    //Вывод ключа
    ui->key->clear();
    ui->LengthKey->clear();
    ui ->key -> setText(keybit.join(""));
    ui ->LengthKey->setText(QString::number(keybit.join("").size()));
    //вывод процента ошибок
    ui->error_pr->clear();
    ui ->error_pr -> setText(QString::number(error)+ "%");
}

/// @brief Действия после выполнения протокола
void MainWindow::Output_bit(){
    //возвращение в начальное положение
    api::WAnglesResponse response;
    response = stand_.SetPlatesAngles({0,0,0,0});
    setUpdatesEnabled(true);
    ParamAngles();
    ui ->TestLine->setEnabled(true);
    ui ->Auto_Filing->setEnabled(true);
    protocol=false;
    mutx_str=false;
}

/// @brief Методы для вывода значения времени, скорости, а также для передачи данных в метод BiuldHistogram
void MainWindow::Date_time_hist(float search_time,float speed,QStringList signalH,QStringList signalV){
    ui ->Time->setText(QString::number(search_time) + "с"); //вывод времени
    ui ->Speed->setText(QString::number(speed/1000)); //вывод скорости
    hst.BiuldHistogram(signalH, signalV);
    if(Flag_1 == true){
        ui ->Time->clear();
        ui ->Speed->clear();
    }
}
void MainWindow::Date_time_hist(float search_time,float speed,QStringList signalH_AE,QStringList signalV_AE, QStringList signalH_EB, QStringList signalV_EB){
    ui ->Time->setText(QString::number(search_time) + "с"); //вывод времени
    ui ->Speed->setText(QString::number(speed/1000)); //вывод скорости
    eva.BiuldHistogram(signalH_AE, signalV_AE, signalH_EB, signalV_EB);
    if(Flag_1 == true){
        ui ->Time->clear();
        ui ->Speed->clear();
    }
}

/// @brief Запуск выполнения протокола
void MainWindow::on_Start_protocol_clicked()
{
    if(mutx_str==false){
        mutx_str=true;
        pStreamWork = new StreamWork(&stand_);
        //очистка строк
        ui->progressBar->setValue(0);
        ui ->RawLine ->clear();
        ui ->Comparison ->clear();
        ui ->BlankLine ->clear();
        ui ->LengthKey->clear();
        ui ->key -> clear();
        ui ->LengthKey->clear();
        ui ->Time->clear();
        ui ->Speed->clear();
        ui ->error_pr->clear();
        ui ->Progress->clear();

        protocol = true;
        Flag_1 = false;

        ui ->TestLine->setEnabled(false);
        ui ->Auto_Filing->setEnabled(false);

        if((ui ->AliceBit -> text()) == "" || (ui ->AliceBasis -> text()) == "" ||(ui ->BobBit -> text()) == "" || (ui ->BobBasis -> text()) == "" ){
            QMessageBox::critical(this,
                                  "Ошибка!",
                                  "Не все поля заполнены!",
                                  QMessageBox::Ok);
            ui ->TestLine->setEnabled(true);
            ui ->Auto_Filing->setEnabled(true);
        }
        else{
            float step = stand_.GetRotateStep().angle_; //шаг двигателя
            //Углы пластин Алисы
            pStreamWork->aHalf_00 = AngleCheck((ui ->aHalf_00 -> text().toFloat()), step); //базис 0 бит 0
            ui ->aHalf_00 -> setText(QString::number(pStreamWork-> aHalf_00));
            pStreamWork-> aQuart_00 = AngleCheck((ui ->aQuart_00 -> text().toFloat()), step); //базис 0 бит 0
            ui ->aQuart_00 -> setText(QString::number(pStreamWork-> aQuart_00));

            pStreamWork-> aHalf_01 = AngleCheck((ui ->aHalf_01 -> text().toFloat()), step); //базис 0 бит 1
            ui ->aHalf_01 -> setText(QString::number(pStreamWork-> aHalf_01));
            pStreamWork-> aQuart_01 = AngleCheck((ui ->aQuart_01 -> text().toFloat()), step); //базис 0 бит 1
            ui ->aQuart_01 -> setText(QString::number(pStreamWork->aQuart_01));

            pStreamWork-> aHalf_10 = AngleCheck((ui ->aHalf_10 -> text().toFloat()), step); //базис 1 бит 0
            ui ->aHalf_10 -> setText(QString::number(pStreamWork->aHalf_10));
            pStreamWork-> aQuart_10 = AngleCheck((ui ->aQuart_10 -> text().toFloat()), step); //базис 1 бит 0
            ui ->aQuart_10 -> setText(QString::number(pStreamWork->aQuart_10));

            pStreamWork-> aHalf_11 = AngleCheck((ui ->aHalf_11 -> text().toFloat()), step); //базис 1 бит 1
            ui ->aHalf_11 -> setText(QString::number(pStreamWork->aHalf_11));
            pStreamWork-> aQuart_11 = AngleCheck((ui ->aQuart_11 -> text().toFloat()), step); //базис 1 бит 1
            ui ->aQuart_11 -> setText(QString::number(pStreamWork->aQuart_11));

            //Углы пластин Bob
            float bHalf_00 = AngleCheck((ui ->bHalf_00 -> text().toFloat()), step); //базис 0 бит 0
            ui ->bHalf_00 -> setText(QString::number(bHalf_00));
            float bQuart_00 = AngleCheck((ui ->bQuart_00 -> text().toFloat()), step); //базис 0 бит 0
            ui ->bQuart_00 -> setText(QString::number(bQuart_00));
            float bHalf_01 = AngleCheck((ui ->bHalf_01 -> text().toFloat()), step); //базис 0 бит 1
            ui ->bHalf_01 -> setText(QString::number(bHalf_01));
            float bQuart_01 = AngleCheck((ui ->bQuart_01 -> text().toFloat()),step); //базис 0 бит 1
            ui ->bQuart_01 -> setText(QString::number(bQuart_01));
            float bHalf_10 = AngleCheck((ui ->bHalf_10 -> text().toFloat()),step); //базис 1 бит 0
            ui ->bHalf_10 -> setText(QString::number(bHalf_10));
            float bQuart_10 = AngleCheck((ui ->bQuart_10 -> text().toFloat()), step); //базис 1 бит 0
            ui ->bQuart_10 -> setText(QString::number(bQuart_10));
            float bHalf_11 = AngleCheck((ui ->bHalf_11 -> text().toFloat()), step); //базис 1 бит 1
            ui ->bHalf_11 -> setText(QString::number(bHalf_11));
            float bQuart_11 = AngleCheck((ui ->bQuart_11 -> text().toFloat()), step); //базис 1 бит 1
            ui ->bQuart_11 -> setText(QString::number(bQuart_11));


            pStreamWork->bHalf_1 = bHalf_11;
            pStreamWork->bHalf_0 = bHalf_00;
            pStreamWork->bQuart_1 = bQuart_11;
            pStreamWork->bQuart_0 = bQuart_00;

            pStreamWork->PV_00 = ui->PV_00 -> text().toInt();
            pStreamWork->PV_01 = ui->PV_01 -> text().toInt();
            pStreamWork->PV_10 = ui->PV_10 -> text().toInt();
            pStreamWork->PV_11 = ui->PV_11 -> text().toInt();

            pStreamWork->PH_00 = ui->PH_00 -> text().toInt();
            pStreamWork->PH_01 = ui->PH_01 -> text().toInt();
            pStreamWork->PH_10 = ui->PH_10 -> text().toInt();
            pStreamWork->PH_11 = ui->PH_11 -> text().toInt();

            QStringList AliceBasis = pStreamWork->ConvertingArray(ui ->AliceBasis -> text());
            QStringList AliceBit = pStreamWork->ConvertingArray(ui->AliceBit -> text());
            QStringList BobBasis = pStreamWork->ConvertingArray(ui ->BobBasis -> text());
            QStringList BobBit = pStreamWork->ConvertingArray(ui ->BobBit -> text());
            pStreamWork->AliceBasis = AliceBasis;
            pStreamWork->AliceBit = AliceBit;
            pStreamWork->BobBasis = BobBasis;
            pStreamWork->BobBit = BobBit;
            pStreamWork->ElectionPD_ = ui->radio_ElectionPD_v2->isDown();

            api::AdcResponse response;
            response = stand_.GetErrorCode();
            if(response.errorCode_ == 0){
                pStreamWork->moveToThread(&pMyThread);
                //Определение алгоритма протокола
                if (ui->Evacheck->isChecked())
                {
                    if((ui ->EvaBasis -> text()) == ""){
                        QMessageBox::critical(this,
                                              "Ошибка!",
                                              "Не все поля заполнены!",
                                              QMessageBox::Ok);
                        ui ->TestLine->setEnabled(true);
                        ui ->Auto_Filing->setEnabled(true);
                    }
                    else{
                        QStringList EvaBasis = pStreamWork->ConvertingArray(ui ->EvaBasis -> text());
                        pStreamWork->EvaBasis = EvaBasis;
                        connect(pStreamWork,SIGNAL(emitdate_eva(float,float,QStringList,QStringList, QStringList,QStringList)),this,SLOT(Date_time_hist(float,float,QStringList,QStringList, QStringList, QStringList))); //время, скорость, гистограммы
                        connect(&pMyThread,SIGNAL(started()),pStreamWork,SLOT(Protocol_Eva())); //выполнение протокола
                        connect(pStreamWork,SIGNAL(emitdate(int, int,QStringList,QStringList, QStringList, QStringList, QStringList, double)),this,SLOT(update(int, int,QStringList,QStringList ,QStringList, QStringList, QStringList, double))); //обновление значений строк

                        connect(pStreamWork,SIGNAL(finished()),this,SLOT(Output_bit()));
                        connect(pStreamWork, &StreamWork::finished, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
                        connect(pStreamWork, SIGNAL(finished()), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
                       // connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(terminate())); // когда закончит работу поток, удаляем и его
                        pMyThread.start();
                    }
                }
                else
                {
                    connect(pStreamWork,SIGNAL(emitdate(float,float,QStringList,QStringList)),this,SLOT(Date_time_hist(float,float,QStringList,QStringList))); //время, скорость, гистограммы
                    connect(pStreamWork,SIGNAL(emitdate(int, int,QStringList, QStringList, QStringList, QStringList, double)),this,SLOT(update(int, int,QStringList, QStringList, QStringList, QStringList, double))); //обновление значений строк
                    connect(&pMyThread,&QThread::started,pStreamWork,&StreamWork::Protocol); //выполнение протокола

                    connect(pStreamWork,SIGNAL(finished()),this,SLOT(Output_bit()));
                    connect(pStreamWork, &StreamWork::finished, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
                    connect(pStreamWork, SIGNAL(finished()), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
                    //connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(terminate())); // когда закончит работу поток, удаляем и его
                    pMyThread.start();
                }
            }
        }
    }

}

/// @brief рандом битовой последовательности
QStringList MainWindow:: Random(int n) {
    QStringList Line;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0,1);
    for (int i = 0; i < n; ++i)
    {
        Line << QString::number(dist(gen));
    }
    return Line;
}

/// @brief заполнение полей рандомной комбинацией согласно заданному значению кол-во бит
void MainWindow::on_Auto_Filing_clicked()
{
    int n = ui ->AutoLine -> text().toInt();

    //Углы пластин Алисы
    QString aHalf_00 = ui ->aHalf_00 -> text(); //базис 0 бит 0
    QString aQuart_00 = ui ->aQuart_00 -> text(); //базис 0 бит 0
    QString aHalf_01 = ui ->aHalf_01 -> text(); //базис 0 бит 1
    QString aQuart_01 = ui ->aQuart_01 -> text(); //базис 0 бит 1
    QString aHalf_10 = ui ->aHalf_10 -> text(); //базис 1 бит 0
    QString aQuart_10 = ui ->aQuart_10 -> text(); //базис 1 бит 0
    QString aHalf_11 = ui ->aHalf_11 -> text(); //базис 1 бит 1
    QString aQuart_11 =ui ->aQuart_11 -> text(); //базис 1 бит 1

    //Углы пластин Bob
    QString bHalf_00 = ui ->bHalf_00 -> text(); //базис 0 бит 0
    QString bQuart_00 = ui ->bQuart_00 -> text(); //базис 0 бит 0
    QString bHalf_01 = ui ->bHalf_01 -> text(); //базис 0 бит 1
    QString bQuart_01 = ui ->bQuart_01 -> text(); //базис 0 бит 1
    QString bHalf_10 = ui ->bHalf_10 -> text(); //базис 1 бит 0
    QString bQuart_10 = ui ->bQuart_10 -> text(); //базис 1 бит 0
    QString bHalf_11 = ui ->bHalf_11 -> text(); //базис 1 бит 1
    QString bQuart_11 = ui ->bQuart_11 -> text(); //базис 1 бит 1

    if(aHalf_00 == "" && aQuart_00 == "" && aHalf_01== "" && aQuart_01== "" && aHalf_10 == "" && aQuart_10== "" && aHalf_11 == "" && aQuart_11 == "" &&
        bHalf_00 == "" && bQuart_00 == "" && bHalf_01 == "" && bQuart_01 == "" && bHalf_10 == "" && bQuart_10 == "" && bHalf_11 == "" && bQuart_11 == ""){

        ui->aHalf_00->setText("0");
        ui->aHalf_01->setText("45");
        ui->aHalf_10->setText("0");
        ui->aHalf_11->setText("0");
        ui->aQuart_00->setText("0");
        ui->aQuart_01->setText("0");
        ui->aQuart_10->setText("45");
        ui->aQuart_11->setText("-45");

        ui->bHalf_00->setText("0");
        ui->bHalf_01->setText("0");
        ui->bHalf_10->setText("0");
        ui->bHalf_11->setText("0");
        ui->bQuart_00->setText("0");
        ui->bQuart_01->setText("0");
        ui->bQuart_10->setText("45");
        ui->bQuart_11->setText("45");

        ui->PH_00->setText("0");
        ui->PH_01->setText("0");
        ui->PH_10->setText("0");
        ui->PH_11->setText("0");

        ui->PV_00->setText("1");
        ui->PV_01->setText("1");
        ui->PV_10->setText("1");
        ui->PV_11->setText("1");
    }

    if (ui->Evacheck->isChecked())
    {
        ui ->AliceBit-> setText(Random(n).join(""));
        ui ->AliceBasis-> setText(Random(n).join(""));
        ui ->BobBit->setText(Random(n).join(""));
        ui ->BobBasis->setText(Random(n).join(""));
        ui ->EvaBasis->setText(Random(n).join(""));
    }
    else
    {
        ui ->AliceBit-> setText(Random(n).join(""));
        ui ->AliceBasis-> setText(Random(n).join(""));
        ui ->BobBit->setText(Random(n).join(""));
        ui ->BobBasis->setText(Random(n).join(""));
    }

}

/// @brief очистка полей во вкладке Протокол
void MainWindow::on_Delete_clicked()
{
    if(protocol == true){on_Stop_protocol_clicked(); Flag_1=true;}

    ui->aHalf_00->clear();
    ui->aHalf_01->clear();
    ui->aHalf_10->clear();
    ui->aHalf_11->clear();
    ui->aQuart_00->clear();
    ui->aQuart_01->clear();
    ui->aQuart_10->clear();
    ui->aQuart_11->clear();

    ui->bHalf_00->clear();
    ui->bHalf_01->clear();
    ui->bHalf_10->clear();
    ui->bHalf_11->clear();
    ui->bQuart_00->clear();
    ui->bQuart_01->clear();
    ui->bQuart_10->clear();
    ui->bQuart_11->clear();

    ui->PH_00->clear();
    ui->PH_01->clear();
    ui->PH_10->clear();
    ui->PH_11->clear();

    ui->PV_00->clear();
    ui->PV_01->clear();
    ui->PV_10->clear();
    ui->PV_11->clear();

    ui ->AliceBit->clear();
    ui ->AliceBasis->clear();
    ui ->BobBit->clear();
    ui ->BobBasis->clear();
    ui ->EvaBit->clear();
    ui ->EvaBasis->clear();

    ui->progressBar->setValue(0);
    ui ->RawLine ->clear();
    ui ->Comparison ->clear();
    ui ->BlankLine ->clear();
    ui ->LengthKey->clear();
    ui ->key -> clear();
    ui ->LengthKey->clear();
    ui ->Time->clear();
    ui ->Speed->clear();
    ui ->error_pr->clear();
    ui ->Progress->clear();
}

/// @brief Заполнение полей тестовой комбинацией
void MainWindow::on_TestLine_clicked()
{
    //Углы пластин Алисы
    QString aHalf_00 = ui ->aHalf_00 -> text(); //базис 0 бит 0
    QString aQuart_00 = ui ->aQuart_00 -> text(); //базис 0 бит 0
    QString aHalf_01 = ui ->aHalf_01 -> text(); //базис 0 бит 1
    QString aQuart_01 = ui ->aQuart_01 -> text(); //базис 0 бит 1
    QString aHalf_10 = ui ->aHalf_10 -> text(); //базис 1 бит 0
    QString aQuart_10 = ui ->aQuart_10 -> text(); //базис 1 бит 0
    QString aHalf_11 = ui ->aHalf_11 -> text(); //базис 1 бит 1
    QString aQuart_11 =ui ->aQuart_11 -> text(); //базис 1 бит 1

    //Углы пластин Bob
    QString bHalf_00 = ui ->bHalf_00 -> text(); //базис 0 бит 0
    QString bQuart_00 = ui ->bQuart_00 -> text(); //базис 0 бит 0
    QString bHalf_01 = ui ->bHalf_01 -> text(); //базис 0 бит 1
    QString bQuart_01 = ui ->bQuart_01 -> text(); //базис 0 бит 1
    QString bHalf_10 = ui ->bHalf_10 -> text(); //базис 1 бит 0
    QString bQuart_10 = ui ->bQuart_10 -> text(); //базис 1 бит 0
    QString bHalf_11 = ui ->bHalf_11 -> text(); //базис 1 бит 1
    QString bQuart_11 = ui ->bQuart_11 -> text(); //базис 1 бит 1

    if(aHalf_00 == "" && aQuart_00 == "" && aHalf_01== "" && aQuart_01== "" && aHalf_10 == "" && aQuart_10== "" && aHalf_11 == "" && aQuart_11 == "" &&
        bHalf_00 == "" && bQuart_00 == "" && bHalf_01 == "" && bQuart_01 == "" && bHalf_10 == "" && bQuart_10 == "" && bHalf_11 == "" && bQuart_11 == ""){

        ui->aHalf_00->setText("0");
        ui->aHalf_01->setText("45");
        ui->aHalf_10->setText("0");
        ui->aHalf_11->setText("0");
        ui->aQuart_00->setText("0");
        ui->aQuart_01->setText("0");
        ui->aQuart_10->setText("45");
        ui->aQuart_11->setText("-45");

        ui->bHalf_00->setText("0");
        ui->bHalf_01->setText("0");
        ui->bHalf_10->setText("0");
        ui->bHalf_11->setText("0");
        ui->bQuart_00->setText("0");
        ui->bQuart_01->setText("0");
        ui->bQuart_10->setText("45");
        ui->bQuart_11->setText("45");

        ui->PH_00->setText("0");
        ui->PH_01->setText("0");
        ui->PH_10->setText("0");
        ui->PH_11->setText("0");

        ui->PV_00->setText("1");
        ui->PV_01->setText("1");
        ui->PV_10->setText("1");
        ui->PV_11->setText("1");
    }
    if (ui->Evacheck->isChecked())
    {
        ui ->AliceBit-> setText("00110011001100110011001100110011");
        ui ->AliceBasis-> setText("00001111000011110000111100001111");
        ui ->BobBit->setText("01010101010101010101010101010101");
        ui ->BobBasis->setText("00001111111100000000111111110000");
        ui ->EvaBasis->setText("00000000000000001111111111111111");
    }
    else {
        ui ->AliceBit-> setText("0011001100110011");
        ui ->AliceBasis-> setText("0000111100001111");
        ui ->BobBit->setText("0101010101010101");
        ui ->BobBasis->setText("0000111111110000");
    }
}

/// @brief поворот пластин на указаные углы во вкладке Мониторинг
void MainWindow::on_RotateAngles_clicked()
{
    if(mutx_str==false){
        api::SendMessageResponse response;
        api::AdcResponse response_1;
        float step = stand_.GetRotateStep().angle_;
        float angles1 = AngleCheck((ui ->Angles1 -> value()), step);
        ui ->Angles1 -> setValue(angles1);
        float angles2 = AngleCheck((ui ->Angles2 -> value()), step);
        ui ->Angles2 -> setValue(angles2);
        float angles3 = AngleCheck((ui ->Angles3 -> value()), step);
        ui ->Angles3 -> setValue(angles3);
        float angles4 = AngleCheck((ui ->Angles4 -> value()), step);
        ui ->Angles4 -> setValue(angles4);
        response_1 = stand_.GetLaserPower();
        if (response_1.errorCode_ == 0){
            int Power = response_1.adcResponse_;
            response  = stand_.Sendmessage({angles1,angles2,angles3,angles4},Power);
            ui ->Cur_PDH -> setText(QString::number(response.currentSignalLevels_.h_));
            ui ->Cur_PDV -> setText(QString::number(response.currentSignalLevels_.v_));
        }
        else {
            ConsoleLog("Команда RotateAngles не выполнена");
            ConsoleLog("Код ошибки: "+ QString::number (response_1.errorCode_), 1);
        }
        ParamAngles();
    }
}

/// @brief Мониторинг посылки
void MainWindow::on_MonitoringSend_clicked()
{
    if(mutx_str==false){
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();
        number =0;

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        ui->graph->xAxis->setLabel("Номер посылки");
        ui->graph->yAxis->setLabel("Уровень сигнала");
        QSharedPointer<QCPAxisTicker> autoTicker(new QCPAxisTicker);
        ui->graph->xAxis->setTicker(autoTicker);

        api::SendMessageResponse response;
        api::AdcResponse response_1;
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;
        int cout = 0;
        float y1_max = 0,y1_ = 0, y1_min =0, n1_;
        float y2_max = 0,y2_ = 0, y2_min =0, n2_;
        //float pdh=0, pdv=0;
        QString PDH_max,PDH_min, PDV_max,PDV_min;
        if(response.errorCode_ == 0){
            while (Flag_ == false)
            {
                response  = stand_.Sendmessage({0,0,0,0},Power);
                //снятие показателей сигналов
                y1_ = response.currentSignalLevels_.h_;
                y2_ = response.currentSignalLevels_.v_;
                //уровень засветки
                n1_ = response.currentLightNoises_.h_;
                n2_ = response.currentLightNoises_.v_;

                //сумма всех показателей без засветки
                //pdh =+(y1_- n1_);
                //pdv =+(y2_- n2_);

                //построение графика без засветки
                x.push_back(cout++);
                y1.push_back(y1_- n1_);
                y2.push_back(y2_- n2_);

                if(y1_ > y1_max){
                    y1_max =y1_;
                }
                if (y1_ < y1_min){
                    y1_min =y1_;
                }

                if(y2_ > y2_max){
                    y2_max =y2_;
                }
                if (y2_ < y2_min){
                    y2_min =y2_;
                }

                PDH_max = QString::number(y1_max);
                PDH_min = QString::number(y1_min);
                PDV_max = QString::number(y2_max);
                PDV_min = QString::number(y2_min);
                ui ->PDH_max -> setText(PDH_max);
                ui ->PDH_min -> setText(PDH_min);
                ui ->PDV_max -> setText(PDV_max);
                ui ->PDV_min -> setText(PDV_min);
                ui ->Nos_PDH -> setText(QString::number (response.currentLightNoises_.h_));
                ui ->Nos_PDV -> setText(QString::number (response.currentLightNoises_.v_));

                if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}

                ui->graph->xAxis->setRange(0 ,cout + 10);
                ui->graph->graph(0)->setData(x,y1);
                ui->graph->graph(1)->setData(x,y2);
                ui->graph->replot();
                QApplication::processEvents();
                connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()));
                connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles1, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles2, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles3, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles4, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);}
    }
}

/// @brief Мониториг уровня засветки
void MainWindow::on_MonitorNoises_clicked()
{
    if(mutx_str==false){
        ui->graph->clearGraphs();
        x.clear();
        y1.clear();
        y2.clear();
        y3.clear();
        number =0;

        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        ui->graph->addGraph();
        ui->graph->graph(2)->setPen(QPen(Qt::green));
        ui->graph->graph(2)->setBrush(QBrush(QColor(0, 255, 0, 20)));

        ui->graph->xAxis->setLabel(" ");
        ui->graph->yAxis->setLabel("Уровень сигнала");
        QSharedPointer<QCPAxisTicker> autoTicker(new QCPAxisTicker);
        ui->graph->xAxis->setTicker(autoTicker);

        QString angles1,angles2,angles3, angles4;

        api::SendMessageResponse response;
        api::AdcResponse response_1;
        response_1 = stand_.GetLaserPower();
        double Power = response_1.adcResponse_;
        Flag_ = false;
        int cout = 0;
        float y1_ = 0, n1_, y2_ = 0, n2_, y1_max = 0, y2_max = 0;
        if(response.errorCode_ == 0){
            while (Flag_ == false)
            {
                angles1 = ui ->Angles1 -> text();
                angles2 = ui ->Angles2 -> text();
                angles3 = ui ->Angles3 -> text();
                angles4 = ui ->Angles4 -> text();
                response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles3.toFloat(),angles4.toFloat()},Power);

                y1_ = response.currentSignalLevels_.h_;
                y2_ = response.currentSignalLevels_.v_;
                n1_ = response.currentLightNoises_.h_;
                n2_ = response.currentLightNoises_.v_;

                if (ui->PD_Check->isChecked()) //выводятся значения PDH
                {
                    x.push_back(cout++);
                    y1.push_back(y1_- n1_);
                    y2.push_back(y1_);
                    y3.push_back(n1_);
                    ui ->Nos_PDH -> setText(QString::number (response.currentLightNoises_.h_));
                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                }
                else // выводятся значения PDV
                {
                    x.push_back(cout++);
                    y1.push_back(y2_- n2_);
                    y2.push_back(y2_);
                    y3.push_back(n2_);
                    ui ->Nos_PDV -> setText(QString::number (response.currentLightNoises_.v_));
                    if(y1_ > y1_max){
                        y1_max =y1_;
                    }
                    if(y2_ > y2_max){
                        y2_max =y2_;
                    }
                }
                if (cout < 20){ui->graph->xAxis->setRange(0 ,20);}
                else{ui->graph->xAxis->setRange(0 ,cout);}

                if (y1_max>y2_max){ui->graph->yAxis->setRange(0 ,y1_max + 10);}
                else{ui->graph->yAxis->setRange(0 ,y2_max + 10);}

                ui->graph->graph(0)->setData(x,y1);
                ui->graph->graph(1)->setData(x,y2);
                ui->graph->graph(2)->setData(x,y3);
                ui->graph->replot();
                QApplication::processEvents();
                connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->LaserTest, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles1, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles2, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles3, SIGNAL( clicked() ), this, SLOT(killLoop()) );
                connect( ui->ScanAngles4, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
            ParamAngles();
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);}
    }
}

/// @brief тест скорости
void MainWindow::on_Test_Speed_2_clicked()
{
    if(mutx_str==false && mutx==false){
        QElapsedTimer timer;
        timer.start();

        api::SendMessageResponse response;
        api::AdcResponse response_2;
        response_2 = stand_.GetLaserPower();
        double Power = response_2.adcResponse_;

        int n = ui ->QuantityTest-> text().toInt(); //кол-во тестов
        if(n<=0){n=1;ui ->QuantityTest-> setText("1");}
        for(int i = 0; i <= n; i++){
            if(i%2 == 0){
                response  = stand_.Sendmessage({0,0,0,0}, Power);
                response  = stand_.Sendmessage({90,90,90,90}, Power);
            }
            else{
                response  = stand_.Sendmessage({90,90,90,90}, Power);
                response  = stand_.Sendmessage({0,0,0,0}, Power);
            }
        }
        float end_time = timer.elapsed();
        float search_time = end_time /1000;//CLOCKS_PER_SEC
        float speed = (float)n/search_time;
        ui ->Test_time->setText(QString::number(search_time) + "с");
        ui ->Test_Speed->setText(QString::number(speed));
    }
}

/// @brief Скрытие строк если Ева не участввует в протоколе
void MainWindow::on_Evacheck_clicked()
{
    if (ui->Evacheck->isChecked()){
        ui ->EvaBasis->setEnabled(true);
        ui ->EvaBit->setEnabled(true);
    }
    else{
        ui ->EvaBasis->setEnabled(false);
        ui ->EvaBit->setEnabled(false);
    }
}

/// @brief Действия после выполнения протокола, запись результатов в файл
void MainWindow::Output_bit_write(){

    this->test_protocol.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writeStream (&this->test_protocol);

    writeStream << ("Алиса базис: " + ui->AliceBasis -> text()+ "\n");
    writeStream << ("Алиса бит: " + ui->AliceBit -> text()+ "\n");
    writeStream << ("Боб базис: " + ui->BobBasis -> text()+ "\n");
    writeStream << ("Боб бит: " + ui->BobBit -> text()+ "\n");

    writeStream << ("Сырая строка полученная Бобом: " + ui->RawLine -> text()+ "\n");
    writeStream << ("Ключ: " + ui->key -> text()+ "\n");
    writeStream << ("Длина ключа: " + ui->LengthKey -> text()+ "\n");
    writeStream << ("Время выполнения: " + ui->Time -> text() + "\n");
    writeStream << ("Скорость: " + ui->Speed -> text()+ "\n");
    writeStream << ("Процент ошибки: " + ui->error_pr -> text()+ "\n");

    //возвращение в начальное положение
    api::WAnglesResponse response;
    response = stand_.SetPlatesAngles({0,0,0,0});
    setUpdatesEnabled(true);
    ParamAngles();
    protocol=false;
}

/// @brief Действия после выполнения протокола с участием Евы, запись результатов в файл
void MainWindow::Output_bit_write_eva() {

    QTextStream writeStream (&this->test_protocol);

    writeStream << ("Ева базис: " + ui->EvaBasis -> text()+ "\n");
    writeStream << ("Ева бит: " + ui->EvaBit -> text()+ "\n");

    writeStream << ("Сырая строка полученная Бобом: " + ui->RawLine -> text()+ "\n");
    writeStream << ("Ключ: " + ui->key -> text()+ "\n");
    writeStream << ("Длина ключа: " + ui->LengthKey -> text()+ "\n");
    writeStream << ("Время выполнения: " + ui->Time -> text() + "\n");
    writeStream << ("Скорость: " + ui->Speed -> text()+ "\n");
    writeStream << ("Процент ошибки: " + ui->error_pr -> text()+ "\n");

    //возвращение в начальное положение
    api::WAnglesResponse response;
    response = stand_.SetPlatesAngles({0,0,0,0});
    setUpdatesEnabled(true);
    ParamAngles();
    ui ->TestLine->setEnabled(true);
    ui ->Auto_Filing->setEnabled(true);
    protocol=false;
    this->test_protocol.close();
}

/// @brief Тестовое выполнение протокола (по умолчанию кол-во битов равно 1024)
/// Основные поля заполняються рандомными битовыми последовательностями определенной длины (по умолчанию кол-во битов равно 1024)
/// Далее выполняется протокол и результат записывается в текстовый файл
/// После с этими же комбинациями выполняется протокол с Евой, и также записываются результаты в тот же файл
/// Кол-во выполнения пары протоколов по умолчанию 1, иначе введенное число
void MainWindow::on_Protocol_test_clicked()
{
    if(mutx_str ==false){
        ui->aHalf_00->setText("0");
        ui->aHalf_01->setText("45");
        ui->aHalf_10->setText("0");
        ui->aHalf_11->setText("0");
        ui->aQuart_00->setText("0");
        ui->aQuart_01->setText("0");
        ui->aQuart_10->setText("45");
        ui->aQuart_11->setText("-45");

        ui->bHalf_00->setText("0");
        ui->bHalf_01->setText("0");
        ui->bHalf_10->setText("0");
        ui->bHalf_11->setText("0");
        ui->bQuart_00->setText("0");
        ui->bQuart_01->setText("0");
        ui->bQuart_10->setText("45");
        ui->bQuart_11->setText("45");

        ui->PH_00->setText("0");
        ui->PH_01->setText("0");
        ui->PH_10->setText("0");
        ui->PH_11->setText("0");
        ui->PV_00->setText("1");
        ui->PV_01->setText("1");
        ui->PV_10->setText("1");
        ui->PV_11->setText("1");

        mutx_str=true;
        protocol = true;
        Flag_1 = false;

        pStreamWork = new StreamWork(&stand_);
        pStreamWork->moveToThread(&pMyThread);

        ui ->TestLine->setEnabled(false);
        ui ->Auto_Filing->setEnabled(false);

        int m = ui ->Quantity_test -> text().toInt(); //количество раз выполнение протокола
        if(ui ->Quantity_test -> text() == " "){m=1;}
        int n =ui ->AutoLine -> text().toInt(); //длина передоваемой комбинации
        if(ui ->AutoLine -> text() == " "){n=1024;}

        for(int i = 1 ;i <= m; i++){
            ui ->AliceBit-> setText(Random(n).join(""));
            ui ->AliceBasis-> setText(Random(n).join(""));
            ui ->BobBit->setText(Random(n).join(""));
            ui ->BobBasis->setText(Random(n).join(""));
            ui ->EvaBasis->setText(Random(n).join(""));

            //выполнения протокола без Евы
            connect(pStreamWork,SIGNAL(emitdate(float,float,QStringList,QStringList)),this,SLOT(Date_time_hist(float,float,QStringList,QStringList))); //время, скорость, гистограммы
            connect(pStreamWork,SIGNAL(emitdate(int, int,QStringList, QStringList, QStringList, QStringList, double)),this,SLOT(update(int, int,QStringList, QStringList, QStringList, QStringList, double))); //обновление значений строк
            connect(&pMyThread,&QThread::started,pStreamWork,&StreamWork::Protocol); //выполнение протокола

            connect(pStreamWork,SIGNAL(finished()),this,SLOT(Output_bit_write()));
            connect(pStreamWork, &StreamWork::finished, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
            connect(pStreamWork, SIGNAL(finished()), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
            connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(terminate())); // когда закончит работу поток, удаляем и его
            pMyThread.start();

            //Выполнения протокола с Евой
            connect(pStreamWork,SIGNAL(emitdate_eva(float,float,QStringList,QStringList, QStringList,QStringList)),this,SLOT(Date_time_hist(float,float,QStringList,QStringList, QStringList, QStringList))); //время, скорость, гистограммы
            connect(&pMyThread,SIGNAL(started()),pStreamWork,SLOT(Protocol_Eva())); //выполнение протокола
            connect(pStreamWork,SIGNAL(emitdate(int, int,QStringList,QStringList, QStringList, QStringList, QStringList, double)),this,SLOT(update(int, int,QStringList,QStringList ,QStringList, QStringList, QStringList, double))); //обновление значений строк

            connect(pStreamWork,SIGNAL(finished()),this,SLOT(Output_bit_write_eva()));
            connect(pStreamWork, &StreamWork::finished, &pMyThread, &QThread::quit); //отправляем команду на завершение потока
            connect(pStreamWork, SIGNAL(finished()), pStreamWork, SLOT(deleteLater())); // удаляем экземпляр обработчика
            connect(&pMyThread, SIGNAL(finished()), &pMyThread, SLOT(terminate())); // когда закончит работу поток, удаляем и его
            pMyThread.start();
        }
    }

}

/// @brief Окно с информацией
void MainWindow::on_Help_clicked()
{
    clc = new HelpWindow(this);
    clc->show();
}

/// @brief поворот одной пластины
void MainWindow::on_PlateAngle_clicked()
{
    api::AngleResponse response;
    QString angles1 = ui ->InitAngles1 -> text();
    QString angles2 = ui ->InitAngles2 -> text();
    QString angles3 = ui ->InitAngles3 -> text();
    QString angles4 = ui ->InitAngles4 -> text();

    int num = ui ->NumberAngle-> text().toInt();
    if(num == 1){response = stand_.SetPlateAngle(num,angles1.toFloat());}
    else if (num == 2){response = stand_.SetPlateAngle(num,angles2.toFloat());}
    else if (num == 3){response = stand_.SetPlateAngle(num,angles3.toFloat());}
    else if (num == 4){response = stand_.SetPlateAngle(num,angles4.toFloat());}
    else{ConsoleLog("Некорректные данные");}

    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда SetPlatesAngles");
        ConsoleLog("Установленные значения:");
        ConsoleLog("Угол поворота пластины " + QString::number(num) + " на " +QString::number (response.angle_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {
        ConsoleLog("Команда SetPlatesAngles не выполнена");
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_), 1);
    }
}

/// @brief эксперимент с Евой, для записи параметров в файл
void MainWindow::on_Eva_key_clicked()
{
    QStringList EvaBasis = pStreamWork->ConvertingArray(ui ->EvaBasis -> text());
    QStringList EvaBit = pStreamWork->ConvertingArray(ui ->EvaBit -> text());
    QStringList AliceBit = pStreamWork->ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = pStreamWork->ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = pStreamWork->ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = pStreamWork->ConvertingArray(ui ->BobBasis -> text());

    QFile fileOut("./Bit_comp.txt");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writeStream (&fileOut);

    writeStream << ("AliceBit: " + AliceBit.join("")+ "\n");
    writeStream << ("EvaBit: " + EvaBit.join("")+ "\n");
    writeStream << ("BobBit: " + BobBit.join("")+ "\n");

    QStringList Combit,CombitA, CombitE ;
    QStringList blankbit,blankbitE, blankbitAB,blankbitE2 ;
    QStringList keybit, keybitE;

    //Сравнение базисов Eva
    for(int j = 0; j<= EvaBit.size()-1;j++){
        if(AliceBasis[j] == EvaBasis[j]){
            CombitE << EvaBit[j];
        }
        else {CombitE << "X";}
    }

    //Сравнение базисов Bob
    for(int j = 0; j<= BobBit.size()-1;j++){
        if(AliceBasis[j] == BobBasis[j]){
            Combit << BobBit[j];
            CombitA<< AliceBit[j];
        }
        else {Combit << "X"; CombitA<<"X";}
    }

    //Сравнение битов Bob
    for(int j = 0; j<= Combit.size()-1;j++){
        if(CombitA[j] == Combit[j]){
            blankbit << Combit[j];
        }
        else{blankbit << "X";}
    }

    //Сравнение битов Eva
    for(int j = 0; j<= CombitE.size()-1;j++){
        if(CombitA[j] == CombitE[j]){
            blankbitE << CombitE[j];
        }
        else{blankbitE << "X";}
    }

    writeStream << ("Сравнение битов Bob: " + blankbit.join("")+ "\n");
    writeStream << ("Сравнение битов Eva: " + blankbitE.join("")+ "\n");

    //чистая строка
    for(int a = 0; a<= blankbit.size()-1;a++){
        if(blankbit[a] == '1' || blankbit[a] == '0'){
            blankbitAB << blankbit[a];
        }
        else{blankbitAB << " ";}
    }

    //Вывод ключа
    for(int v = 0; v<= blankbitAB.size()-1;v++){
        if(blankbitAB[v] == '1' || blankbitAB[v] == '0'){
            keybit << blankbitAB[v];
        }
        else{keybit << "";}
    }

    writeStream << ("Ключ Alisa-Bob: " + keybit.join("")+ "\n");
    writeStream << ("LengthKey: " + QString::number(keybit.join("").size())+ "\n");

    //чистая строка
    for(int a = 0; a<= blankbitE.size()-1;a++){
        if(blankbitE[a] == '1' || blankbitE[a] == '0'){
            blankbitE2 << blankbitE[a];
        }
        else{blankbitE2 << " ";}
    }

    //Вывод ключа
    for(int v = 0; v<= blankbitE2.size()-1;v++){
        if(blankbitE2[v] == '1' || blankbitE2[v] == '0'){
            keybitE << blankbitE2[v];
        }
        else{keybitE << "";}
    }
    writeStream << ("Ключ Eva: " + keybitE.join("")+ "\n");
    writeStream << ("LengthKey: " + QString::number(keybitE.join("").size())+ "\n");
}

/// @brief Подсчет процента ошибок
/// Вводим битовые последовательности Алиса-бит, Алиса-базис, Боб-бит(значения полученные в результате эксперимента), Боб-базиса
/// В результате получаем процент ошибок основываясь на введенных комбинации
void MainWindow::on_Error_key_clicked()
{
    QStringList AliceBit = pStreamWork->ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = pStreamWork->ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = pStreamWork->ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = pStreamWork->ConvertingArray(ui ->BobBasis -> text()); //сырая строка

    QStringList Combit,CombitA;
    QStringList blankbit,blankbitAB;
    QStringList keybit, keybitE;
    double error;

    //Сравнение базисов Bob
    for(int j = 0; j<= BobBit.size()-1;j++){
        if(AliceBasis[j] == BobBasis[j]){
            Combit << BobBit[j];
            CombitA<< AliceBit[j];
        }
        else {Combit << "X"; CombitA<<"X";}
    }
    //чистая строка
    for(int a = 0; a<= Combit.size()-1;a++){
        if(Combit[a] == '1' || Combit[a] == '0'){
            blankbit << Combit[a];
        }
        else{blankbit << " ";}
    }

    //Вывод ключа
    for(int v = 0; v<= blankbit.size()-1;v++){
        if(blankbit[v] == '1' || blankbit[v] == '0'){
            keybit << blankbit[v];
        }
        else{keybit << "";}
    }

    //Сравнение битов Bob
    for(int j = 0; j<= Combit.size()-1;j++){
        if(CombitA[j] == Combit[j]){
            blankbitAB << Combit[j];
        }
        else{blankbitAB << "X";}
    }

    //Вывод ключа
    for(int v = 0; v<= blankbitAB.size()-1;v++){
        if(blankbitAB[v] == '1' || blankbitAB[v] == '0'){
            keybitE << blankbitAB[v];
        }
        else{keybitE << "";}
    }

    error = ((double(keybit.join("").size())-double(keybitE.join("").size()))/double(keybit.join("").size()))*100;
    ui ->error_pr -> setText(QString::number(error)+ "%");
    ui ->Progress->setText(QString::number(keybit.join("").size()));
    ui ->LengthKey->setText(QString::number(keybitE.join("").size()));
}

/// @brief снятие показателей и постоение графика по нажатию кнопки
void MainWindow::on_Test_monitor_clicked()
{
    if(mutx_str==false){
        if(number == 0){
            ui->graph->clearGraphs();
            x.clear();
            y1.clear();
            y2.clear();
            ui->max_PDH->clear();
            ui->min_PDH->clear();
            ui->max_PDV->clear();
            ui->min_PDV->clear();
        }

        //отрисовка графика
        ui->graph->yAxis->setRange(0,3000);
        ui->graph->addGraph();
        ui->graph->graph(0)->setPen(QPen(Qt::blue));
        ui->graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

        ui->graph->addGraph();
        ui->graph->graph(1)->setPen(QPen(Qt::red));
        ui->graph->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

        ui->graph->xAxis->setLabel("Кол-во снятых показателей");
        ui->graph->yAxis->setLabel("Уровень сигнала");

        api::SLevelsResponse response;
        response = stand_.GetSignalLevels();
        x.push_back(number++);
        y1.push_back(response.signal_.h_);
        y2.push_back(response.signal_.v_);

        ui ->Cur_PDH -> setText(QString::number(response.signal_.h_));
        ui ->Cur_PDV -> setText(QString::number(response.signal_.v_));

        ui->graph->xAxis->setRange(0 ,number + 10);
        ui->graph->yAxis->setRange(0 ,17500);
        ui->graph->graph(0)->setData(x,y1);
        ui->graph->graph(1)->setData(x,y2);
        ui->graph->replot();
    }
}

/// @brief тестирование времени работы
void MainWindow::on_TestTimebut_clicked()
{
    QElapsedTimer timer;
    timer.start();

    api::AngleResponse response;
    response = stand_.GetRotateStep(); //запрос шага

    float end_time = timer.elapsed();
    float search_time = end_time /1000;//CLOCKS_PER_SEC

    ui ->Test_time->setText(QString::number(search_time) + "с");
}

/// @brief установка пароля
void MainWindow::on_CreateConfigSecret_clicked()
{
    QString password = ui ->password-> text();
    api::AdcResponse response;
    response = stand_.CreateConfigSecret(password.toStdString());
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
    ui ->Console_2 -> append("Успешность выполнения: " + QString::number (response.adcResponse_));
}

/// @brief переход в режим API
void MainWindow::on_OpenConfigMode_clicked()
{
    api::AdcResponse response;
    QString password = ui ->password-> text();
    response = stand_.OpenConfigMode(password.toStdString());
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
    if(response.errorCode_==0){
        ui ->Console_2 -> append("Успешность выполнения: " + QString::number (response.adcResponse_));
        if(response.adcResponse_==1){ ui ->Console_2 -> append("Вход в режим API");}
        else{ ui ->Console_2 -> append("Не верный пароль!!!");}}
}

/// @brief выход из режима API
void MainWindow::on_CloseConfigMode_clicked()
{
    stand_.CloseConfigMode();
    ui ->Console_2 -> append("Выход из режима API");
    ui ->Console_2 ->clear();
    ui ->password->clear();
    ui->tabWidget->setTabEnabled(3,false);
    ui->radio_admin->setChecked(false);

}

/// @brief Возращает версию протокола
void MainWindow::on_GetProtocolVersion_clicked()
{
    api::versionProtocolResponse response;
    response=stand_.GetProtocolVersion ();
    ui ->Console_2 -> append("Версия протокола: " + QString::number (response.version_) + "." + QString::number (response.subversion_));
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
}

/// @brief Возвращает версию прошивки
void MainWindow::on_GetProtocolFirmwareVersion_clicked()
{
    api::versionFirmwareResponse response;
    response = stand_.GetCurrentFirmwareVersion();
    ui ->Console_2 -> append("Версия прошивки: " + QString::number (response.major_) + "." + QString::number (response.minor_)+ "." +QString::number (response.micro_));
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
}

/// @brief Возращает текущий режим работы пользоваетля
void MainWindow::on_GetCurrentMode_clicked()
{
    if(QString::number(stand_.GetCurrentMode()) == "1"){
        ConsoleLog("Режим работы: 1 - технологический");
    }
    if(QString::number(stand_.GetCurrentMode()) == "0"){
        ConsoleLog("Режим работы: 0 - штатный");
    }

}

/// @brief Установка порта
void MainWindow::on_SetComPortName_clicked()
{
    QString port = ui ->Port-> text();//наименование порта
    if(port == " "){
        ui ->Console_2 -> append("Команда SetComPortName не выполнена");
        ui ->Console_2 -> append("Не указан порт");
    }
    else {
        stand_.SetComPortName(port.toStdString().c_str());
        ui ->Console_2 -> append("Порт установлен");
    }
}

/// @brief Название порта, к которому подключен стенд
void MainWindow::on_GetComPortName_clicked()
{
    stand_.GetComPortName();
    ui ->Console_2 -> append("Порт:" + QString::fromStdString(stand_.GetComPortName()));
}

/// @brief Функция получения максимального количества передаваемых байтов
void MainWindow::on_GetMaxPayloadSize_clicked()
{
    ui ->Console_2 -> append("Количество байт:" + QString::number(stand_.GetMaxPayloadSize()));
}

/// @brief Перепрошивка стенда
void MainWindow::on_FirmwareUpdate_clicked()
{
    QString file = ui ->File_path -> text();
    api::AdcResponse response;
    if(file==""){
        ui ->Console_2 -> append("Прошивка стенда не выполена");
    }
    else{
        response=stand_.FirmwareUpdate(file.toStdString());
        if(response.errorCode_ == 0){
            ui ->Console_2 -> append("Прошивка стенда завершена");
            ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
        }
        else{
            ui ->Console_2 -> append("Прошивка стенда не выполена");
            ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
        }

    }
}

/// @brief Переход в режим администратора
void MainWindow::on_radio_admin_clicked()
{
    if(mutx_str==false){
        // Инициализируем окно Adminlogin
        adm = new Adminlogin();
        adm->Adminlogin_init(&stand_);
        if (ui->radio_admin->isChecked()){
            adm->show();
            connect(adm, &Adminlogin::firstWindow, this, &MainWindow::Flag_admin);
        }
        else{ui->tabWidget->setTabEnabled(3,false);}
    }
}
void MainWindow::Flag_admin(){
    ui->tabWidget->setTabEnabled(3,true);
}

/// @brief Открытие окна с гистограммами
void MainWindow::on_histogram_clicked()
{
    if (ui->Evacheck->isChecked()){ eva.show();}
    else{hst.show();}
}

void MainWindow::on_radio_ElectionPD_v2_clicked()
{
    ui->radio_ElectionPD->setDown(false);
}

void MainWindow::on_timer_check_clicked()
{
    if(ui->timer_check->isChecked()){
        ui ->Timer_->setEnabled(true);
    }
    else{ ui ->Timer_->setEnabled(false);}
}

void MainWindow::on_Stop_protocol_clicked()
{
    if( protocol == true){
        pStreamWork->killLoop();
    }
}


void MainWindow::on_file_selection_clicked()
{
    QString file = QFileDialog::getOpenFileName();
    ui ->File_path-> setText(file);
}

void MainWindow::on_WriteEEPROM_clicked()
{
    api::AdcResponse response;
    QString Address = ui ->Address-> text();
    QString Value = ui ->Value-> text();
    if(Address == "" || Address == " "){
        ui ->Console_2 -> append("Команда WriteEEPROM не выполнена");
        ui ->Console_2 -> append("Не указан адрес смещения в ПЗУ МК");
    }
    else if (Value == "" || Value == " "){
        ui ->Console_2 -> append("Команда WriteEEPROM не выполнена");
        ui ->Console_2 -> append("Не указано значение для записи");
    }
    else {
        response = stand_.WriteEEPROM(Address.toUInt(), Value.toUInt());
        if (response.errorCode_ == 0){
            ui ->Console_2 -> append("Выполнена команда WriteEEPROM");
            ui ->Console_2 -> append("Установленные значения:");
            ui ->Console_2 -> append("Значение в ячейке: "+ QString::number (response.adcResponse_));
            ui ->Console_2 -> append("Код ошибки: "+ QString::number (response.errorCode_));
        }
        else {
            ui ->Console_2 -> append("Команда WriteEEPROM не выполнена");
            ui ->Console_2 -> append("Код ошибки: "+ QString::number (response.errorCode_));
        }
    }
}

void MainWindow::on_ReadEEPROM_clicked()
{
    api::AdcResponse response;
    QString Address = ui ->Address-> text();
    if(Address == "" || Address == " "){
        ui ->Console_2 -> append("Команда ReadEEPROM не выполнена");
        ui ->Console_2 -> append("Не указан адрес смещения в ПЗУ МК");
    }
    else {
        response = stand_.ReadEEPROM(Address.toUInt());
        if (response.errorCode_ == 0){
            ui ->Console_2 -> append("Выполнена команда ReadEEPROM");
            ui ->Console_2 -> append("Полученные значения:");
            ui ->Console_2 -> append("Значение в ячейке: "+ QString::number (response.adcResponse_));
            ui ->Console_2 -> append("Код ошибки: "+ QString::number (response.errorCode_));
        }
        else {
            ui ->Console_2 -> append("Команда ReadEEPROM не выполнена");
            ui ->Console_2 -> append("Код ошибки: "+ QString::number (response.errorCode_));
        }
    }
}

