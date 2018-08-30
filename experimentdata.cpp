#include "ExperimentData.h"
//#include <Vector.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

ExperimentData::ExperimentData()
{
     data = 0;
}

ExperimentData::ExperimentData(QString fileName)
{

}

ExperimentData::ExperimentData(QString sxn, double width, double height, double fy, double E)
    :sxn(thesxn), width(thewidth), height(theheight), fy(0), E(0)
{
    data = theData;
}

ExperimentData::~ExperimentData()
{
    if (data != 0)
        delete data;
}

void ExperimentData::inputFromJSON(QJsonObject &jsonObj)
{
    QJsonValue theValue = jsonObj["sxn"];
    name=theValue.toString();
    theValue = jsonObj["width"];
    dt=theValue.toDouble();
    theValue = jsonObj["height"];
    dt=theValue.toDouble();
    theValue = jsonObj["fy"];
    dt=theValue.toDouble();
    theValue = jsonObj["E"];
    dt=theValue.toDouble();

    //if (data != 0)
    //    delete [] data;
    //data = new Vector(numSteps);

    //theValue = jsonObj["data"];
    //QJsonArray dataPoints = theValue.toArray();
    //for (int i=0; i<numSteps; i++)
    //    (*data)[i] = dataPoints.at(i).toDouble();
}
