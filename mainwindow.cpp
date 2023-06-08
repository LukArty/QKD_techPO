#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <QSettings>
#include <QVariant>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include <string>
#include <conserial.h>
using namespace std;


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
{
DebugLogger::StartLogging("./log.log");
ui->setupUi(this);


ui->widget->xAxis->setRange(0,180);
ui->widget->yAxis->setRange(0,3000);
}


MainWindow::~MainWindow()
{
DebugLogger::EndLogging();
    delete ui;
}

void MainWindow::ConsoleLog(QString text)
{
ui->CommandConsole->append(text);
//ui->CommandConsole->scroll()
}


void MainWindow::on_InitBut_clicked()
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

            ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
            ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

            ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
            ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
   }
}


void MainWindow::on_RunSelfTestBut_clicked()
{
    api::AdcResponse response;
    response = stand_.RunTest(1);
    ConsoleLog("Выполнена команда RunSelfTest");
    ConsoleLog("Тестирование лазера и фотодетекторов:");
     if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Значение: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
     }
     else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}


/*if(GetLaserState())
{
    //SetLaserState(0);
    int a = 5;
}*/
//ConsoleLog("Тест 1/7: Успешно");

//GetSignalLevel(Plight1, Plight2);
//uint16_t* startLightNoise1 = Plight1;
//uint16_t*startLightNoise2 = Plight2;
//ConsoleLog("Тест 2/7: Успешно");

/*SetLaserState(1);
if (!GetLaserState())
{
    ConsoleLog("Тест 3/7: Неудачно");
    return;
}
*/
//ConsoleLog("Тест 3/7: Успешно");

//GetSignalLevel(Plight1, Plight2);
//uint16_t* firstSignal1 = Plight1;
//uint16_t* firstSignal2 = Plight2;
/*
if (startLightNoise1 > Plight1 & startLightNoise2 > Plight2)
{
    ConsoleLog("Тест 4/7: Неудачно");
    return;
}
ConsoleLog("Тест 4/7: Успешно");
*/
//uint16_t minPower = 1;
//SetLaserPower(minPower);
//LaserPower = GetLaserPower();
/*if (LaserPower != minPower)
{
    ConsoleLog("Тест 5/7: Неудачно");
    //return;
}
ConsoleLog("Тест 5/7: Успешно");
*/
//GetSignalLevel(Plight1, Plight2);
/*uint16_t* secondSignal1 = Plight1;
uint16_t* secondSignal2 = Plight2;
if (firstSignal1 == Plight1 & firstSignal2 == Plight2)
{
    ConsoleLog("Тест 6/7: Неудачно");
    //return;
}
ConsoleLog("Тест 6/7: Успешно");

uint16_t maxPower = 255;

//SetLaserPower(maxPower);
//GetSignalLevel(Plight1, Plight2);
if (secondSignal1 > Plight1 & secondSignal2 > Plight2)
{
    ConsoleLog("Тест 7/7: Неудачно");
    //return;
}
*/
//ConsoleLog("Тест 7/7: Успешно");
//ConsoleLog("Тестирование завершено");
}


void MainWindow::on_GetErrorCodeBut_clicked()
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



void MainWindow::on_GetLaserStateBut_clicked()
{
    api::AdcResponse response;
    response = stand_.GetLaserState();
    ConsoleLog("Выполнена команда GetLaserState");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_SetLaserStateBut_clicked()
{
    api::AdcResponse response;

    if (ui->LaserState->isChecked())
    {
        response = stand_.SetLaserState(1);
       // ConsoleLog("    Установлено состояние лазера - вкл");
    }
    else
    {
        response = stand_.SetLaserState(0);
        //ConsoleLog("    Установлено состояние лазера - выкл");
    }
    ConsoleLog("Выполнена команда SetLaserState");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Состояние лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_SetLaserPowerBut_clicked()
{
    api::AdcResponse response;
    QString value_ = ui ->LaserPowerValue -> text();
    response = stand_.SetLaserPower(value_.toUInt());
    ConsoleLog("Выполнена команда SetLaserPower");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Значение: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_GetLaserPowerBut_clicked()
{
    ConsoleLog("Выполнена команда GetLaserPower");

    api::AdcResponse response;
    response = stand_.GetLaserPower();
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Мощность лазера: "+ QString::number (response.adcResponse_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_GetRotateStepBut_clicked()
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


void MainWindow::on_SetPlateAngleBut_clicked()
{
     api::WAnglesResponse response;
     QString angles1 = ui ->InitAngles1 -> text();
     QString angles2 = ui ->InitAngles2 -> text();
     QString angles3 = ui ->InitAngles3 -> text();
     QString angles4 = ui ->InitAngles4 -> text();

     response = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
     ConsoleLog("Выполнена команда SetPlateAngle");
     if (response.errorCode_ == 0){
         ConsoleLog("Установленны значения:");
         ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
         ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
         ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
         ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
         ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
     }
     else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_GetCurPlatesAnglesBut_clicked()
{
    api::WAnglesResponse response;
    response = stand_.GetPlatesAngles();
    ConsoleLog("Выполнена команда GetCurPlatesAngles");
    if (response.errorCode_ == 0){
        ConsoleLog("Установленны значения:");
        ConsoleLog("Угол на полуволновой пластине Алисы: "+ QString::number (response.angles_.aHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Алисы: "+ QString::number (response.angles_.aQuart_));
        ConsoleLog("Угол на полуволновой пластине Боба: "+ QString::number(response.angles_.bHalf_));
        ConsoleLog("Угол на четвертьволновой пластине Боба: "+ QString::number (response.angles_.bQuart_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_SetTimeoutBut_clicked()
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


void MainWindow::on_GetTimeoutBut_clicked()
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


void MainWindow::on_GetSignalLevelBut_clicked()
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


void MainWindow::on_GetLightNoisesBut_clicked()
{
    api::SLevelsResponse response;
    response = stand_.GetLightNoises();
    ConsoleLog("Выполнена команда GetLightNoises");
    ConsoleLog("Установленны значения:");
    if(response.errorCode_ == 0){
        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.signal_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.signal_.v_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}


void MainWindow::on_SendMessageBut_clicked()
{
     api::SendMessageResponse response;
     response  = stand_.Sendmessage({0,0,0,0},30);
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
     }
     else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

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

void MainWindow::on_InitByButtons_clicked() //
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

        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

        ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}

}

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

void MainWindow::on_InitByPD_clicked()
{
    //std::cout << stand.Test() << endl;
    api::InitResponse response;
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

        ConsoleLog("Уровень сигнала на первом фотодетекторе: "+ QString::number (response.maxSignalLevels_.h_));
        ConsoleLog("Уровень сигнала на втором фотодетекторе: "+ QString::number (response.maxSignalLevels_.v_));

        ConsoleLog("Мощность лазера: "+ QString::number (response.maxLaserPower_));
        ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_PulseLaser_clicked()
{
    Flag_ = false;
    api::AdcResponse response;

        response = stand_.GetLaserPower();
        response = stand_.SetLaserPower(response.adcResponse_);
        if(response.errorCode_ == 0){
        while (Flag_ == false)
        {
            response = stand_.SetLaserState(1);
            response = stand_.SetLaserState(0);
            QApplication::processEvents();
            connect( ui->StopLaser, SIGNAL( clicked() ), this, SLOT(killLoop()) );
        }
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

    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ui->widget->addGraph();
    ui->widget->graph(1)->setPen(QPen(Qt::red));
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));

    api::SendMessageResponse response;
    api::AdcResponse response_1;
    QString angles1 = ui ->Angles1 -> text();
    QString angles2 = ui ->Angles2 -> text();
    QString angles3 = ui ->Angles3 -> text();
    QString angles4 = ui ->Angles4 -> text();
    float h = 0.3;
    float n = angles1.toFloat(); //начальный отступ от 0
    float angles = angles1.toDouble(); // значения для Sendmessage
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;

    float y1_max = 0,y1_ = 0, y1_min =0;
    float y2_max = 0,y2_ = 0, y2_min =0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    for (angles; angles <= n+180;angles +=h)
    {
        response  = stand_.Sendmessage({angles,angles2.toFloat(),angles3.toFloat(),angles4.toFloat()},Power);
        x.push_back(angles-n);
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

        PDH_max = QString::number(y1_max);
        PDH_min = QString::number(y1_min);
        PDV_max = QString::number(y2_max);
        PDV_min = QString::number(y2_min);
        ui ->PDH_max -> setText(PDH_max);
        ui ->PDH_min -> setText(PDH_min);
        ui ->PDV_max -> setText(PDV_max);
        ui ->PDV_min -> setText(PDV_min);

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }

    /*for (angles; angles <= n + 180;angles +=h)
    {
        x.push_back(angles-n);
        y1.push_back(angles+50);
        y2.push_back(angles+2);
        y1_ = angles+50;
        if(y1_ > y1_max){
            y1_max =y1_;
        }
        if (y1_ < y1_min){
            y1_min =y1_;
        }

        y2_ = angles+2;
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

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }*/
}

void MainWindow::on_ScanAngles2_clicked()
{
    //очистка предыдущего графика
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
    QString angles1 = ui ->Angles1 -> text();
    QString angles2 = ui ->Angles2 -> text();
    QString angles3 = ui ->Angles3 -> text();
    QString angles4 = ui ->Angles4 -> text();
    float h = 0.3;
    float n = angles2.toFloat();
    float angles = angles2.toDouble();
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;

    float y1_max = 0,y1_ = 0, y1_min =0;
    float y2_max = 0,y2_ = 0, y2_min =0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    for (angles; angles <= n+180;angles +=h)
    {
        response  = stand_.Sendmessage({angles1.toFloat(),angles,angles3.toFloat(),angles4.toFloat()},Power);
        x.push_back(angles-n);
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

        PDH_max = QString::number(y1_max);
        PDH_min = QString::number(y1_min);
        PDV_max = QString::number(y2_max);
        PDV_min = QString::number(y2_min);
        ui ->PDH_max -> setText(PDH_max);
        ui ->PDH_min -> setText(PDH_min);
        ui ->PDV_max -> setText(PDV_max);
        ui ->PDV_min -> setText(PDV_min);

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
}

void MainWindow::on_ScanAngles3_clicked()
{
    //очистка предыдущего графика
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
    QString angles1 = ui ->Angles1 -> text();
    QString angles2 = ui ->Angles2 -> text();
    QString angles3 = ui ->Angles3 -> text();
    QString angles4 = ui ->Angles4 -> text();
    float h = 0.3;
    float n = angles3.toFloat();
    float angles = angles3.toDouble();
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;

    float y1_max = 0,y1_ = 0, y1_min =0;
    float y2_max = 0,y2_ = 0, y2_min =0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    for (angles; angles <= n+180;angles +=h)
    {
        response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles,angles4.toFloat()},Power);
        x.push_back(angles-n);
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

        PDH_max = QString::number(y1_max);
        PDH_min = QString::number(y1_min);
        PDV_max = QString::number(y2_max);
        PDV_min = QString::number(y2_min);
        ui ->PDH_max -> setText(PDH_max);
        ui ->PDH_min -> setText(PDH_min);
        ui ->PDV_max -> setText(PDV_max);
        ui ->PDV_min -> setText(PDV_min);

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
}

void MainWindow::on_ScanAngles4_clicked()
{
    //очистка предыдущего графика
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
    QString angles1 = ui ->Angles1 -> text();
    QString angles2 = ui ->Angles2 -> text();
    QString angles3 = ui ->Angles3 -> text();
    QString angles4 = ui ->Angles4 -> text();
    float h = 0.3;
    float n = angles4.toFloat();
    float angles = angles4.toDouble();
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;

    float y1_max = 0,y1_ = 0, y1_min =0;
    float y2_max = 0,y2_ = 0, y2_min =0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    for (angles; angles <= n+180;angles +=h)
    {
        response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles3.toFloat(),angles},Power);
        x.push_back(angles-n);
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

        PDH_max = QString::number(y1_max);
        PDH_min = QString::number(y1_min);
        PDV_max = QString::number(y2_max);
        PDV_min = QString::number(y2_min);
        ui ->PDH_max -> setText(PDH_max);
        ui ->PDH_min -> setText(PDH_min);
        ui ->PDV_max -> setText(PDV_max);
        ui ->PDV_min -> setText(PDV_min);

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
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


    api::SLevelsResponse response;

    /*api::SendMessageResponse response;
    api::AdcResponse response_1;
    response_1 = stand_.GetLaserPower();
    double Power = response_1.adcResponse_;*/
    Flag_ = false;
    int time = 0;
    float y1_max = 0,y1_ = 0, y1_min =0;
    float y2_max = 0,y2_ = 0, y2_min =0;
    QString PDH_max,PDH_min, PDV_max,PDV_min;
    if(response.errorCode_ == 0){
    while (Flag_ == false)
    {
        /*response  = stand_.Sendmessage({0,0,0,0},Power);
        x.push_back(time++);
        y1_ = response.currentSignalLevels_.h_;
        y2_ = response.currentSignalLevels_.v_;*/

        response = stand_.GetSignalLevels();
        x.push_back(time++);
        y1_ = response.signal_.h_;
        y2_ = response.signal_.v_;
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

        PDH_max = QString::number(y1_max);
        PDH_min = QString::number(y1_min);
        PDV_max = QString::number(y2_max);
        PDV_min = QString::number(y2_min);
        ui ->PDH_max -> setText(PDH_max);
        ui ->PDH_min -> setText(PDH_min);
        ui ->PDV_max -> setText(PDV_max);
        ui ->PDV_min -> setText(PDV_min);

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
   /* while (Flag_ == false)
    {
        x.push_back(time++);
        y1.push_back(time+50);
        y2.push_back(time+2);
        y1_ = time+50;
        if(y1_ > y1_max){
            y1_max =y1_;
        }
        if (y1_ < y1_min){
            y1_min =y1_;
        }

        y2_ = time+2;
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

        ui->widget->xAxis->setRange(0 ,time + 10);
        ui->widget->yAxis->setRange(0 ,y1_max + 10);
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
        QApplication::processEvents();
        connect( ui->Stop_monitoring, SIGNAL( clicked() ), this, SLOT(killLoop()) );
    }*/
}

void MainWindow::on_GetInitParams_clicked()
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
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}
