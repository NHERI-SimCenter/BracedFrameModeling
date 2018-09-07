#include <cmath>
#include "experiment.h"
#include <Vector.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

double interpolate(QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate);

Experiment::Experiment()
{
    numSteps = 1;
    dataD = new QVector<double>(numSteps,0.);
    dataP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    dt = 0.1;
}

Experiment::~Experiment()
{
    if (dataD != NULL)
        delete dataD;
    if (dataP != NULL)
        delete dataP;
    if (time != NULL)
        delete time;
}

QVector<double>* Experiment::getDataP(void)
{
    dataP->prepend(0.);
    return dataP;
}

QVector<double>* Experiment::getDataD(void)
{
    dataD->prepend(0.);
    return dataD;
}

QVector<double>* Experiment::getTime(void)
{
    time->prepend(0.);
    return time;
}

double Experiment::getdt(void)
{
    return dt;
}

int Experiment::getNumSteps(void)
{
    return numSteps+1;
}

int Experiment::inputFromJSON(QJsonValue &json)
{
    int ok = 0;

    // load experiment data
    if (json.isNull() || json.isUndefined()) {
        ok = -1;

    } else {

        QJsonObject theData = json.toObject();

        // axial def
        if (theData["axialDef"].isNull() || theData["axialDef"].isUndefined()) {
            ok = -2;

        } else {
            QJsonArray theArray=theData["axialDef"].toArray();
            numSteps = theArray.size();

            dataD->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataD)[j] = theArray.at(j).toDouble();
        }

        // axial force
        if (theData["axialForce"].isNull() || theData["axialForce"].isUndefined()) {

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = 0.;

            ok =  -3;

        } else {
            QJsonArray theArray=theData["axialForce"].toArray();
            if (numSteps != theArray.size()) {
                numSteps = std::min(numSteps,theArray.size());

                ok = -4;
            }

            dataP->resize(numSteps);
            for (int j=0; j<numSteps; j++)
                (*dataP)[j] = theArray.at(j).toDouble();
        }
        interpData();
    }

    return ok;
}

void Experiment::setTime(QVector<double> *data)
{   
    time->resize(numSteps);
    (*time)[0] = 0.;

    for (int j=1; j < numSteps; j++) {
        (*time)[j] = (*time)[j-1] + fabs((*data)[j] - (*data)[j-1]);
    }
}

void Experiment::interpData()
{
    // initialize
    setTime(dataD);
    int nstep = ceil((*time)[numSteps-1]/dt);
    QVector<double> *tmpP = new QVector<double>(nstep,0.);
    QVector<double> *tmpD = new QVector<double>(nstep,0.);

    // interpolate
    for (int t=0; t<nstep; t++) {
        (*tmpP)[t] = interpolate(*time,*dataP,t*dt,true);
        (*tmpD)[t] = interpolate(*time,*dataD,t*dt,true);
    }

    // re-size
    numSteps = nstep;
    dataD->resize(numSteps);
    dataP->resize(numSteps);
    dataD = tmpD;
    dataP = tmpP;

    // re-time
    time->resize(numSteps);
    (*time)[0] = 0.;
    for (int j=1; j < numSteps; j++)
        (*time)[j] = (*time)[j-1] + dt;
}
/*
double interpolate(QVector<double> &xData, QVector<double> &yData, double x, bool extrapolate)
{
    int size = xData.size();

    // left end of interval
    int j=0;
    if (x >= xData[size-2]) // special case - beyond right end
        j = size-2;
    else
        while (x>xData[j+1])
            j++;

    // points on either side of x
    double xL = xData[j], yL = yData[j], xR = xData[j+1], yR = yData[j+1];

    // if not extrapolating
    if (!extrapolate)
    {
        if (x<xL)
            yR = yL;
        if (x>xR)
            yL = yR;
    }

    // gradient
    double dydx;
    if (xR==xL)
        dydx = 0;
    else
        dydx = (yR-yL)/(xR-xL);

    // linear interpolation
    return yL + dydx*(x - xL);
}
*/
