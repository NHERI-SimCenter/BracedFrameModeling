#ifndef HYSTERESISWIDGET_H
#define HYSTERESISWIDGET_H

#include <QWidget>
#include <QVector>
#include <QLabel>

class QCustomPlot;
class QCPGraph;
class QCPCurve;
class Resp;

class hysteresisWidget : public QWidget
{
    Q_OBJECT

private:
    QCustomPlot *thePlot;
    QCPGraph *graph;
    QCPCurve *curve1;
    QCPCurve *curve2;
    QLabel *label1;
    QLabel *label2;
    QVector<double> *xi;
    QVector<double> *yi;
    QVector<double> *xj;
    QVector<double> *yj;
    int size;
    double maxX;
    double maxY;

public:
    hysteresisWidget(QString xLabel, QString yLabel, QWidget *parent = 0);
    ~hysteresisWidget();

    void setModel(QVector<double> *data_x, QVector<double> *data_y);
    void setResp(QVector<double> *data_p, QVector<double> *data_q);
    void plotModel();
    void plotResponse(int t = 0);
};

#endif // HYSTERESISWIDGET_H
