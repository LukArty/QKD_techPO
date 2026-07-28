#ifndef HISTOGRAMEVA_H
#define HISTOGRAMEVA_H

#include <QDialog>
#include <QVector>
#include "qcustomplot.h"

namespace Ui {
class HistogramEva;
}

class HistogramEva : public QDialog
{
    Q_OBJECT

public:
    explicit HistogramEva(QWidget *parent = nullptr);
    ~HistogramEva();

    void BiuldHistogram (QStringList pdhae, QStringList pdvae, QStringList pdheb, QStringList pdveb);

private slots:
    void horzScrollBarChanged(int value);
    void xAxisChanged(QCPRange range);

private:
    Ui::HistogramEva *ui;
    QVector<double> x1, x2, y1, y2, y3, y4;
};

#endif // HISTOGRAMEVA_H
