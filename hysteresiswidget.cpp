#include "hysteresiswidget.h"
#include <qcustomplot/qcustomplot.h>
#include "resp.h"
#include <QLabel>

hysteresisWidget::hysteresisWidget(QString xLabel, QString yLabel, QWidget *parent)
                    : QWidget(parent)
{
    // initialize
    size = 2;
    xi = new QVector<double>(size,0.);
    yi = new QVector<double>(size,0.);
    xj = new QVector<double>(size,0.);
    yj = new QVector<double>(size,0.);

    // setup plot
    thePlot = new QCustomPlot();
    thePlot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    thePlot->setMinimumHeight(0.35*width);
    //thePlot->setMaximumHeight(0.3*width);
    thePlot->setMinimumWidth(0.35*width);
    //thePlot->setMaximumWidth(0.3*width);
    thePlot->xAxis->setLabel(xLabel);
    thePlot->yAxis->setLabel(yLabel);

    // axes
    thePlot->xAxis2->setVisible(true);
    thePlot->yAxis2->setVisible(true);
    thePlot->xAxis->setTicks(true);
    thePlot->yAxis->setTicks(true);
    thePlot->xAxis->setSubTicks(false);
    thePlot->yAxis->setSubTicks(false);
    thePlot->xAxis2->setTickLabels(false);
    thePlot->yAxis2->setTickLabels(false);
    thePlot->xAxis2->setTicks(false);
    thePlot->yAxis2->setTicks(false);
    thePlot->xAxis2->setSubTicks(false);
    thePlot->yAxis2->setSubTicks(false);
    //thePlot->axisRect()->setAutoMargins(QCP::msNone);
    //thePlot->axisRect()->setMargins(QMargins(1,1,1,1));

    // print current value...
    label1 = new QLabel;
    label2 = new QLabel;
    label1->setAlignment(Qt::AlignLeft);
    label2->setAlignment(Qt::AlignLeft);

    // add to layout
    QGridLayout *Lay = new QGridLayout(this);
    Lay->addWidget(thePlot,0,0,1,2);
    Lay->addWidget(label1,1,0);
    Lay->addWidget(label2,1,1);
    Lay->setMargin(0);
    this->setLayout(Lay);
}

hysteresisWidget::~hysteresisWidget()
{
    delete thePlot;
    delete graph;
    delete curve1;
    delete curve2;
    delete xi;
    delete xj;
    delete yi;
    delete yj;
}

void hysteresisWidget::setModel(QVector<double> *data_x, QVector<double> *data_y)
{
    size = data_x->size();

    // initialize
    xi->resize(size);
    yi->resize(size);
    xj->resize(size);
    yj->resize(size);

    // set
    xi = data_x;
    yi = data_y;

    // max -X
    maxX = 0.;
    for (int j=0; j < size; j++) {
        double val = (*xi)[j];
        if (fabs(val) > maxX)
            maxX = fabs(val);
    }

    // max -Y
    maxY = 0.;
    for (int j=0; j < size; j++) {
        double val = (*yi)[j];
        if (fabs(val) > maxY)
            maxY = fabs(val);
    }
}

void hysteresisWidget::setResp(QVector<double> *data_p, QVector<double> *data_q)
{
    size = data_p->size();

    // re-size
    xj->resize(size);
    yj->resize(size);

    // store
    for (int j=0; j < size; j++) {
        (*xj)[j] = (*data_p)[j];
        (*yj)[j] = (*data_q)[j];
    }

    // max -X
    for (int j=0; j < size; j++) {
        double val = (*xj)[j];
        if (fabs(val) > maxX)
            maxX = fabs(val);
    }

    // max -Y
    for (int j=0; j < size; j++) {
        double val = (*yj)[j];
        if (fabs(val) > maxY)
            maxY = fabs(val);
        }
}

void hysteresisWidget::plotModel()
{
    thePlot->clearPlottables();
    //thePlot->clearGraphs();
    //thePlot->clearItems();
    thePlot->legend->setVisible(true);
    
    // create curves
    curve1 = new QCPCurve(thePlot->xAxis, thePlot->yAxis);
    curve2 = new QCPCurve(thePlot->xAxis, thePlot->yAxis);

    curve1->setName(tr("Experimental"));
    curve2->setName(tr("Simulation"));

    // create pen
    QPen pen;
    pen.setWidthF(3);
    pen.setColor(QColor(Qt::gray));
    curve1->setPen(pen);
    pen.setColor(QColor(Qt::blue));
    curve2->setPen(pen);

    // set data
    // curve1->setData(*xi,*yi);

    // axes
    thePlot->xAxis->setRange(-maxX-1,maxX+1);
    thePlot->yAxis->setRange(-maxY-10,maxY+10);

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    //thePlot->update();

    // update label
    //label->setText(QString("current = %1 in.").arg(maxY,0,'f',2));
}

void hysteresisWidget::plotResponse(int t)
{
    // re-plot baseline
    plotModel();

    // set data
    curve1->setData(xi->mid(0,t),yi->mid(0,t));    
    curve2->setData(xj->mid(0,t),yj->mid(0,t));

    // update plot
    thePlot->replot(QCustomPlot::rpQueuedReplot);
    //thePlot->update();

    // update label
    label1->setText(QString("Experiment: (%1 in., %2 kips)").arg((*xi)[t],0,'f',1).arg((*yi)[t],0,'f',0));
    label2->setText(QString("Simulation: (%1 in., %2 kips)").arg((*xj)[t],0,'f',1).arg((*yj)[t],0,'f',0));
}
