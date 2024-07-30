#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <conserial.h>
#include <QVector>
#include <helpwindow.h>
#include <histogram.h>
#include <histogrameva.h>
#include <QTimer>
#include <QTime>

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

    void on_SetPlateAngleBut_clicked();

    void on_GetCurPlatesAnglesBut_clicked();

    void on_SetTimeoutBut_clicked();

    void on_GetTimeoutBut_clicked();

    void on_GetSignalLevelBut_clicked();

    void on_GetLightNoisesBut_clicked();

    void on_SendMessageBut_clicked();

    void ConsoleLog(QString text);

    void ConsoleLog(QString text, bool bad);

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

    void on_PDRatio_clicked();

    void on_LaserTest_clicked();

    void on_Start_protocol_clicked();

    void on_Auto_Filing_clicked();

    void on_Delete_clicked();

    void on_TestLine_clicked();

    void on_RotateAngles_clicked();

    void on_MonitoringSend_clicked();

    void on_MonitorNoises_clicked();

    void on_Test_Speed_2_clicked();

    void on_Evacheck_clicked();

    void on_Protocol_test_clicked();

    void on_Help_clicked();

    void on_PlateAngle_clicked();

    void on_Eva_key_clicked();

    void on_Error_key_clicked();

    void on_Test_monitor_clicked();

    void on_TestTimebut_clicked();

    void on_CreateConfigSecret_clicked();

    void on_SetComPortName_clicked();

    void on_OpenConfigMode_clicked();

    void on_CloseConfigMode_clicked();

    void on_GetProtocolVersion_clicked();

    void on_GetProtocolFirmwareVersion_clicked();

    void on_GetCurrentMode_clicked();

    void on_GetComPortName_clicked();

    void on_GetMaxPayloadSize_clicked();

    void on_FirmwareUpdate_clicked();

    void on_StopLaser2_clicked();

    void slotTimerAlarm();

    void on_radio_admin_clicked();

    void on_histogram_clicked();

    void on_radio_ElectionPD_v2_clicked();


private:
    QTimer *timer;
    float AngleCheck (float angle, float step);
    QString ElectionPD(int PDH, int PDV, int yh_, int yv_);
    QString ElectionPD_v2(int PDH, int PDV, int yh_, int yv_, int MaxSig_h);
    QStringList ConvertingArray (QString str);
    int randomBetween(int low, int high);
    QStringList Protocol (QStringList  AliceBit, QStringList  AliceBasis, QStringList  BobBit, QStringList  BobBasis);
    QStringList Protocol_Eva (QStringList  AliceBit, QStringList  AliceBasis, QStringList EvaBasis, QStringList  BobBit, QStringList  BobBasis);
    QStringList Random(int n);
    void ParamAngles();
    Ui::MainWindow *ui;
    HelpWindow *clc;

    hwe::Conserial stand_;
    Histogram hst;
    HistogramEva eva;
    bool Flag_;
    QVector<double> x,y1,y2,y3;
    QShortcut       *keyEnter;    // объект горячей клавиши Enter
};
#endif // MAINWINDOW_H
