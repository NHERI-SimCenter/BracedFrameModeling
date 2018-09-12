#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QString>
class QJsonValue;

class Experiment
{
private:
    int numSteps;
    double dt;
    QString testType; /*!< String description of test type */
    QVector<double> *time;
    QVector<double> *dataP;
    QVector<double> *dataD;

    void setTime(QVector<double> *data);
    void interpData();

public:
    Experiment();
    ~Experiment();

    /*!
     * Get the type of test
     * @return A QString describing the test type
     */
    QString getTestType() const;
    
    int inputFromJSON(QJsonValue &json);
    int getNumSteps(void);
    QVector<double>* getDataP(void);
    QVector<double>* getDataD(void);
    QVector<double>* getTime(void);
    double getdt(void);
    //void setDataD(QVector<double>* inU);
};

#endif // EXPERIMENT_H
