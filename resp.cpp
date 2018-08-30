#include "resp.h"
#include <QVector>

Resp::Resp()
{
    steps = 0;
    size = 0;
    data = NULL;
}

Resp::Resp(int inSize, int inSteps)
{
    size = inSize;
    steps = inSteps;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

Resp::~Resp()
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }
}

void Resp::reStep(int newSteps)
{
    steps = newSteps;
    for (int j=0; j<steps; j++) {
        data[j]->resize(newSteps);
    }
    zero();
}

void Resp::reSize(int newSize)
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }

    size = newSize;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

void Resp::reSize(int newSize, int newSteps)
{
    if (data != NULL) {
        for (int j=0; j < size; j++) {
            delete data[j];
        }
        delete [] data;
    }

    size = newSize;
    steps = newSteps;

    data = new QVector<double> *[size];
    for (int j=0; j<size; j++) {
        data[j] = new QVector<double>(steps,0.);
    }
}

void Resp::zero()
{
    for (int j=0; j<size; j++) {
        for (int k=0; k<steps; k++) {
            (*data[j])[k] = 0.;
        }
    }
}
