#ifndef RESPONSEWIDGET_H
#define RESPONSEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>

class QCustomPlot;
class QCPGraph;
class Resp;

class responseWidget : public QWidget
{
    Q_OBJECT

private:
    QCustomPlot *thePlot;
    QCPGraph *graph;
    QLabel *label;
    QVector<double> *xi;
    QVector<double> *yi;
    Resp *p;
    Resp *q;
    int steps;
    int size;
    int els;
    double maxX;
    double minX;
    double maxY;
    double minY;

public:
    responseWidget(QString xLabel, QString yLabel, QWidget *parent = 0);
    ~responseWidget();

    void setModel(QVector<double> *data_x);
    void setResp(Resp *data_p, Resp *data_q);
    void plotModel();
    void plotResponse(int t = 0);
};

#endif // RESPONSEWIDGET_H
