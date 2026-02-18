#ifndef STREAMWORK_H
#define STREAMWORK_H

#include <QObject>
#include <QMainWindow>
#include <conserial.h>
#include <histogram.h>
#include <histogrameva.h>
#include <ui_mainwindow.h>


class StreamWork: public QObject
{
    Q_OBJECT
public:
    StreamWork( hwe::Conserial *stand_);
    StreamWork();

    float bHalf_1, bHalf_0, bQuart_1, bQuart_0;
    float aHalf_00, aQuart_00, aHalf_01, aQuart_01;
    float aHalf_10, aQuart_10, aHalf_11, aQuart_11;
    int PV_11, PV_10, PV_01, PV_00;
    int PH_11, PH_10, PH_01, PH_00;
    bool ElectionPD_;

    QStringList AliceBit, AliceBasis, BobBit, BobBasis, EvaBasis;

public slots:
    QString ElectionPD(int PDH, int PDV, int yh_, int yv_);
    QString ElectionPD_v2(int PDH, int PDV, int yh_, int yv_, int MaxSig_h);
    QStringList Protocol ();
    QStringList Protocol_Eva ();
    QStringList ConvertingArray (QString str);

    void InitByPD();
    void LaserTest();
    void killLoop() { flag = false;}
signals:
    void emitdate(int, int,QStringList, QStringList, QStringList, QStringList, double);
    void emitdate(int,int,QStringList,QStringList,QStringList,QStringList, QStringList, double);
    void emitdate(float,float,QStringList,QStringList); //сигнал для передачи времени, скорости и значений для гистограммы
    void emitdate_eva(float,float,QStringList,QStringList,QStringList,QStringList); //сигнал для передачи времени, скорости и значений для гистограммы
    void finished();    // Сигнал, по которому будем завершать поток
    void finished1(float, float, float,float, int, int, int, int, int, int);
    void emitdate(float,float, int);


private slots:



    void on_SetLaserStateBut_clicked();

    void on_file_selection_clicked();

private:
    Ui::MainWindow *ui;
    hwe::Conserial *stand_;
    Histogram hst;
    HistogramEva eva;
    bool flag = true;
};

#endif // STREAMWORK_H
