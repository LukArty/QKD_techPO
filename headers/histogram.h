#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QDialog>
#include <QVector>
#include "qcustomplot.h"

namespace Ui {
class Histogram;
}

class Histogram : public QDialog
{
    Q_OBJECT

public:
    explicit Histogram(QWidget *parent = nullptr);
    ~Histogram();
    void BiuldHistogram (QStringList pdh, QStringList pdv);

private slots:
    void horzScrollBarChanged(int value);
    void xAxisChanged(QCPRange range);

private:
    Ui::Histogram *ui;
    QVector<double> x1, x2, y1, y2;
};

#endif // HISTOGRAM_H
