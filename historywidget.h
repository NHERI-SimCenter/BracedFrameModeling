#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QVector>

class QCustomPlot;
class QCPGraph;

class historyWidget : public QWidget
{
    Q_OBJECT

private:
    QCustomPlot *thePlot;
    QCPGraph *graph;

    QVector<double> *data;
    QVector<double> *time;
    int steps;
    double maxVal;

public:
    historyWidget(QString xLabel, QString yLabel, QWidget *parent = 0);
    ~historyWidget();

    void setData(QVector<double> *inData, QVector<double> *inTime);
    void plotModel();
    void moveDot(double xi, double yi);

};

#endif // HISTORYWIDGET_H
