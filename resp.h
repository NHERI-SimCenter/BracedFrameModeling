#ifndef RESP_H
#define RESP_H

#include <QString>

class Resp
{
public:
    Resp();
    Resp(int inSize, int inSteps);
    ~Resp();

    int steps;
    int size;
    QVector<double> **data;

    void reStep(int newSteps);
    void reSize(int newSize);
    void reSize(int newSize, int newSteps);
    void zero();
};

#endif // RESP_H
