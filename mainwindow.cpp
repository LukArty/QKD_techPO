#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <QSettings>
#include <QVariant>
#include <QFile>
#include <ctime>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <conserial.h>
#include <stdlib.h>
#include <helpwindow.h>
#include <random>
using namespace std;


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
{
    //Интерфейс
ui->setupUi(this);
ui->InitByPD->setStyleSheet("background-color: #7CFC00"); //цвет кнопки "Иниц. по Фд"
ui->tabWidget->setTabEnabled(3,false);
ui ->Interval -> setText("180"); //интервал по-умолчанию
ui->progressBar->setValue(0);

ParamAngles();

ui->widget->xAxis->setRange(0,180);
ui->widget->yAxis->setRange(0,3000);
ui ->EvaBasis->setEnabled(false);
ui ->EvaBit->setEnabled(false);
ui->radio_ElectionPD->setDown(true);
//скрытие кнопок
ui->Error_key->hide();
ui->Eva_key->hide();
ui->Protocol_test->hide();
ui->Test_monitor->hide();
}

MainWindow::~MainWindow()
{
    //после закрытия приложения
    stand_.SetLaserPower(0); // установка мощности лазера в 0
    stand_.SetLaserState(0); //перевод лазер в состояние вкл

    delete ui;
}

void MainWindow::ConsoleLog(QString text)
{
ui->CommandConsole->append(text);
//ui->CommandConsole->scroll()
}

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


void MainWindow::on_InitBut_clicked() //инициализация по концовикам
{
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

        ConsoleLog("Выполнена команда Init.");
        if (response.errorCode_ == 0){
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
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
    }
    else{
        api::InitResponse response;
        response = stand_.InitByPD();
        ConsoleLog("Выполнена команда Init.");
        if (response.errorCode_ == 0){
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
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
    }
}

void MainWindow::on_RunSelfTestBut_clicked()
{

}

void MainWindow::on_GetErrorCodeBut_clicked() //определения кода ошибки
{
    api::AdcResponse response;
    response = stand_.GetErrorCode();
    ConsoleLog("Выполнена команда GetErrorCode");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Значение: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetLaserStateBut_clicked() //определения состояния лазера
{
    api::AdcResponse response;
    response = stand_.GetLaserState();
    ConsoleLog("Выполнена команда GetLaserState");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        if(response.adcResponse_ == 1){ui->laser_st->setText("ВКЛ");}
        else{ ui->laser_st->setText("ВЫКЛ");}
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_SetLaserStateBut_clicked() //установка значения лазера
{
    api::AdcResponse response;
    if (ui->LaserState->isChecked())
    {
        response = stand_.SetLaserState(1);
        ui->laser_st->setText("ВКЛ");
    }
    else
    {
        response = stand_.SetLaserState(0);
        ui->laser_st->setText("ВЫКЛ");
    }
    ConsoleLog("Выполнена команда SetLaserState");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_SetLaserPowerBut_clicked() //установка мощности лазера
{
    api::AdcResponse response;
    QString value_ = ui ->LaserPowerValue -> text();
    response = stand_.SetLaserPower(value_.toUInt());
    ConsoleLog("Выполнена команда SetLaserPower");
    if (response.errorCode_ == 0){
        ui->laser_pw->setText(QString::number (response.adcResponse_));
        ConsoleLog("Установленны значения:");
        ConsoleLog("Значение: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetLaserPowerBut_clicked() //определения мощности лазера
{
    api::AdcResponse response;
    response = stand_.GetLaserPower();
    ConsoleLog("Выполнена команда GetLaserPower");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Мощность лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ui->laser_pw->setText(QString::number (response.adcResponse_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetRotateStepBut_clicked() //определения значения минимального шага двигателя
{
    api::AngleResponse response;
    response = stand_.GetRotateStep();
    ConsoleLog("Выполнена команда GetRotateStep");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Значение шага: "+ QString::number (response.angle_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_SetPlateAngleBut_clicked()//поворот пластин
{
        api::WAnglesResponse response;
        QString angles1 = ui ->InitAngles1 -> text();
        QString angles2 = ui ->InitAngles2 -> text();
        QString angles3 = ui ->InitAngles3 -> text();
        QString angles4 = ui ->InitAngles4 -> text();
        response = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
        ConsoleLog("Выполнена команда SetPlatesAngles");
        if (response.errorCode_ == 0){
            ConsoleLog("Установленны значения:");
            ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
            ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
            ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
            ParamAngles();
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetCurPlatesAnglesBut_clicked()//текущие углы пластин
{
    api::WAnglesResponse response;
    response = stand_.GetPlatesAngles();
    ConsoleLog("Выполнена команда GetPlatesAngles");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_SetTimeoutBut_clicked()//установка значения таймаута
{
    api::AdcResponse response;
    QString value_ = ui -> TimeoutValue-> text();
    response = stand_.SetTimeout(value_.toUInt());
    ConsoleLog("Выполнена команда SetTimeout");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
        ConsoleLog("Значение таймаута: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetTimeoutBut_clicked()//текущее значение таймаута
{
    api::AdcResponse response;
    response = stand_.GetTimeout();
    ConsoleLog("Выполнена команда GetTimeout");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
        ConsoleLog("Значение таймаута: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetSignalLevelBut_clicked()//снятие показателей с ФД
{
    api::SLevelsResponse response;
    response = stand_.GetSignalLevels();
    ConsoleLog("Выполнена команда GetSignalLevel");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.signal_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.signal_.v_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_GetLightNoisesBut_clicked() //снятие уровня засветки
{
    api::SLevelsResponse response;
    response = stand_.GetLightNoises();
    ConsoleLog("Выполнена команда GetLightNoises");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
        ConsoleLog("Уровень засветки на первом фотодетекторе: "+ QString::number (response.signal_.h_));
        ConsoleLog("Уровень засветки на втором фотодетекторе: "+ QString::number (response.signal_.v_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_SendMessageBut_clicked() //отправка посылки
{
    api::SendMessageResponse response;
    response  = stand_.Sendmessage({0,0,0,0},60);
    ConsoleLog("Выполнена команда SendMessage");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
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
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_WriteIni_clicked() //запись углов ini-файл
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

void MainWindow::on_InitByButtons_clicked() //инициализация по датчикам
{
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

    ConsoleLog("Выполнена команда InitByButtons");
    if(response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
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
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_ReadIni_clicked() //считывание углов из ini-файла
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

void MainWindow::on_InitByPD_clicked() //инициализация по ФД
{
    api::InitResponse response;
    api::AdcResponse response_1;
    response = stand_.InitByPD();

    ConsoleLog("Выполнена команда InitByPD");
    if(response.errorCode_ == 0){

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

            response_1 = stand_.GetLaserPower();
            ConsoleLog("Мощность лазера: "+ QString::number (response_1.adcResponse_));
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
            ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_PulseLaser_clicked()//импульсный режим
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    Flag_ = false; // флаг для остваноки
    api::AdcResponse response;
    api::SLevelsResponse response1;
    int time = 0;
    int y1_max = 0,y1_ = 0, y1_min = 1000000;
    int y2_max = 0,y2_ = 0, y2_min = 1000000;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
        if(response.errorCode_ == 0){
        while (Flag_ == false)
        {
            // вкл/выкл лазера
            if(time%2){response = stand_.SetLaserState(1); QThread::msleep(200);}
            else{response = stand_.SetLaserState(0);}
            QThread::msleep(200);
            response1 = stand_.GetSignalLevels();
            x.push_back(time++);
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

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 1);}
            ui->widget->xAxis->setRange(0 ,time + 10);
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();

            //остановка цикла по кнопке
            QApplication::processEvents();
            connect( ui->StopLaser, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            connect( ui->StopLaser2, SIGNAL( clicked() ), this, SLOT(killLoop()) );
        }
        response = stand_.SetLaserState(1);
        }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_StopLaser_clicked()
{

}

void MainWindow::on_ScanAngles1_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика
    int interval = ui ->Interval-> text().toInt();
    ui->widget->xAxis->setRange(0,interval);
    ui->widget->yAxis->setRange(0,3000);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    api::SendMessageResponse response;
    api::AdcResponse response_1;
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
    QFile fileOut("./test1.txt");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writeStream (&fileOut);

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

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();

            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
        }
    api::SLevelsResponse response_3;
    stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
    response_3 = stand_.GetSignalLevels();
    ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
    ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
    ParamAngles();
}

void MainWindow::on_ScanAngles2_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    int interval = ui ->Interval-> text().toInt();
    ui->widget->xAxis->setRange(0,interval);
    ui->widget->yAxis->setRange(0,3000);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

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

    double h = (ui ->Step_scan-> value());//значение шага
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;
    Flag_ = false;

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
    QString PDH_max,PDH_min, PDV_max,PDV_min;

        QFile fileOut("./test2.txt");
        fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeStream (&fileOut);

        for (float angles=0; angles <=interval;angles +=h)
        {

            if( Flag_ == false){
            response  = stand_.Sendmessage({angles1,angles,angles3,angles4},Power);
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

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();
            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
    }
    api::SLevelsResponse response_3;
    stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
    response_3 = stand_.GetSignalLevels();
    ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
    ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
    ParamAngles();
}

void MainWindow::on_ScanAngles3_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    int interval = ui ->Interval-> text().toInt();
    ui->widget->xAxis->setRange(0,interval);
    ui->widget->yAxis->setRange(0,3000);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));


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


    double h = (ui ->Step_scan-> value());//значение шага
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;
    Flag_ = false;

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
    QString PDH_max,PDH_min, PDV_max,PDV_min;

    QFile fileOut("./test3.txt");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writeStream (&fileOut);

        for (float angles=0; angles <= interval;angles +=h)
        {
            if( Flag_ == false){
            response  = stand_.Sendmessage({angles1,angles2,angles,angles4},Power);
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

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();
            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
            }
    }
    api::SLevelsResponse response_3;
    stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
    response_3 = stand_.GetSignalLevels();
    ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
    ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
    ParamAngles();
}

void MainWindow::on_ScanAngles4_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    int interval = ui ->Interval-> text().toInt();
    ui->widget->xAxis->setRange(0,interval);
    ui->widget->yAxis->setRange(0,3000);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

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


    double h = (ui ->Step_scan-> value());//значение шага
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;
    Flag_ = false;

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;

    QFile fileOut("./test4.txt");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream writeStream (&fileOut);

        for (float angles = 0; angles <= interval;angles +=h)
         {
            if(Flag_ == false){
            response  = stand_.Sendmessage({angles1,angles2,angles3,angles},Power);
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
            ui ->PDH_max -> setText(QString::number(y1_max));
            ui ->PDH_min -> setText(QString::number(y1_min));
            ui ->PDV_max -> setText(QString::number(y2_max));
            ui ->PDV_min -> setText(QString::number(y2_min));
            ui ->Cur_PDH -> setText(QString::number(y1_));
            ui ->Cur_PDV -> setText(QString::number(y2_));

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();
            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );}
            else{break;}
        }
    api::SLevelsResponse response_3;
    stand_.SetPlatesAngles({angles1, angles2, angles3, angles4});
    response_3 = stand_.GetSignalLevels();
    ui ->Cur_PDH -> setText(QString::number(response_3.signal_.h_));
    ui ->Cur_PDV -> setText(QString::number(response_3.signal_.v_));
    ParamAngles();
}

void MainWindow::on_MonitoringPD_clicked()
{
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    ui->timer->setText(QTime::currentTime().toString("hh:mm:ss"));
    api::SLevelsResponse response;
    api::AdcResponse response_1;
    //double Power;
    api::AdcResponse response_2;
    Flag_ = false;
    int time = 0;
    float y1_max = 0,y1_ = 0, y1_min =100000, pdh=0;
    float y2_max = 0,y2_ = 0, y2_min =100000, pdv=0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    if(response.errorCode_ == 0){
    while (Flag_ == false)
    {
        response = stand_.GetSignalLevels();
        x.push_back(time++);
        y1_ = response.signal_.h_;
        y2_ = response.signal_.v_;
        y1.push_back(y1_);
        y2.push_back(y2_);

        pdh=pdh+response.signal_.h_;;
        pdv=pdv+response.signal_.v_;

        response_1 = stand_.GetLaserPower();
        //Power = response_1.adcResponse_;

        /*if (((y1_*100)/y1_max) > 101 && y1_max !=0 ) {
            response_2 = stand_.SetLaserPower(Power-1);
            ConsoleLog(QString::number(Power-1));
        }
        else if (((y2_*100)/y2_max)> 101 && y2_max !=0) {
            response_2 = stand_.SetLaserPower(Power-1);
            ConsoleLog(QString::number(Power-1));
        }
        else if(((y1_*100)/y1_min) < 99 && Power < 100 && y1_min !=100000){
            response_2 = stand_.SetLaserPower(Power+1);
            ConsoleLog(QString::number(Power+1));
        }
        else if(((y2_*100)/y2_min)< 99 && Power < 100 && y2_min !=100000){
            response_2 = stand_.SetLaserPower(Power+1);
            ConsoleLog(QString::number(Power+1));
        }*/

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

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}

        ui->widget->xAxis->setRange(0 ,time + 10);
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
        QApplication::processEvents();
        connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );

        timer = new QTimer();
        timer->setInterval(10000);
        connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
        timer->start();
    }
    ConsoleLog("Среднее знаение для PDH:" + QString::number (pdh/time));
    ConsoleLog("Отклонение от максимума PDH: " + QString::number (((y1_max-(pdh/time))*100)/(pdh/time))+ " %");
    ConsoleLog("Отклонение от минимума PDH: " + QString::number ((((pdh/time)-y1_min)*100)/(pdh/time))+ " %");
    ConsoleLog("Среднее знаение для PDV:" + QString::number (pdv/time));
    ConsoleLog("Отклонение от максимума PDV: " + QString::number (((y2_max-(pdv/time))*100)/(pdv/time))+ " %");
    ConsoleLog("Отклонение от минимума PDV: " + QString::number ((((pdv/time)-y2_min)*100)/(pdv/time))+ " %");
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}

}

void MainWindow::on_GetInitParams_clicked() //параметры после инициализации
{
   api::InitResponse response;
    response = stand_.GetInitParams();

    ConsoleLog("Выполнена команда GetInitParams:");
    if (response.errorCode_ == 0){
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
        ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_PDRatio_clicked()
{
    ConsoleLog("Корректность распределений мощностей в поляризационном кубике:");
    int qua = ui ->QuantityTest -> text().toInt();//количество тестов
    api::InitResponse response;
    //проводим инициализацию
    response = stand_.InitByPD();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда Init.");
        float angles1 = response.startPlatesAngles_.aHalf_; // значения для Sendmessage
        float angles2 = response.startPlatesAngles_.aQuart_;
        float angles3 = response.startPlatesAngles_.bHalf_;
        float angles4 = response.startPlatesAngles_.bQuart_;

        api::SendMessageResponse response_1;
        api::AdcResponse response_2;
        float h = 0.3; //шаг
        float y1_=0, y2_=0;
        float sum_PDH=0, sum_PDV=0, ratio_1 = 0, ratio_2=0;
        response_2 = stand_.GetLaserPower();
        double Power = response_2.adcResponse_;
        //выполняем тесты
        if(qua == 0){qua=1;}
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
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_LaserTest_clicked() //тестирование лазера
{
    //очистка поля под график
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));


    api::SLevelsResponse response;
    api::AdcResponse response_1;
    api::InitResponse response_2;
    int Power = 0;
    float y1_max = 0,y1_ = 0, y1_min =100000;
    float y2_max = 0,y2_ = 0, y2_min =100000;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    if(response.errorCode_ == 0){
    while (Power <= 100)
    {
        //установка мощности
        response_1 = stand_.SetLaserPower(Power);
        QThread::msleep(120);
        //снятие показателей
        response = stand_.GetSignalLevels();
        x.push_back(Power);
        y1_ = response.signal_.h_;
        y2_ = response.signal_.v_;
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

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}

        ui->widget->xAxis->setRange(0 ,Power);
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
        //увеличение мощности
        Power = Power + 2;
    }
    ConsoleLog("Тестирование закончено!");
    ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

float MainWindow:: AngleCheck (float angle, float step) //проверка кратрости
{
   angle = (angle*10);
   int an = angle;
   int h = step*10;
    if(an % h != 0){
       while (an % h != 0) {
           an += 1;
           angle +=1;
       }
       return angle/10;}
   else{return angle/10;}
}

QStringList MainWindow:: ConvertingArray (QString str){ //преобразование строк в лист
    QStringList list = str.split("",Qt::SkipEmptyParts);
    /*foreach(QString num, list)
        cout << num.toInt() << endl;*/

    return list;
}

QString MainWindow:: ElectionPD_v2(int PDH, int PDV, int yh_, int yv_, int MaxSig_h){ //метод вероятностей

    QString bit = "";
    double val = rand(); //рандом числа
    double val_max = RAND_MAX;
    double val_de = val/val_max;
    double h_de = double(yh_)/double(MaxSig_h); //определение в каком интервале находиться сигнал
    if(yh_ < 200 && yv_< 200){
        QErrorMessage errorMessage;
        errorMessage.showMessage("Ошибка!!! Не работает лазер!");
        errorMessage.exec();
        return bit='X';
    }
    else{
        if( val_de <= h_de){
            if(PDH == 0 & PDV == 1){
                return bit='0';
            }
            else if(PDH == 1 & PDV == 0){
                return bit='1';
               }
            else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");return bit='X';}
        }
        else{
            if(PDH == 0 & PDV == 1){
                return bit='1';
            }
            else if(PDH == 1 & PDV == 0){
                return bit='0';
               }
            else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");return bit='X';}
        }
    }
}
QString MainWindow:: ElectionPD(int PDH, int PDV, int yh_, int yv_){ //метод сравнений

    QString bit = "";
    if(yh_ < 200 && yv_< 200){
        QErrorMessage errorMessage;
        errorMessage.showMessage("Ошибка!!! Не работает лазер!");
        errorMessage.exec();
        return bit='X';
    }
    else{
        if(PDH == 0 & PDV == 1){
            if(yh_ > yv_){return bit='0';}
            else if(yh_ < yv_){return bit='1';}
            else if (yh_ == yv_){return bit='X';}
            else if (yh_ == 0 && yv_ == 0){return bit='X';}
            else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");return bit='X';}
        }
        else if(PDH == 1 & PDV == 0){
            if(yh_ > yv_){return bit='1';}
            else if(yh_ < yv_){return bit='0';}
            else if (yh_ == yv_){return bit='X';}
            else if (yh_ == 0 && yv_ == 0){return bit='X';}
            else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");return bit='X';}
        }
        else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");return bit='X';}
    }
}

QStringList MainWindow:: Protocol (QStringList  AliceBit,QStringList  AliceBasis,QStringList  BobBit, QStringList  BobBasis){ //выполнение протокола

    QStringList bit;
    float aHalf_,aQuart_,bHalf_,bQuart_;
    int PDH_, PDV_;
    int MaxSig_h = (stand_.GetInitParams().maxSignalLevels_.h_)*1.1; //для протокола 1.5 и 1.2
    //int MaxSig_h = (stand_.GetMaxSignalLevels().signal_.h_)*1.1;
    int yh_ = 0, yv_=0;
    api::SendMessageResponse response;
    double Power = stand_.GetLaserPower().adcResponse_;
    for(int i = 0; i<= AliceBit.size()-1;i++){

        //определение параметров для Боба
        if(BobBasis[i]=='1'){
            if(BobBit[i]=='1'){
                bHalf_= ui ->bHalf_11 -> text().toFloat();
                bQuart_= ui ->bQuart_11 -> text().toFloat();
                PDH_ = ui ->PH_11 -> text().toInt();
                PDV_ = ui ->PV_11 -> text().toInt();}
            else{
                bHalf_= ui ->bHalf_10 -> text().toFloat();
                bQuart_= ui ->bQuart_10 -> text().toFloat();
                PDH_ = ui ->PH_10 -> text().toInt();
                PDV_ = ui ->PV_10 -> text().toInt();}
        }
        else{
            if(BobBit[i]=='0'){
                bHalf_= ui ->bHalf_00 -> text().toFloat();
                bQuart_= ui ->bQuart_00 -> text().toFloat();
                PDH_ = ui ->PH_00 -> text().toInt();
                PDV_ = ui ->PV_00 -> text().toInt();}
            else{
                bHalf_= ui ->bHalf_01 -> text().toFloat();
                bQuart_= ui ->bQuart_01 -> text().toFloat();
                PDH_ = ui ->PH_01 -> text().toInt();
                PDV_ = ui ->PV_01 -> text().toInt();
            }
        }

        //определение параметров для Алисы
        if(AliceBasis[i]=='1'){
            if(AliceBit[i]=='1'){
                aHalf_= ui ->aHalf_11 -> text().toFloat();
                aQuart_= ui ->aQuart_11 -> text().toFloat();}
            else{
                aHalf_= ui ->aHalf_10 -> text().toFloat();
                aQuart_= ui ->aQuart_10 -> text().toFloat();}
        }
        else{
            if(AliceBit[i]=='1'){
                aHalf_= ui ->aHalf_01 -> text().toFloat();
                aQuart_= ui ->aQuart_01 -> text().toFloat();}
            else{
                aHalf_= ui ->aHalf_00 -> text().toFloat();
                aQuart_= ui ->aQuart_00 -> text().toFloat();
            }
        }
        response  = stand_.Sendmessage({aHalf_, aQuart_, bHalf_, bQuart_}, Power); //отправка посылки
        //снятие показателей
        yh_ = response.currentSignalLevels_.h_;
        yv_ = response.currentSignalLevels_.v_;
        if(yh_ < 200 && yv_< 200){
            QErrorMessage errorMessage;
            errorMessage.showMessage("Ошибка!!! Не работает лазер!");
            errorMessage.exec();
            break;
        }

        //выбор метода для определения бита
        if (ui->radio_ElectionPD_v2->isDown())
        {
            bit << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
        }
        else
        {
            bit << ElectionPD(PDH_, PDV_, yh_, yv_);
        }

        //Progress
        ui->Progress->clear();
        ui ->Progress->append(QString::number(i+1)+" из "+QString::number(AliceBit.size()));
        ui->progressBar->setValue(((i+1)*100)/AliceBit.size());

        //Вывод сырой строки
        ui->RawLine->clear();
        ui ->RawLine -> append(bit.join(""));
        setUpdatesEnabled(true);
        repaint();
        setUpdatesEnabled(false);
    }
    return bit;
    bit.clear();
}

QStringList MainWindow:: Protocol_Eva (QStringList  AliceBit,QStringList  AliceBasis, QStringList EvaBasis,QStringList  BobBit,QStringList  BobBasis){ //протокол с Евой

    QFile fileOut("./EvaBit.txt");
    fileOut.open(QFile::Append | QFile::Text);
    QTextStream writeStream (&fileOut);

    int MaxSig_h = stand_.GetInitParams().maxSignalLevels_.h_; //1.5 and 1.2
    //int MaxSig_h = (stand_.GetMaxSignalLevels().signal_.h_)*1.1;
    QStringList bit, bit_e;
    QString bit_eva =""; //Биты полученные Евой
    float aHalf_,aQuart_,bHalf_,bQuart_,eHalf_,eQuart_;
    int PDH_, PDV_;
    int yh_ = 0, yv_=0;
    api::SendMessageResponse response;
    double Power = stand_.GetLaserPower().adcResponse_;
    for(int i = 0; i<= AliceBit.size()-1;i++){

        //определение параметров для Ева
        if(EvaBasis[i]=='1'){
            eHalf_= ui ->bHalf_11 -> text().toFloat();
            eQuart_= ui ->bQuart_11 -> text().toFloat();
        }
        else{
            eHalf_= ui ->bHalf_00 -> text().toFloat();
            eQuart_= ui ->bQuart_00 -> text().toFloat();
        }

        //определение параметров для Боба
        if(BobBasis[i]=='1'){
            if(BobBit[i]=='1'){
                bHalf_= ui ->bHalf_11 -> text().toFloat();
                bQuart_= ui ->bQuart_11 -> text().toFloat();
                PDH_ = ui ->PH_11 -> text().toInt();
                PDV_ = ui ->PV_11 -> text().toInt();}
            else{
                bHalf_= ui ->bHalf_10 -> text().toFloat();
                bQuart_= ui ->bQuart_10 -> text().toFloat();
                PDH_ = ui ->PH_10 -> text().toInt();
                PDV_ = ui ->PV_10 -> text().toInt();}
        }
        else{
            if(BobBit[i]=='1'){
                bHalf_= ui ->bHalf_01 -> text().toFloat();
                bQuart_= ui ->bQuart_01 -> text().toFloat();
                PDH_ = ui ->PH_01 -> text().toInt();
                PDV_ = ui ->PV_01 -> text().toInt();}
            else{
                bHalf_= ui ->bHalf_00 -> text().toFloat();
                bQuart_= ui ->bQuart_00 -> text().toFloat();
                PDH_ = ui ->PH_00 -> text().toInt();
                PDV_ = ui ->PV_00 -> text().toInt();
            }
        }

        //определение параметров для Алисы
        if(AliceBasis[i]=='1'){
            if(AliceBit[i]=='1'){
                aHalf_= ui ->aHalf_11 -> text().toFloat();
                aQuart_= ui ->aQuart_11 -> text().toFloat();}
            else{
                aHalf_= ui ->aHalf_10 -> text().toFloat();
                aQuart_= ui ->aQuart_10 -> text().toFloat();}
        }
        else{
            if(AliceBit[i]=='1'){
                aHalf_= ui ->aHalf_01 -> text().toFloat();
                aQuart_= ui ->aQuart_01 -> text().toFloat();}
            else{
                aHalf_= ui ->aHalf_00 -> text().toFloat();
                aQuart_= ui ->aQuart_00 -> text().toFloat();
            }
        }
        response  = stand_.Sendmessage({aHalf_, aQuart_, eHalf_, eQuart_}, Power); //отправка посылки от Алисы к Еве
        //снятие показателей
        yh_ = response.currentSignalLevels_.h_;
        yv_ = response.currentSignalLevels_.v_;
        if(yh_ < 200 && yv_< 200){
            QErrorMessage errorMessage;
            errorMessage.showMessage("Ошибка!!! Не работает лазер!");
            errorMessage.exec();
            break;
        }
        //выбор метода для определения бита
        if (ui->radio_ElectionPD_v2->isDown())
        {
            bit_eva = ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
            bit_e << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
        }
        else
        {

            bit_eva = ElectionPD(PDH_, PDV_, yh_, yv_);
            bit_e << ElectionPD(PDH_, PDV_, yh_, yv_);
        }
        //определение параметров для Евы
        if(EvaBasis[i]=='1'){
            if(bit_eva =='1'){
                eHalf_= ui ->aHalf_11 -> text().toFloat();
                eQuart_= ui ->aQuart_11 -> text().toFloat();}
            else{
                eHalf_= ui ->aHalf_10 -> text().toFloat();
                eQuart_= ui ->aQuart_10 -> text().toFloat();}
        }
        else{
            if(bit_eva =='1'){
                eHalf_= ui ->aHalf_01 -> text().toFloat();
                eQuart_= ui ->aQuart_01 -> text().toFloat();}
            else{
                eHalf_= ui ->aHalf_00 -> text().toFloat();
                eQuart_= ui ->aQuart_00 -> text().toFloat();
            }
        }
        response  = stand_.Sendmessage({eHalf_, eQuart_, bHalf_, bQuart_}, Power); //отправка посылки от Евы к Бобу
         //снятие показателей
        yh_ = response.currentSignalLevels_.h_;
        yv_ = response.currentSignalLevels_.v_;
        //выбор метода для определения бита
        if (ui->radio_ElectionPD_v2->isDown())
        {
            bit << ElectionPD_v2(PDH_, PDV_, yh_, yv_,MaxSig_h);
        }
        else
        {
            bit << ElectionPD(PDH_, PDV_, yh_, yv_);
        }

        //Progress
        ui->Progress->clear();
        ui ->Progress->append(QString::number(i+1)+" из "+QString::number(AliceBit.size()));

        //Вывод сырой строки
        ui->RawLine->clear();
        ui ->EvaBit -> setText(bit_e.join(""));
        ui ->RawLine -> append(bit.join(""));
        setUpdatesEnabled(true);
        repaint();
        setUpdatesEnabled(false);
    }
    writeStream << ("EvaBit: " + bit_e.join("")+ "\n"); //запись битов Евы в файл
    return bit;
    bit.clear();
    bit_e.clear();
}

void MainWindow::on_Start_protocol_clicked() //Начало выполнения протокола
{
    //запуск таймера
    QElapsedTimer timer;
    timer.start();

    ui->progressBar->setValue(0);

    //очистка строк
    ui ->RawLine ->clear();
    ui ->Comparison ->clear();
    ui ->BlankLine ->clear();
    ui ->LengthKey->clear();
    ui ->key -> clear();
    ui ->LengthKey->clear();
    ui ->Time->clear();
    ui ->Speed->clear();
    ui ->error_pr->clear();


    float step = stand_.GetRotateStep().angle_; //шаг двигателя
    //Углы пластин Алисы
    float aHalf_00 = AngleCheck((ui ->aHalf_00 -> text().toFloat()), step); //базис 0 бит 0
    ui ->aHalf_00 -> setText(QString::number(aHalf_00));
    float aQuart_00 = AngleCheck((ui ->aQuart_00 -> text().toFloat()), step); //базис 0 бит 0
    ui ->aQuart_00 -> setText(QString::number(aQuart_00));
    float aHalf_01 = AngleCheck((ui ->aHalf_01 -> text().toFloat()), step); //базис 0 бит 1
    ui ->aHalf_01 -> setText(QString::number(aHalf_01));
    float aQuart_01 = AngleCheck((ui ->aQuart_01 -> text().toFloat()), step); //базис 0 бит 1
    ui ->aQuart_01 -> setText(QString::number(aQuart_01));
    float aHalf_10 = AngleCheck((ui ->aHalf_10 -> text().toFloat()), step); //базис 1 бит 0
    ui ->aHalf_10 -> setText(QString::number(aHalf_10));
    float aQuart_10 = AngleCheck((ui ->aQuart_10 -> text().toFloat()), step); //базис 1 бит 0
    ui ->aQuart_10 -> setText(QString::number(aQuart_10));
    float aHalf_11 = AngleCheck((ui ->aHalf_11 -> text().toFloat()), step); //базис 1 бит 1
    ui ->aHalf_11 -> setText(QString::number(aHalf_11));
    float aQuart_11 = AngleCheck((ui ->aQuart_11 -> text().toFloat()), step); //базис 1 бит 1
    ui ->aQuart_11 -> setText(QString::number(aQuart_11));

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

    //Базис и бит
    QStringList AliceBit = ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = ConvertingArray(ui ->BobBasis -> text());
    QStringList EvaBasis = ConvertingArray(ui ->EvaBasis -> text());

    //Переменные для вывода результата
    QStringList Combit;
    QStringList  CombitA;
    QStringList blankbit;
    QStringList keybit;
    QStringList bit;

    //Определение алгоритма протокола
    if (ui->Evacheck->isChecked())
    {
        bit = Protocol_Eva(AliceBit, AliceBasis, EvaBasis, BobBit, BobBasis);
    }
    else
    {
        bit = Protocol(AliceBit, AliceBasis, BobBit, BobBasis);
    }

    //возвращение в начальное положение
    api::WAnglesResponse response;
    response = stand_.SetPlatesAngles({0,0,0,0});
    setUpdatesEnabled(true);
    //Сравнение базисов
    for(int j = 0; j<= bit.size()-1;j++){
        if(AliceBasis[j] == BobBasis[j]){
            Combit << bit[j];
           CombitA<< AliceBit[j];
        }
        else {Combit << "X"; CombitA<<"X";}
    }
    ui ->Comparison -> setText(Combit.join(""));

    //чистая строка
    for(int a = 0; a<= Combit.size()-1;a++){
        if(Combit[a] == '1' || Combit[a] == '0'){
            blankbit << Combit[a];
        }
        else{blankbit << " ";}
    }
    ui ->BlankLine -> setText(blankbit.join(""));

    //Вывод ключа
    for(int v = 0; v<= blankbit.size()-1;v++){
        if(blankbit[v] == '1' || blankbit[v] == '0'){
            keybit << blankbit[v];
        }
        else{keybit << "";}
    }
    ui ->key -> setText(keybit.join(""));
    ui ->LengthKey->setText(QString::number(keybit.join("").size()));

    float end_time = timer.elapsed(); //остановка таймера
    float search_time = end_time /1000;//CLOCKS_PER_SEC
    float speed = (AliceBasis.size()*1000)/(float)search_time;
    ui ->Time->setText(QString::number(search_time) + "с"); //вывод времени
    ui ->Speed->setText(QString::number(speed/1000)); //вывод скорости
    int nerror=0;

    //Подсчет процента ошибок
    for(int j = 0; j<= Combit.size()-1;j++){
        if(CombitA[j] == Combit[j]){
            nerror++;
        }
    }
    double error = ((double(Combit.size())-double(nerror))/double(Combit.size()))*100;
    ui ->error_pr -> setText(QString::number(error)+ "%"); //вывод процента ошибок
    ParamAngles();
}

int MainWindow:: randomBetween(int low, int high)
{
    return (rand() % ((high + 1) - low) + low);
}

QStringList MainWindow:: Random(int n){
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

void MainWindow::on_Auto_Filing_clicked() //заполнение полей
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

void MainWindow::on_Delete_clicked() //очистка полей
{
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

    ui ->RawLine ->clear();
    ui ->Comparison ->clear();
    ui ->BlankLine ->clear();
    ui ->LengthKey->clear();
    ui ->key -> clear();
    ui ->LengthKey->clear();
    ui ->Time->clear();
    ui ->Speed->clear();
}

void MainWindow::on_TestLine_clicked() //тестовая комбинация
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

    ui ->AliceBit-> setText("0011001100110011");
    ui ->AliceBasis-> setText("0000111100001111");
    ui ->BobBit->setText("0101010101010101");
    ui ->BobBasis->setText("0000111111110000");
    ui ->EvaBasis->setText("1111111111111111");
}

void MainWindow::on_RotateAngles_clicked()// поворот пластин на указаные углы во вкладке Мониторинг
{
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
        else {ConsoleLog("Код ошибки: "+ QString::number (response_1.errorCode_));}
}

void MainWindow::on_MonitoringSend_clicked() //Мониторинг посылки
{
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));


    api::SendMessageResponse response;
    api::AdcResponse response_1;
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;
    Flag_ = false;
    int time = 0;
    float y1_max = 0,y1_ = 0, y1_min =0, n1_;
    float y2_max = 0,y2_ = 0, y2_min =0, n2_;
    //float pdh=0, pdv=0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    if(response.errorCode_ == 0){
    while (Flag_ == false)
    {
        response  = stand_.Sendmessage({0,0,0,0},Power);
        x.push_back(time++);
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
        x.push_back(time++);
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

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}

        ui->widget->xAxis->setRange(0 ,time + 10);
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
        QApplication::processEvents();
        connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
    }
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::slotTimerAlarm()
{

    ui->timer->setText(QTime::currentTime().toString());
}


void MainWindow::on_MonitorNoises_clicked() //Мониториг уровня засветки
{
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();
    y3.clear();

    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    ui->widget->addGraph();
    ui->widget->graph(2)->setPen(QPen(Qt::green));
    ui->widget->graph(2)->setBrush(QBrush(QColor(0, 255, 0, 20)));

    QString angles1,angles2,angles3, angles4;

    api::SendMessageResponse response;
    api::AdcResponse response_1;
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;
    Flag_ = false;
    int time = 0;
    float y1_ = 0, n1_, y2_ = 0, n2_, y1_max = 0, y2_max = 0;
    if(response.errorCode_ == 0){
        while (Flag_ == false)
        {
            angles1 = ui ->Angles1 -> text();
            angles2 = ui ->Angles2 -> text();
            angles3 = ui ->Angles3 -> text();
            angles4 = ui ->Angles4 -> text();
            response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles3.toFloat(),angles4.toFloat()},Power);
            x.push_back(time++);
            y1_ = response.currentSignalLevels_.h_;
            y2_ = response.currentSignalLevels_.v_;
            n1_ = response.currentLightNoises_.h_;
            n2_ = response.currentLightNoises_.v_;

            if (ui->PD_Check->isChecked()) //выводятся значения PDH
            {
                x.push_back(time++);
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
                x.push_back(time++);
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
            if (time < 20){ui->widget->xAxis->setRange(0 ,20);}
            else{ui->widget->xAxis->setRange(0 ,time);}

            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}

            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->graph(2)->addData(x,y3);
            ui->widget->replot();
            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
        }
        ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_Test_Speed_2_clicked()
{
    QElapsedTimer timer;
    timer.start();

     api::SendMessageResponse response;
     api::AdcResponse response_2;
     response_2 = stand_.GetLaserPower();
     double Power = response_2.adcResponse_;

    float n = ui ->QuantityTest-> text().toInt(); //
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


void MainWindow::on_Evacheck_clicked()
{
    if (ui->Evacheck->isChecked()){
        ui ->EvaBasis->setEnabled(true);
        ui ->EvaBit->setEnabled(true);}
    else{
        ui ->EvaBasis->setEnabled(false);
        ui ->EvaBit->setEnabled(false);}
}


void MainWindow::on_Protocol_test_clicked() //Тестовое выполнение протокола
{
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

    int m = ui ->AutoLine -> text().toInt(); //количество раз выполнение протокола
    int n =1024; //длина передоваемой комбинации
    api::WAnglesResponse response;

    QStringList AliceBit;
    QStringList AliceBasis;
    QStringList BobBit;
    QStringList BobBasis;
    QStringList EvaBasis;

    //Переменные для вывода результата
    QStringList Combit;
    QStringList  CombitA, CombitAE;
    QStringList blankbit;
    QStringList keybit;
    QStringList bit;
    QStringList bit_eva;
    //QElapsedTimer timer;
    double error;
    float end_time,search_time,speed;
    int nerror=0;

    QFile fileOut("./test_protocol.txt");
    fileOut.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writeStream (&fileOut);

    for(int i = 1 ;i <= m; i++){
       // timer.start();

        bit.clear();
        Combit.clear();
        CombitA.clear();
        blankbit.clear();
        keybit.clear();
        nerror =0;



        AliceBit = Random(n);
        AliceBasis  = Random(n);
        BobBit = Random(n);
        BobBasis = Random(n);
        EvaBasis = Random(n);
        writeStream << ("Test: " + QString::number(i)+ "\n");
        writeStream << ("AliceBit: " + AliceBit.join("")+ "\n");
        writeStream << ("AliceBasis: " + AliceBasis.join("")+ "\n");
        writeStream << ("BobBit: " + BobBit.join("")+ "\n");
        writeStream << ("BobBasis: " + BobBasis.join("")+ "\n");

     /*  bit = Protocol(AliceBit, AliceBasis, BobBit, BobBasis);
       writeStream << ("Сырая строка: " + bit.join("")+ "\n");

        response = stand_.SetPlatesAngles({0,0,0,0});

        //Сравнение базисов
        for(int j = 0; j<= bit.size()-1;j++){
            if(AliceBasis[j] == BobBasis[j]){
                Combit << bit[j];
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
        writeStream << ("Key: " + keybit.join("")+ "\n");
        writeStream << ("LengthKey: " + QString::number(keybit.join("").size())+ "\n");

        end_time = timer.elapsed();
        search_time = end_time /1000;//CLOCKS_PER_SEC
        speed = (AliceBasis.size()*1000)/(float)search_time;
        writeStream << ("Time: " + QString::number(search_time) + "с"+ "\n");
        writeStream << ("Speed: " + QString::number(speed/1000)+ "\n");

        //Подсчет процента ошибок
        for(int j = 0; j<= Combit.size()-1;j++){
            if(CombitA[j] == Combit[j]){
                nerror++;
            }
        }
        error = ((double(Combit.size())-double(nerror))/double(Combit.size()))*100;
        writeStream << ("Error_pr: " + QString::number(error)+ "%"+ "\n");

        timer.start();
        CombitAE.clear();
        bit_eva.clear();
        Combit.clear();
        blankbit.clear();
        keybit.clear();
        nerror =0;
        error=0;

        writeStream << ("EvaBasis: " + EvaBasis.join("")+ "\n");
        bit_eva = Protocol_Eva(AliceBit, AliceBasis, EvaBasis, BobBit, BobBasis);
        writeStream << ("Сырая строка (Eva): " + bit_eva.join("")+ "\n");
        response = stand_.SetPlatesAngles({0,0,0,0});
        //Сравнение базисов
        for(int j = 0; j<= bit_eva.size()-1;j++){
            if(AliceBasis[j] == BobBasis[j]){
                Combit << bit_eva[j];
               CombitAE<< AliceBit[j];
            }
            else {Combit << "X"; CombitAE<<"X";}
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
        writeStream << ("Key: " + keybit.join("")+ "\n");
        writeStream << ("LengthKey: " + QString::number(keybit.join("").size())+ "\n");

        end_time = timer.elapsed();
        search_time = end_time /1000;//CLOCKS_PER_SEC
        speed = (AliceBasis.size()*1000)/(float)search_time;
        writeStream << ("Time: " + QString::number(search_time) + "с"+ "\n");
        writeStream << ("Speed: " + QString::number(speed/1000)+ "\n");

        //Подсчет процента ошибок
        for(int j = 0; j<= Combit.size()-1;j++){
            if(CombitAE[j] == Combit[j]){
                nerror++;
            }
        }
        error = ((double(Combit.size())-double(nerror))/double(Combit.size()))*100;
        writeStream << ("Error_pr: " + QString::number(error)+ "%"+ "\n");*/
    }
    //response = stand_.SetPlatesAngles({0,0,0,0});

}
void MainWindow::on_Help_clicked()
{
    clc = new HelpWindow(this);
    clc->show();
}

void MainWindow::on_PlateAngle_clicked() //поворот одной пластины
{
    //api::WAnglesResponse response;
    api::AngleResponse response;
    QString angles1 = ui ->InitAngles1 -> text();
    QString angles2 = ui ->InitAngles2 -> text();
    QString angles3 = ui ->InitAngles3 -> text();
    QString angles4 = ui ->InitAngles4 -> text();

    int num = (ui ->NumberAngle-> value());
    if(num == 1){response = stand_.SetPlateAngle(num,angles1.toFloat());}
    else if (num == 2){response = stand_.SetPlateAngle(num,angles2.toFloat());}
    else if (num == 3){response = stand_.SetPlateAngle(num,angles3.toFloat());}
    else if (num == 4){response = stand_.SetPlateAngle(num,angles4.toFloat());}
    else{ConsoleLog("Некорректные данные");}

    ConsoleLog("Выполнена команда SetPlatesAngles");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Угол поворота пластины " + QString::number(num) + " на " +QString::number (response.angle_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        ParamAngles();
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_Eva_key_clicked()
{
    QStringList EvaBasis = ConvertingArray(ui ->EvaBasis -> text());
    QStringList EvaBit = ConvertingArray(ui ->EvaBit -> text());
    QStringList AliceBit = ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = ConvertingArray(ui ->BobBasis -> text());

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


void MainWindow::on_Error_key_clicked()
{
    QStringList AliceBit = ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = ConvertingArray(ui ->BobBasis -> text());

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


void MainWindow::on_Test_monitor_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика
    ui->widget->yAxis->setRange(0,3000);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    api::SendMessageResponse response;
    api::AdcResponse response_1;
    float angles1 = (ui ->Angles1 -> text()).toFloat();
    float angles2 = (ui ->Angles2 -> text()).toFloat();
    float angles3 = (ui ->Angles3 -> text()).toFloat();
    float angles4 = (ui ->Angles4 -> text()).toFloat();
    double h = (ui ->Step_scan-> value());//значение шага
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_; //запись значения мощность
    Flag_ = false;
    int time = 0;
    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;

        while(Flag_ == false){
            response  = stand_.Sendmessage({angles1,angles2,angles3,angles4},Power);
            x.push_back(time++);
            y1_ = response.currentSignalLevels_.h_;
            y2_ = response.currentSignalLevels_.v_;
            y1.push_back(y1_);
            y2.push_back(y2_);

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
            ui ->PDH_max -> setText(QString::number(y1_max));
            ui ->PDH_min -> setText(QString::number(y1_min));
            ui ->PDV_max -> setText(QString::number(y2_max));
            ui ->PDV_min -> setText(QString::number(y2_min));
            ui ->Cur_PDH -> setText(QString::number(y1_));
            ui ->Cur_PDV -> setText(QString::number(y2_));

            ui->widget->xAxis->setRange(0 ,time + 10);
            if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
            else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
            ui->widget->graph(0)->addData(x,y1);
            ui->widget->graph(1)->addData(x,y2);
            ui->widget->replot();
            angles1=angles1 + h;
            //angles2=angles2+(h);
            //angles3=angles3+(h);
            //angles4 = angles4 + h;
            QApplication::processEvents();
            connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
        }
    stand_.SetPlatesAngles({0,0,0,0});
}



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

void MainWindow::on_CreateConfigSecret_clicked()
{
    QString password = ui ->password-> text();
    api::AdcResponse response;
    response = stand_.CreateConfigSecret(password.toStdString());
    ui ->Console_2 -> setText("Код ошибки: " + QString::number (response.errorCode_));
    ui ->Console_2 -> setText("Успешность выполнения: " + QString::number (response.adcResponse_));
}

void MainWindow::on_OpenConfigMode_clicked()
{
    api::AdcResponse response;
    QString password = ui ->password-> text();
    response = stand_.OpenConfigMode(password.toStdString());
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
    if(response.errorCode_==0){
    ui ->Console_2 -> append("Успешность выполнения: " + QString::number (response.adcResponse_));
    if(response.adcResponse_==0){ ui ->Console_2 -> append("Вход в режим API");}
    else{ ui ->Console_2 -> append("Не верный пароль!!!");}}
}

void MainWindow::on_CloseConfigMode_clicked()
{
    stand_.CloseConfigMode();
    ui ->Console_2 -> append("Выход из режима API");
}

void MainWindow::on_GetProtocolVersion_clicked()
{
    hwe::Conserial::versionProtocolResponse response;
     response=stand_.GetProtocolVersion ();
     ui ->Console_2 -> append("Версия библиотеки: " + QString::number (response.version_) + "." + QString::number (response.subversion_));
     ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
 }

void MainWindow::on_GetProtocolFirmwareVersion_clicked()
{
    hwe::Conserial::versionFirmwareResponse response;
    response = stand_.GetCurrentFirmwareVersion();
    ui ->Console_2 -> append("Версия прошивки: " + QString::number (response.major_) + "." + QString::number (response.minor_)+ "." +QString::number (response.micro_));
    ui ->Console_2 -> append("Код ошибки: " + QString::number (response.errorCode_));
}

void MainWindow::on_GetCurrentMode_clicked()
{
   ui ->Console_2 -> append("Режим работы: " + QString::number(stand_.GetCurrentMode()));
}

void MainWindow::on_SetComPortName_clicked()
{
    QString port = ui ->Port-> text();//наименование порта
    stand_.SetComPortName(port.toStdString().c_str());
     ui ->Console_2 -> append("Порт установлен");
}

void MainWindow::on_GetComPortName_clicked()
{
    stand_.GetComPortName();
    ui ->Console_2 -> append("Порт:" + QString::fromStdString(stand_.GetComPortName()));
}

void MainWindow::on_GetMaxPayloadSize_clicked()
{
   ui ->Console_2 -> setText("Количество байт:" + QString::number(stand_.GetMaxPayloadSize()));
}

void MainWindow::on_FirmwareUpdate_clicked()
{
    QString path = ui ->File_path-> text();
    stand_.FirmwareUpdate(path.toStdString());
    ui ->Console_2 -> append("Прошивка стенда");
}

void MainWindow::on_StopLaser2_clicked()
{

}


void MainWindow::on_radio_admin_clicked()
{
    if (ui->radio_admin->isChecked()){
        ui->tabWidget->setTabEnabled(3,true);}
    else{ ui->tabWidget->setTabEnabled(3,false);}
}


void MainWindow::on_histogram_clicked()
{

}

