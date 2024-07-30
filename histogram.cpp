#include "histogram.h"
#include "ui_histogram.h"
#include "qcustomplot.h"

Histogram::Histogram(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Histogram)
{
    ui->setupUi(this);
    ui->histogram->xAxis->setRange(0, 15);
    ui->histogram->yAxis->setRange(0, 1);
    ui->horizontalScrollBar->setRange(0,20);

    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->histogram->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

}

Histogram::~Histogram()
{
    delete ui;
}

void Histogram::horzScrollBarChanged(int value)
{
    if (qAbs(ui->histogram->xAxis->range().center()-value/100.0) > 0.01) //пользователь перетаскивает график
    {
        ui->histogram->xAxis->setRange(value/100.0, ui->histogram->xAxis->range().size(), Qt::AlignLeft);
        ui->histogram->replot();
    }
}

void Histogram::xAxisChanged(QCPRange range)
{
    ui->horizontalScrollBar->setValue(qRound(range.center() *100.0)); // отрегулировать положение ползунка
    ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); // настроить размер ползунка полосы прокрутки
}

void Histogram::BiuldHistogram (QStringList pdh, QStringList pdv){

    //очистка поля под график
    ui->histogram->clearGraphs();
    x1.clear();
    x2.clear();
    y1.clear();
    y2.clear();

    ui->histogram->yAxis->setRange(0, 1);
    //отрисовка графика
    ui->histogram->addGraph();
    ui->histogram->graph(0)->setPen(QPen(Qt::blue));
    ui->histogram->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->histogram->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram->addGraph();
    ui->histogram->graph(1)->setPen(QPen(Qt::red));
    ui->histogram->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->histogram->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram->axisRect()->setRangeDrag(Qt::Horizontal);   //перетаскивание только по горизонтальной оси
    ui->histogram->axisRect()->setRangeZoom(Qt::Horizontal);   //удаление/приближение только по горизонтальной оси
    ui->histogram->graph(0)->setAntialiased(false);         // Отключаем сглаживание
    ui->histogram->graph(0)->setLineStyle(QCPGraph::lsImpulse); // График в виде импульсных тиков
    ui->histogram->graph(1)->setAntialiased(false);
    ui->histogram->graph(1)->setLineStyle(QCPGraph::lsImpulse);


    double probaH, probaV;
    double summa;

    for(int i = 0; i < pdh.size(); i++){
        summa = pdh[i].toDouble() + pdv[i].toDouble();
        probaH = (pdh[i].toDouble())/summa;
        probaV = (pdv[i].toDouble())/summa;

        x1.push_back(i+1);
        x2.push_back(i+1.3);
        y1.push_back(probaH);
        y2.push_back(probaV);

        ui->histogram->xAxis->setRange(0, pdh.size()+1);
        ui->histogram->graph(0)->addData(x1,y1);
        ui->histogram->graph(1)->addData(x2,y2);
        ui->histogram->axisRect()->setupFullAxesBox(true);
        ui->histogram->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        ui->histogram->replot();
    }
}
