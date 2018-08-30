#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QString>
class QJsonValue;

class Experiment
{
private:
    int numSteps;
    double dt;
    QVector<double> *time;
    QVector<double> *dataP;
    QVector<double> *dataD;

    void setTime(QVector<double> *data);
    void interpData();

public:
    Experiment();
    ~Experiment();

    int inputFromJSON(QJsonValue &json);
    int getNumSteps(void);
    QVector<double>* getDataP(void);
    QVector<double>* getDataD(void);
    QVector<double>* getTime(void);
    double getdt(void);
    //void setDataD(QVector<double>* inU);
};

#endif // EXPERIMENT_H
