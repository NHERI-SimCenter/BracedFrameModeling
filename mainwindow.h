#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <math.h>
#include <map>

#include <QtGui>
#include <QtWidgets>
#include <QTCore>
#include <QDebug>
#include <qcustomplot/qcustomplot.h>

#include <Vector.h>

class Experiment;
class Resp;
class historyWidget;
class deformWidget;
class responseWidget;
class hysteresisWidget;

// organization of sxnShape
enum class sxnShape {
    W,
    M,
    S,
    HP,
    C,
    MC,
    L,
    dL,
    WT,
    MT,
    ST,
    HSS,
    RND,
    PIPE
};

// create struct for **
struct doublePointer {
    int steps;
    int size;
    QVector<double> **data;
};

// size
struct windowSize {
    int width;
    int height;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // analysis
    void buildModel();

    // custom plots
    //void dPlot(QCustomPlot *plot, int Fig);

private slots:
    // pushButton
    void addExp_clicked();
    void addAISC_clicked();
    void theAISC_sectionClicked(int row);
    void reset();
    void doAnalysis();
    void stop_clicked();
    void play_clicked();
    void pause_clicked();
    void restart_clicked();

    //connect(play,SIGNAL(clicked()), this, SLOT(play_clicked()));
    //connect(stop,SIGNAL(clicked()), this, SLOT(stop_clicked()));
    //connect(reset,SIGNAL(clicked()), this, SLOT(reset_clicked()));

    // Combo Box
    void inSxn_currentIndexChanged(int row);
    void inOrient_currentIndexChanged(int row);
    void inElType_currentIndexChanged(int row);
    void inElDist_currentIndexChanged(int row);
    void inIM_currentIndexChanged(int row);
    void inShape_currentIndexChanged(int row);
    void inMat_currentIndexChanged(int row);

    // Spin box
    void inNe_valueChanged(int var);
    void inNIP_valueChanged(int var);
    void inNbf_valueChanged(int var);
    void inNtf_valueChanged(int var);
    void inNd_valueChanged(int var);
    void inNtw_valueChanged(int var);

    // double spin box
    void inL_valueChanged(double var);
    void inDelta_valueChanged(double var);
    void inEs_valueChanged(double var);
    void infy_valueChanged(double var);
    void inb_valueChanged(double var);

    // check box
    //void includePDeltaChanged(int);
    void matOpt_checked(int);
    void matCFT_checked(int);
    void matFat_checked(int);

    // slider
    void slider_valueChanged(int value);
    //void slider_sliderPressed();
    //void slider_sliderReleased();

private:
    Ui::MainWindow *ui;

    // main layouts
    //void createHeaderBox();
    //void createFooterBox();
    void createInputPanel();
    void createOutputPanel();

    // load information
    void loadAISC();
    void setExp(Experiment *exp);
    void initialize();
    void zeroResponse();

    // plot
    void repaint();

    // load
    void loadFile(const QString &Filename);

    // initialize

    // created main layouts
    QHBoxLayout *mainLayout;
    QVBoxLayout *largeLayout;
    //QHBoxLayout *headerLayout;
    //QHBoxLayout *footerLayout;
    QGridLayout *outLay;
    QVBoxLayout *inLay;

    // Button
    QPushButton *addExp;
    QPushButton *addAISC;

    // Combo Box
    QComboBox *inExp;
    QComboBox *inSxn;
    QComboBox *inElType;
    QComboBox *inElDist;
    QComboBox *inIM;
    QComboBox *inShape;
    QComboBox *inOrient;
    QComboBox *inMat;

    // spin box    
    QSpinBox *inNe;
    QSpinBox *inNIP;
    QSpinBox *inNbf;
    QSpinBox *inNtf;
    QSpinBox *inNd;
    QSpinBox *inNtw;

    // double spin box
    QDoubleSpinBox *inL;
    QDoubleSpinBox *inDelta;
    QDoubleSpinBox *inEs;
    QDoubleSpinBox *infy;
    QDoubleSpinBox *inb;

    // check box
    QCheckBox *matOpt;
    QCheckBox *matCFT;
    QCheckBox *matFat;

    // labels
    QLabel *deltaL;
    QLabel *Alabel;
    QLabel *Ilabel;
    QLabel *Zlabel;
    QLabel *Slabel;
    QLabel *rlabel;
    QLabel *tlabel;
    QLabel *dlabel;
    QLabel *bflabel;
    QLabel *twlabel;
    QLabel *tflabel;

    // QList
    QList<QStandardItem *> sxnProp; // huh?? want to grab row of QStandardItemModel
    // AISC lists
    QStandardItemModel *AISCshapes;
    QStringList propList;
    QStringList sxnList;

    // QStrings
    QString sxn;
    QString orient;
    QString elType;
    QString elDist;
    QString IM;
    QString shape;
    QString mat;

    // enums
    sxnShape sxnType;

    // QSlider
    QSlider *slider;

    // window size
    windowSize wSize;

    // ints
    int ne;
    int nn;
    int NIP;
    int nbf;
    int ntf;
    int nd;
    int ntw;
    int stepCurr;

    // doubles
    // constants
    double pi;
    // geom
    double Lwp;
    double L;
    double delta;
    // mat
    double fy;
    double Es;
    double b;
    // sxn prop
    double A;
    double Ix;
    double Zx;
    double Sx;
    double rx;
    double Iy;
    double Zy;
    double Sy;
    double ry;
    double I;
    double Z;
    double S;
    double r;
    // sxn geom
    double d;
    double bf;
    double tw;
    double tf;
    // combactness
    double bftf;
    double htw;

    // bools
    bool inclOpt;
    bool inclCFT;
    bool inclFat;
    //bool movSlider;
    bool pause;
    bool stop;

    // coordinates
    QVector<double> xc;
    QVector<double> yc;

    // experiment
    int numSteps;
    QVector<double> *expD;
    QVector<double> *expP;
    QVector<double> *time;
    double dt;

    // response
    Resp *Ux;
    Resp *Uy;
    Resp *q1;
    Resp *q2;
    Resp *q3;

    // output widgets
    deformWidget *dPlot;
    historyWidget *tPlot;
    responseWidget *pPlot;
    responseWidget *mPlot;
    responseWidget *kPlot;
    hysteresisWidget *hPlot;
};

#endif // MAINWINDOW_H
