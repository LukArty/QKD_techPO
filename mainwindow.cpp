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
#include <conserial.h>
using namespace std;


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
{
DebugLogger::StartLogging("./log.log");
ui->setupUi(this);

QString filename("/home/diana/QKDStand/PD.png");
    QImage* img=new QImage;
    img->load(filename);
    ui->imageLabel->setPixmap(QPixmap::fromImage(*img));

ui->widget->xAxis->setRange(0,180);
ui->widget->yAxis->setRange(0,3000);
}


MainWindow::~MainWindow()
{
stand_.SetLaserState(0);
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
     ConsoleLog("Выполнена команда SetPlatesAngles");
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
    ConsoleLog("Выполнена команда GetPlatesAngles");
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

    ui->widget->xAxis->setRange(0,180);
    ui->widget->yAxis->setRange(0,3000);
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

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
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
        ui ->Cur_PDH -> setText(QString::number(y1_));
        ui ->Cur_PDV -> setText(QString::number(y2_));

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
    api::WAnglesResponse response_2;
    response_2 = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});

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
        ui ->Cur_PDH -> setText(QString::number(y1_));
        ui ->Cur_PDV -> setText(QString::number(y2_));

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

    ui->widget->xAxis->setRange(0,180);
    ui->widget->yAxis->setRange(0,3000);
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

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
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
        ui ->Cur_PDH -> setText(QString::number(y1_));
        ui ->Cur_PDV -> setText(QString::number(y2_));

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
    api::WAnglesResponse response_2;
    response_2 = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
}

void MainWindow::on_ScanAngles3_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    ui->widget->xAxis->setRange(0,180);
    ui->widget->yAxis->setRange(0,3000);
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

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
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
        ui ->Cur_PDH -> setText(QString::number(y1_));
        ui ->Cur_PDV -> setText(QString::number(y2_));

        if (y1_max>y2_max){ui->widget->yAxis->setRange(0 ,y1_max + 10);}
        else{ui->widget->yAxis->setRange(0 ,y2_max + 10);}
        ui->widget->graph(0)->addData(x,y1);
        ui->widget->graph(1)->addData(x,y2);
        ui->widget->replot();
    }
    api::WAnglesResponse response_2;
    response_2 = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
}

void MainWindow::on_ScanAngles4_clicked()
{
    //очистка предыдущего графика
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    ui->widget->xAxis->setRange(0,180);
    ui->widget->yAxis->setRange(0,3000);
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

    int y1_max = 0,y1_ = 0, y1_min = 100000;
    int y2_max = 0,y2_ = 0, y2_min = 100000;
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
    }
    api::WAnglesResponse response_2;
    response_2 = stand_.SetPlatesAngles({angles1.toFloat(), angles2.toFloat(), angles3.toFloat(), angles4.toFloat()});
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

void MainWindow::on_PDRatio_clicked()
{
    ConsoleLog("Корректность распределений мощностей в поляризационном кубике:");
    int qua = ui ->QuantityTest -> text().toInt();//количество тестов
    api::InitResponse response;
    response = stand_.InitByPD();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда Init.");
        float angles1 = response.startPlatesAngles_.aHalf_; // значения для Sendmessage
        float angles2 = response.startPlatesAngles_.aQuart_;
        float angles3 = response.startPlatesAngles_.bHalf_;
        float angles4 = response.startPlatesAngles_.bQuart_;

    api::SendMessageResponse response_1;
    api::AdcResponse response_2;
    float h = 0.3;
    float n = angles1; //начальный отступ от 0
    float y1_=0, y2_=0;
    float sum_PDH=0, sum_PDV=0, ratio_1 = 0, ratio_2=0;
    response_2 = stand_.GetLaserPower();
    double Power = response_2.adcResponse_;
    if(qua == 0){qua=1;}
    for (int i = 1; i <= qua; i++){
        for (angles1; angles1 <= n+180;angles1 +=h)
        {
            response_1  = stand_.Sendmessage({angles1, angles2, angles3, angles4}, Power);
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
         angles1 = n;
         sum_PDH=0;
         sum_PDV=0;
         ratio_1 = 0;
         ratio_2 =0;
    }
    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

void MainWindow::on_LaserTest_clicked()
{
    ui->widget->clearGraphs();
    x.clear();
    y1.clear();
    y2.clear();

    ui->widget->xAxis->setRange(0,110);
    ui->widget->yAxis->setRange(0,110);
    ui->widget->addGraph();
    ui->widget->graph(0)->setPen(QPen(Qt::blue));
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    ConsoleLog("Тестирование работы лазера:");
    api::InitResponse response;
    response = stand_.InitByPD();
    if (response.errorCode_ == 0){
        ConsoleLog("Выполнена команда Init.");
        float angles1 = response.startPlatesAngles_.aHalf_; // значения для Sendmessage
        float angles2 = response.startPlatesAngles_.aQuart_;
        float angles3 = response.startPlatesAngles_.bHalf_;
        float angles4 = response.startPlatesAngles_.bQuart_;

    api::SendMessageResponse response_1;
    float h = 0.3;
    int step = ui ->StepLaser -> text().toInt();//шаг для изменения мощности лазера
    float n = angles1; //начальный отступ от 0
    float y1_=0, y2_=0;
    float sum_PDH=0, sum_PDV=0, ratio_1 = 0, ratio_2;
    int Power = 10;
    int i = 1;
    if(step == 0){step=1;}

    for(Power;Power<=100;Power += step){

        for (angles1; angles1 <= n+180;angles1 +=h)
        {
            response_1  = stand_.Sendmessage({angles1, angles2, angles3, angles4}, Power);
            y1_ = response_1.currentSignalLevels_.h_;
            y2_ = response_1.currentSignalLevels_.v_;
            sum_PDH += y1_;
            sum_PDV += y2_;
        }
        ratio_1= (sum_PDH*100)/(sum_PDH + sum_PDV);
        ratio_2= (sum_PDV*100)/(sum_PDH + sum_PDV);
         ConsoleLog("Итерация "+ QString::number (i));
         ConsoleLog("Сумма всех значений полученых на PDH: "+ QString::number (sum_PDH));
         ConsoleLog("Сумма всех значений полученых на PDV: "+ QString::number (sum_PDV));
         ConsoleLog("Соотношение мощностей: "+ QString::number (ratio_1)+" %" +"   "+ QString::number (ratio_2)+" %");
         x.push_back(Power);
         if(ratio_1 >= ratio_2){
            y1.push_back(ratio_1-ratio_2);
         }
         else {y1.push_back(ratio_2-ratio_1);}

         ui->widget->graph(0)->addData(x,y1);
         ui->widget->replot();

         angles1 = n;
         i++; sum_PDH=0, sum_PDV=0, ratio_1 = 0; ratio_2 = 0;
    }

    }
    else {ConsoleLog("Код ошибки: "+ QString::number (response.errorCode_));}
}

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
       return angle/10;}
   else{return angle/10;}
}

QStringList MainWindow:: ConvertingArray (QString str){
    QStringList list = str.split("",QString::SkipEmptyParts);
    /*foreach(QString num, list)
        cout << num.toInt() << endl;*/

    return list;
}

QString MainWindow:: ElectionPD(int PDH, int PDV, int yh_, int yv_){

    QString bit = "";
    if(PDH == 0 & PDV == 1){
        if(yh_ > yv_){return bit='0';}
        else if(yh_ < yv_){return bit='1';}
        else if (yh_ == yv_){return bit='X';}
        else if (yh_ == 0 && yv_ == 0){return bit='X';}
    else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");}
    }
    else if(PDH == 1 & PDV == 0){
        if(yh_ > yv_){return bit='1';}
        else if(yh_ < yv_){return bit='0';}
        else if (yh_ == yv_){return bit='X';}
        else if (yh_ == 0 && yv_ == 0){return bit='X';}
    else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");}
    }
    else {ConsoleLog("Ошибка!!! Некорректно заполнены поля PH и PV");}
}

void MainWindow::on_Start_protocol_clicked()
{
    unsigned int start_time =  clock();

    api::AngleResponse response_5;
    response_5 = stand_.GetRotateStep();
    float step = response_5.angle_;
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

    //Фотодетекторы
    int PDH_00 = ui ->PH_00 -> text().toInt(); //базис 0 бит 0
    int PDH_01 = ui ->PH_01 -> text().toInt(); //базис 0 бит 1
    int PDH_10 = ui ->PH_10 -> text().toInt(); //базис 1 бит 0
    int PDH_11 = ui ->PH_11 -> text().toInt(); //базис 1 бит 1

    int PDV_00 = ui ->PV_00 -> text().toInt(); //базис 0 бит 0
    int PDV_01 = ui ->PV_01 -> text().toInt(); //базис 0 бит 1
    int PDV_10 = ui ->PV_10 -> text().toInt(); //базис 1 бит 0
    int PDV_11 = ui ->PV_11 -> text().toInt(); //базис 1 бит 1

    //Базис и бит
    QStringList AliceBit = ConvertingArray(ui ->AliceBit -> text());
    QStringList AliceBasis = ConvertingArray(ui ->AliceBasis -> text());
    QStringList  BobBit = ConvertingArray(ui ->BobBit -> text());
    QStringList BobBasis = ConvertingArray(ui ->BobBasis -> text());

   api::SendMessageResponse response;
    api::AdcResponse response_2;
    response_2 = stand_.GetLaserPower();
    double Power = response_2.adcResponse_;
    int yh_ = 0, yv_=0;
    QStringList bit;

    //Выполнение протокола
    for(int i = 0; i<= AliceBit.size()-1;i++){
        if(AliceBit[i] == '0' && AliceBasis[i] == '0' && BobBit[i] == '0' && BobBasis[i] == '0'){
            response  = stand_.Sendmessage({aHalf_00, aQuart_00, bHalf_00, bQuart_00}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='0' && BobBit[i]=='0' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_01, aQuart_01, bHalf_00, bQuart_00}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='1' && BobBit[i]=='0' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_10, aQuart_10, bHalf_00, bQuart_00}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='1' && BobBit[i]=='0' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_11, aQuart_11, bHalf_00, bQuart_00}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='0' && BobBit[i]=='1' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_00, aQuart_00, bHalf_01, bQuart_01}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_01, PDV_01, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='0' && BobBit[i]=='1' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_01, aQuart_01, bHalf_01, bQuart_01}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_01, PDV_01, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='1' && BobBit[i]=='1' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_10, aQuart_10, bHalf_01, bQuart_01}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_01, PDV_01, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]== '1' && BobBit[i]=='1' && BobBasis[i]=='0'){
            response  = stand_.Sendmessage({aHalf_11, aQuart_11, bHalf_01, bQuart_01}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_01, PDV_01, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='0' && BobBit[i]=='0' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_00, aQuart_00, bHalf_10, bQuart_10}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_10, PDV_10, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='0' && BobBit[i]=='0' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_01, aQuart_01, bHalf_10, bQuart_10}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_10, PDV_10, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='1' && BobBit[i]=='0' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_10, aQuart_10, bHalf_10, bQuart_10}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_10, PDV_10, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='1' && BobBit[i]=='0' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_11, aQuart_11, bHalf_10, bQuart_10}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_10, PDV_10, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='0' && BobBit[i]=='1' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_00, aQuart_00, bHalf_11, bQuart_11}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_11, PDV_11, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='0' && BobBit[i]=='1' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_01, aQuart_01, bHalf_11, bQuart_11}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_11, PDV_11, yh_, yv_);
        }
        else if(AliceBit[i]=='0' && AliceBasis[i]=='1' && BobBit[i]=='1' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_10, aQuart_10, bHalf_11, bQuart_11}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_11, PDV_11, yh_, yv_);
        }
        else if(AliceBit[i]=='1' && AliceBasis[i]=='1' && BobBit[i]=='1' && BobBasis[i]=='1'){
            response  = stand_.Sendmessage({aHalf_11, aQuart_11, bHalf_11, bQuart_11}, Power);
            yh_ = response.currentSignalLevels_.h_;
            yv_ = response.currentSignalLevels_.v_;
            bit << ElectionPD(PDH_11, PDV_11, yh_, yv_);
        }
         else {ConsoleLog("Ошибка!!! Некорректно заполнено поле");}
    }
    api::WAnglesResponse response_3;
    response_3 = stand_.SetPlatesAngles({0,0,0,0});

    /*for(int i = 0; i <= AliceBit.size()-1; i++){
            if(AliceBit[i]== '0' && AliceBasis[i]== '0' && BobBit[i]== '0' && BobBasis[i]== '0'){
                yh_ = 100;
                yv_ = 10;
                bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
            }
            else if(AliceBit[i]== '1' && AliceBasis[i]== '0' && BobBit[i]== '0' && BobBasis[i]== '0'){
                yh_ = 10;
                yv_ = 100;
                bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
            }
            else if(AliceBit[i]== '0' && AliceBasis[i]== '1' && BobBit[i]== '0' && BobBasis[i]== '0'){
                yh_ = 100;
                yv_ = 10;
                bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
            }
            else if(AliceBit[i]== '1' && AliceBasis[i]== '1' && BobBit[i]== '0' && BobBasis[i]== '0'){
                yh_ = 10;
                yv_ = 100;
                bit << ElectionPD(PDH_00, PDV_00, yh_, yv_);
            }
            else {ConsoleLog("Ошибка!!! Некорректно заполнено поле");}
    }*/

    //Вывод сырой строки
    ui ->RawLine -> setText(bit.join(""));

    //Сравнение базисов
    QStringList Combit;
    for(int i = 0; i<= AliceBasis.size()-1;i++){
        if(AliceBasis[i] == BobBasis[i]){
            Combit << bit[i];
        }
        else {Combit << "X";}
    }
    ui ->Comparison -> setText(Combit.join(""));

    //чистая строка
    QStringList blankbit;
    for(int i = 0; i<= Combit.size()-1;i++){
        if(Combit[i] == '1' || Combit[i] == '0'){
            blankbit << Combit[i];
        }
        else{blankbit << " ";}
    }
    ui ->BlankLine -> setText(blankbit.join(""));

    //Вывод ключа
    QStringList keybit;
    for(int i = 0; i<= blankbit.size()-1;i++){
        if(blankbit[i] == '1' || blankbit[i] == '0'){
            keybit << blankbit[i];
        }
        else{keybit << "";}
    }
    ui ->key -> setText(keybit.join(""));
    ui ->LengthKey->setText(QString::number(keybit.join("").size()));

    unsigned int end_time = clock();
    unsigned int search_time = end_time - start_time;
    float speed = (AliceBasis.size()*1000)/(float)search_time;
    ui ->Time->setText(QString::number(search_time/1000) + " с");
    ui ->Speed->setText(QString::number(speed));
}

int MainWindow:: randomBetween(int low, int high)
{
    return (qrand() % ((high + 1) - low) + low);
}

QStringList MainWindow:: Random(int n){
    QStringList Line;
    for (int i = 0; i < n; ++i)
        {
             Line << QString::number(randomBetween(0, 1));
        }
    return Line;
}

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

    ui ->AliceBit-> setText(Random(n).join(""));
    ui ->AliceBasis-> setText(Random(n).join(""));
    ui ->BobBit->setText(Random(n).join(""));
    ui ->BobBasis->setText(Random(n).join(""));
}

void MainWindow::on_Delete_clicked()
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

    ui ->RawLine ->clear();
    ui ->Comparison ->clear();
    ui ->BlankLine ->clear();
    ui ->LengthKey->clear();
    ui ->key -> clear();
    ui ->LengthKey->clear();
    ui ->Time->clear();
    ui ->Speed->clear();
}

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

    ui ->AliceBit-> setText("0000111100001111");
    ui ->AliceBasis-> setText("0000000011111111");
    ui ->BobBit->setText("0101010101010101");
    ui ->BobBasis->setText("0011001100110011");
}

void MainWindow::on_RotateAngles_clicked()
{
        api::SendMessageResponse response;
        api::AdcResponse response_1;
        QString angles1 = ui ->Angles1 -> text();
        QString angles2 = ui ->Angles2 -> text();
        QString angles3 = ui ->Angles3 -> text();
        QString angles4 = ui ->Angles4 -> text();
        response_1 = stand_.GetLaserPower();
        if (response_1.errorCode_ == 0){
           int Power = response_1.adcResponse_;
           response  = stand_.Sendmessage({angles1.toFloat(),angles2.toFloat(),angles3.toFloat(),angles4.toFloat()},Power);
           ui ->Cur_PDH -> setText(QString::number(response.currentSignalLevels_.h_));
           ui ->Cur_PDV -> setText(QString::number(response.currentSignalLevels_.v_));
        }
        else {ConsoleLog("Код ошибки: "+ QString::number (response_1.errorCode_));}
}
