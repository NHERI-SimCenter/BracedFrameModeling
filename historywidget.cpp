#include "historywidget.h"
#include <qcustomplot/qcustomplot.h>

historyWidget::historyWidget(QString xLabel, QString yLabel, QWidget *parent)
              : QWidget(parent)
{   
    steps = 2;
    data = new QVector<double>(steps,0.);
    time = new QVector<double>(steps,0.);

    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    thePlot->setMinimumHeight(0.2*height);
    thePlot->setMaximumHeight(0.2*height);
    thePlot->xAxis->setLabel(xLabel);
    thePlot->yAxis->setLabel(yLabel);

    // axes
    thePlot->xAxis2->setVisible(true);
    thePlot->yAxis2->setVisible(true);
    thePlot->xAxis->setTicks(false);
    thePlot->yAxis->setTicks(false);
    thePlot->xAxis->setSubTicks(false);
    thePlot->yAxis->setSubTicks(false);
    thePlot->xAxis2->setTickLabels(false);
    thePlot->yAxis2->setTickLabels(false);
    thePlot->xAxis2->setTicks(false);
    thePlot->yAxis2->setTicks(false);
    thePlot->xAxis2->setSubTicks(false);
    thePlot->yAxis2->setSubTicks(false);
    thePlot->axisRect()->setAutoMargins(QCP::msNone);
    thePlot->axisRect()->setMargins(QMargins(1,1,1,1));

    // aff to layout
    QGridLayout *Lay = new QGridLayout(this);
    Lay->addWidget(thePlot,0,0);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

historyWidget::~historyWidget()
{
    delete thePlot;
    delete graph;

    delete data;
    delete time;
}

void historyWidget::setData(QVector<double> *inData, QVector<double> *inTime)
{
    // resize
    steps = inData->size();
    data->resize(steps);
    time->resize(steps);

    // re-allocate
    data = inData;
    time = inTime;

    // max
    maxVal = 0.;
    for (int j=0; j < steps; j++) {
        double val = (*data)[j];
        if (fabs(val) > maxVal)
            maxVal = fabs(val);
    }

    if (maxVal==0)
        maxVal = 1;

    plotModel();
}

void historyWidget::plotModel()
{
    // setup system plot
    thePlot->clearPlottables();
    thePlot->clearGraphs();
    //thePlot->clearItems();

    // add graph
    graph = thePlot->addGraph();

    // create pen
    QPen pen;
    pen.setWidthF(2);

    // line color
    pen.setColor(QColor(Qt::gray));
    thePlot->graph(0)->setPen(pen);
    thePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 1));
    thePlot->graph(0)->setLineStyle(QCPGraph::lsNone);

    // set data
    thePlot->graph(0)->setData(*time,*data,true);

    // axes
    thePlot->xAxis->setRange(0,(*time)[steps-1]);
    thePlot->yAxis->setRange(-maxVal, maxVal);

    // tracer
    thePlot->addGraph();
    pen.setColor(QColor(Qt::red));
    thePlot->graph(1)->setPen(pen);
    thePlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));

    // trace
    //QVector<double> z = {0.};
    //thePlot->graph(1)->setData(z,z);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();
}

void historyWidget::moveDot(double xi, double yi)
{
    // new coords
    QVector<double> x = {xi};
    QVector<double> y = {yi};

    // create graph
    plotModel();
    thePlot->graph(1)->setData(x,y,true);

    // plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    thePlot->update();
}
