#ifndef EXPERIMENTDATA_H
#define EXPERIMENTDATA_H

#include <QString>
class Vector;
class QJsonObject;

class ExperimentData
{
public:
    ExperimentData();
    ExperimentData(QString fileName);
    ExperimentData(QString sxn, double width, double height, double fy, double E); // Vector *data);
    ~ExperimentData();

    void outputToJSON(QJsonObject &jsonObj);
    int inputFromJSON(QJsonObject &jsonObj);

    QString sxn;
    double width;
    double height;
    double fy;
    double E;
    Vector *data;
};

#endif // EXPERIMENTDATA_H

