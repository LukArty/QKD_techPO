#include "histogrameva.h"
#include "ui_histogrameva.h"
#include "qcustomplot.h"

HistogramEva::HistogramEva(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HistogramEva)
{
    ui->setupUi(this);
    ui->horizontalScrollBar->setRange(800, 1600);

    // инициализируем диапазон осей
    ui->histogram_AE->xAxis->setRange(0,16);
    ui->histogram_AE->yAxis->setRange(0,1);
    ui->histogram_EB->xAxis->setRange(0,16);
    ui->histogram_EB->yAxis->setRange(0,1);

    //создаем соединение между осями и полосой прокрутки
    connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horzScrollBarChanged(int)));
    connect(ui->histogram_AE->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
    connect(ui->histogram_EB->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));
}

HistogramEva::~HistogramEva()
{
    delete ui;
}

void HistogramEva::horzScrollBarChanged(int value)
{
    if (qAbs(ui->histogram_AE->xAxis->range().center()-value/100.0) > 0.01)
    {
        ui->histogram_AE->xAxis->setRange(value/100.0, ui->histogram_AE->xAxis->range().size(), Qt::AlignCenter);
        ui->histogram_AE->replot();
        ui->histogram_EB->xAxis->setRange(value/100.0, ui->histogram_EB->xAxis->range().size(), Qt::AlignCenter);
        ui->histogram_EB->replot();
    }
}

void HistogramEva::xAxisChanged(QCPRange range)
{
    ui->horizontalScrollBar->setValue(qRound(range.center()*100.0)); /// отрегулировать положение ползунка
    ui->horizontalScrollBar->setPageStep(qRound(range.size()*100.0)); /// настроить размер ползунка полосы прокрутки
}

void HistogramEva::BiuldHistogram (QStringList pdhae, QStringList pdvae, QStringList pdheb, QStringList pdveb){

    //очистка поля под график
    ui->histogram_AE->clearGraphs();
    ui->histogram_EB->clearGraphs();
    x1.clear();
    x2.clear();
    y1.clear();
    y2.clear();

    //отрисовка графика Алиса - Ева
    ui->histogram_AE->addGraph();
    ui->histogram_AE->graph(0)->setPen(QPen(Qt::blue));
    ui->histogram_AE->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->histogram_AE->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram_AE->addGraph();
    ui->histogram_AE->graph(1)->setPen(QPen(Qt::red));
    ui->histogram_AE->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->histogram_AE->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram_AE->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
    ui->histogram_AE->axisRect()->setRangeZoom(Qt::Horizontal);   //удаление/приближение только по горизонтальной оси
    ui->histogram_AE->graph(0)->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
    ui->histogram_AE->graph(0)->setLineStyle(QCPGraph::lsImpulse); // График в виде импульсных тиков
    ui->histogram_AE->graph(1)->setAntialiased(false);
    ui->histogram_AE->graph(1)->setLineStyle(QCPGraph::lsImpulse);

    //отрисовка графика Ева - Боб
    ui->histogram_EB->addGraph();
    ui->histogram_EB->graph(0)->setPen(QPen(Qt::blue));
    ui->histogram_EB->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->histogram_EB->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram_EB->addGraph();
    ui->histogram_EB->graph(1)->setPen(QPen(Qt::red));
    ui->histogram_EB->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->histogram_EB->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));

    ui->histogram_EB->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
    ui->histogram_EB->axisRect()->setRangeZoom(Qt::Horizontal);   //удаление/приближение только по горизонтальной оси
    ui->histogram_EB->graph(0)->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
    ui->histogram_EB->graph(0)->setLineStyle(QCPGraph::lsImpulse); // График в виде импульсных тиков
    ui->histogram_EB->graph(1)->setAntialiased(false);
    ui->histogram_EB->graph(1)->setLineStyle(QCPGraph::lsImpulse);

    ui->horizontalScrollBar->setRange(800,(pdvae.size()-8)*100); //граници для скролла от 0 до кол-во битов в массиве
    double probaH, probaV;
    double summa;

    for(int i = 0; i < pdhae.size(); i++){
        //Для графика Алиса - Ева
        summa = pdhae[i].toDouble() + pdvae[i].toDouble();
        probaH = (pdhae[i].toDouble())/summa;
        probaV = (pdvae[i].toDouble())/summa;

        x1.push_back(i+0.1);
        x2.push_back(i+0.4);
        y1.push_back(probaH);
        y2.push_back(probaV);

        ui->histogram_AE->graph(0)->setData(x1,y1);
        ui->histogram_AE->graph(1)->setData(x2,y2);
        ui->histogram_AE->axisRect()->setupFullAxesBox(true);
        ui->histogram_AE->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//возможность перемещать график по горизонтальной оси и возможность удаление/приближение
        ui->histogram_AE->replot();

        //Для графика Ева - Боб
        summa = pdheb[i].toDouble() + pdveb[i].toDouble();
        probaH = (pdheb[i].toDouble())/summa;
        probaV = (pdveb[i].toDouble())/summa;

        y3.push_back(probaH);
        y4.push_back(probaV);

        ui->histogram_EB->graph(0)->setData(x1,y3);
        ui->histogram_EB->graph(1)->setData(x2,y4);
        ui->histogram_EB->axisRect()->setupFullAxesBox(true);
        ui->histogram_EB->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//возможность перемещать график по горизонтальной оси и возможность удаление/приближение
        ui->histogram_EB->replot();
    }

}
