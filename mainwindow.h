#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <conserial.h>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_InitBut_clicked();

    void on_RunSelfTestBut_clicked();

    void on_GetErrorCodeBut_clicked();

    void on_GetLaserStateBut_clicked();

    void on_SetLaserStateBut_clicked();

    void on_SetLaserPowerBut_clicked();

    void on_GetLaserPowerBut_clicked();

    void on_GetRotateStepBut_clicked();

    void on_GetMaxLaserPowerBut_clicked();

    void on_SetPlateAngleBut_clicked();

    void on_GetStartPlatesAnglesBut_clicked();

    void on_GetCurPlatesAnglesBut_clicked();

    void on_SetTimeoutBut_clicked();

    void on_GetTimeoutBut_clicked();

    void on_GetSignalLevelBut_clicked();

    void on_GetMaxSignalLevel_clicked();

    void on_GetLightNoisesBut_clicked();

    void on_GetStartLightNoisesBut_clicked();

    void on_SendMessageBut_clicked();

    void ConsoleLog(QString text);

    //int SendUart(char NameComand, uint16_t Parametr);

    //bool GetLaserState();

    //uint16_t GetLaserPower();

    //void SetLaserState(uint16_t LState);

    //void GetSignalLevel(uint16_t* PLight1, uint16_t* PLight2);

    //void SetLaserPower(uint16_t LPower);

    void on_WriteIni_clicked();

    void on_InitByButtons_clicked();

    void on_ReadIni_clicked();

    void on_InitByPD_clicked();

    void on_PulseLaser_clicked();

    void on_StopLaser_clicked();

    void killLoop() { Flag_ = true; }

    void on_ScanAngles1_clicked();

    void on_ScanAngles2_clicked();

    void on_ScanAngles3_clicked();

    void on_ScanAngles4_clicked();

    void on_MonitoringPD_clicked();

    void on_GetInitParams_clicked();

private:
    Ui::MainWindow *ui;

    hwe::Conserial stand_;
    bool Flag_;
    QVector<double> x,y1,y2;
};
#endif // MAINWINDOW_H
