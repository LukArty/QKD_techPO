#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <conserial.h>
#include <QVector>
#include <helpwindow.h>
#include <adminlogin.h>
#include <histogram.h>
#include <histogrameva.h>
 #include <QProcess>
#include <QTimer>
#include <QTime>
#include <QThread>
#include <streamwork.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
     void Flag_admin();

private slots:

    void update(int i, int size,QStringList bit, QStringList Combit, QStringList blankbit, QStringList keybit, double error);
    void update(int i, int size,QStringList bit,QStringList bit_e, QStringList Combit, QStringList blankbit, QStringList keybit, double error);
    void Date_time_hist(float,float,QStringList,QStringList);
    void Date_time_hist(float search_time,float speed,QStringList signalH_AE,QStringList signalV_AE, QStringList signalH_EB, QStringList signalV_EB);
    void Output_bit();
    /// @brief инициализация по ФД
    void InitByPD(api::InitResponse response);
    void LaserTest(float,float,int);

    void on_timer_check_clicked();
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

    void killLoop() { Flag_ = true;  number =0; }

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

    void slotTimerAlarm();

    void on_radio_admin_clicked();

    void on_histogram_clicked();

    void on_radio_ElectionPD_v2_clicked();

    void slotShortcutCtrl1();  void slotShortcutCtrl2();  void slotShortcutCtrl3();  void slotShortcutCtrl4(); void slotShortcutAdmin(); void slotShortcutGameMod();



private:
    QTimer *timer, *timer2;
    int m,s, number=0;
    float AngleCheck (float angle, float step);

    int randomBetween(int low, int high);
    QStringList Random(int n);
    void ParamAngles();
    Ui::MainWindow *ui;
    HelpWindow *clc;
    float y1_max,y2_max,y1_min, y2_min;
    hwe::Conserial stand_;
    Adminlogin *adm;
    Histogram hst;
    HistogramEva eva;
    bool Flag_;
    QVector<double> x,y1,y2,y3;
    QShortcut       *keyCNTR1, *keyCNTR2, *keyCNTR3, *keyCNTR4, *keyAdmin, *keyGameMod;    // объект горячей клавиши
    QThread pMyThread, pMyThread2;
    StreamWork *pStreamWork;
};
#endif // MAINWINDOW_H
