#include "mainwindow.h"
#include "ui_mainwindow.h"

// layouts
//#include <HeaderWidget.h>
//#include <FooterWidget.h>
//#include "sectiontitle.h"

// custom
//#include <SimpleSpreadsheetWidget.h>
#include <qcustomplot/qcustomplot.h>
#include "experiment.h"
#include "resp.h"
#include "historywidget.h"
#include "deformwidget.h"
#include "responsewidget.h"
#include "hysteresiswidget.h"

// widget libraries
#include <QtGui>
#include <QtWidgets>
#include <QTCore>
#include <QDebug>

#include <Response.h>
#include <Information.h>
// other
#include <math.h>

// files
#include <QFile>
//#include <QtSql>
//#include <QMap>

//styles
//#include <QGroupBox>
//#include <QFrame>

// OpenSees include files
#include <Node.h>
#include <SP_Constraint.h>
#include <MP_Constraint.h>

// other
#include <Vector.h>
#include <Matrix.h>
#include <ID.h>
//#include <SimulationInformation.h>

// materials
#include <Steel01.h>
#include <Steel02.h>
#include <Steel4.h>

// integration
#include <LegendreBeamIntegration.h>
#include <LobattoBeamIntegration.h>

// transformation
#include <CorotCrdTransf2d.h>
#include <LinearCrdTransf3d.h>

// fiber
//#include <ElasticSection2d.h>
#include <QuadPatch.h>
#include <CircPatch.h>

// section
#include <UniaxialFiber2d.h>
#include <FiberSection2d.h>

// elements
#include <DispBeamColumn2d.h>
#include <ForceBeamColumn2d.h>
#include <CorotTruss.h>
#include <ElasticBeam2d.h>
#include <ZeroLength.h>

// patterns
#include <LinearSeries.h>
#include <NodalLoad.h>
#include <LoadPattern.h>
//#include <PathSeries.h>
#include <PathTimeSeries.h>
//#include <GroundMotion.h>
//#include <UniformExcitation.h>

// fortran libraries
#ifdef _FORTRAN_LIBS
#include <BandGenLinSOE.h>
#include <BandGenLinLapackSolver.h>
#endif

// solver
#include <Newmark.h>
#include <LoadControl.h>
#include <DisplacementControl.h>
#include <RCM.h>
#include <PlainNumberer.h>
#include <NewtonRaphson.h>
#include <NewtonLineSearch.h>
#include <KrylovNewton.h>
//#include <CTestNormDispIncr.h>
#include <CTestEnergyIncr.h>
#include <PlainHandler.h>
#include <PenaltyConstraintHandler.h>
#include <TransformationConstraintHandler.h>
#include <ProfileSPDLinDirectSolver.h>
#include <ProfileSPDLinSOE.h>
//#include <DirectIntegrationAnalysis.h>
#include <StaticAnalysis.h>
#include <AnalysisModel.h>
#include <SymBandEigenSOE.h>
#include <SymBandEigenSolver.h>

// OpenSees
#include "Domain.h"
#include "StandardStream.h"
StandardStream sserr;
OPS_Stream *opserrPtr = &sserr;
Domain theDomain;

//---------------------------------------------------------------
// Misc. functions
QCheckBox *addCheck(QString text, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QComboBox *addCombo(QString text, QStringList items, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);
QSpinBox *addSpin(QString text, QString *unitText =0,
           QGridLayout *gridLay =0, int row =-1, int col =-1, int nrow =1, int ncol =1);

//---------------------------------------------------------------
// structures
// fiber pointer
struct fiberPointer {
    int fill;
    Fiber **data;
};

//---------------------------------------------------------------
// new window
QWidget *createNewWindow(QString title)
{
    QWidget *window = new QWidget;
    window->show();
    window->setWindowTitle(title);

    return window;
}

// constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // constants
    pi = 4*atan(1);

    // create layout
    mainLayout = new QHBoxLayout();
    largeLayout = new QVBoxLayout();

    /*/ create header, footer
    createHeaderBox();
    createFooterBox();
    */

    // load
    loadAISC();

    // create input / output panels
    createInputPanel();
    createOutputPanel();

    // main widget set to screen size
    QWidget *widget = new QWidget();
    widget->setLayout(largeLayout);
    this->setCentralWidget(widget);

    // screen size
    QRect rec = QApplication::desktop()->screenGeometry();
    wSize.height = 0.7*rec.height();
    wSize.width = 0.9*rec.width();
    this->resize(wSize.width, wSize.height);

    // initialize data
    initialize();
    reset();
}
//---------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
    delete AISCshapes;

    // experiment
    delete time;
    delete expD;
    delete expP;

    // response
    delete Ux;
    delete Uy;
    delete q1;
    delete q2;
    delete q3;
}
//---------------------------------------------------------------
void MainWindow::initialize()
{
    // initialize loading
    numSteps = 2;

    // experimental data
    expD = new QVector<double>(numSteps,0.);
    expP = new QVector<double>(numSteps,0.);
    time = new QVector<double>(numSteps,0.);
    dt = 0.1;

    // response
    Ux = new Resp();
    Uy = new Resp();
    q1 = new Resp();
    q2 = new Resp();
    q3 = new Resp();

    // slider
    slider->setValue(1);

    // spinBox
    inNe->setValue(10);
    inNIP->setValue(10);
    inNbf->setValue(10);
    inNd->setValue(10);
    inNtw->setValue(10);
    inNtf->setValue(10);

    // initialize QComboBoxes
    inOrient->setCurrentIndex(1);
    inSxn->setCurrentIndex(1);
    inElType->setCurrentIndex(1);
    inElDist->setCurrentIndex(1);
    inIM->setCurrentIndex(1);
    inShape->setCurrentIndex(1);
    inMat->setCurrentIndex(1);
}

// reset
void MainWindow::reset()
{
    stop = false;

    // remove experiment name
    inExp->clear();
    inExp->addItem("");

    // initialize QComboBoxes
    inOrient->setCurrentIndex(0);
    inSxn->setCurrentIndex(0);
    inElType->setCurrentIndex(0);
    inElDist->setCurrentIndex(0);
    inIM->setCurrentIndex(0);
    inShape->setCurrentIndex(0);
    inMat->setCurrentIndex(0);

    // spin box
    // reset to trigger valueChanged
    inNe->setValue(2);
    inNIP->setValue(5);
    inNbf->setValue(12);
    inNd->setValue(1);
    inNtw->setValue(2);
    inNtf->setValue(1);

    // reset to trigger valueChanged
    inL->setValue(100.0);
    inDelta->setValue(0.2);
    inEs->setValue(29000.0);
    infy->setValue(55.0);
    inb->setValue(.1);

    // check box
    matOpt->setChecked(true);
    matCFT->setChecked(false);
    matFat->setChecked(false);

    // initialize experiment
    Experiment *exp = new Experiment();
    setExp(exp);
}

// read file
void MainWindow::loadFile(const QString &Filename)
{
    // open files
    QFile mFile(Filename);

    // open warning
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(Filename), mFile.errorString()));
        return;
    }

    // place file contents into json object
    QString mText = mFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(mText.toUtf8());
    if (doc.isNull() || doc.isEmpty()) {
        QMessageBox::warning(this, tr("Application"),
                tr("Error loading file: not a JSON file or is empty."));
        return;
    }
    QJsonObject jsonObject = doc.object();

    // clean up / initialize
    // to do -> set response to zero

    // read brace
    QJsonValue json = jsonObject["brace"];

    // load brace data
    if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Brace data not specified.");

    else {
        QJsonObject theData = json.toObject();

        // brace section
        if (theData["sxn"].isNull() || theData["sxn"].isUndefined())
            QMessageBox::warning(this, "Warning","Section not specified.");

        else {
            sxn = theData["sxn"].toString();
            int index = inSxn->findText(sxn);

            if (index != -1) {
                inSxn->setCurrentIndex(index);

            } else
                QMessageBox::warning(this, "Warning","Loaded section not in current AISC Shape Database.");
        }

        // brace length
        if ((theData["width"].isNull())     || (theData["height"].isNull())
          || theData["width"].isUndefined() || theData["height"].isUndefined())
            QMessageBox::warning(this, "Warning","Brace length not specified.");

        else {
            double W = theData["width"].toDouble();
            double H = theData["height"].toDouble();

            Lwp = sqrt(pow(W,2)+pow(H,2));
            inL->setValue(Lwp);
        }
    }

    // read brace material
    json = jsonObject["material"];

    // load brace material
    if (json.isNull() || json.isUndefined())
        QMessageBox::warning(this, "Warning","Brace material data not specified.");

    else {
        QJsonObject theData = json.toObject();

        // fy
        if (theData["fy"].isNull() || theData["fy"].isUndefined())
            QMessageBox::warning(this, "Warning","Material: fy not specified.");

        else {
            fy=theData["fy"].toDouble();
            infy->setValue(fy);
        }

        // Es
        if (theData["E"].isNull() || theData["E"].isUndefined())
            QMessageBox::warning(this, "Warning","Material: Es not specified.");

        else {
            Es=theData["E"].toDouble();
            inEs->setValue(Es);
        }
    }

    // read experiment loading
    json = jsonObject["test"];

    Experiment *exp = new Experiment();
    int ok = exp->inputFromJSON(json);

    if (ok == -1)
        QMessageBox::warning(this, "Warning","Experiment loading not specified.");
    else if (ok == -2)
        QMessageBox::warning(this, "Warning","Experiment history: axial deformation not specified.");
    else if (ok == -3)
        QMessageBox::warning(this, "Warning","Experiment history: axial force not specified.");
    else if (ok == -4)
        QMessageBox::warning(this, "Warning","Loaded axial force and deformation history are not the same length. Histories are truncated accordingly.");

    setExp(exp);

    // name experiment
    QString name = Filename.section("/", -1, -1);

    // set as current
    inExp->addItem(name);
    int index = inExp->findText(name);
    inExp->setCurrentIndex(index);
    inExp->removeItem(!index);

    // close file
    mFile.close();
}

// load experiment
void MainWindow::addExp_clicked()
{
    // call load file function
    QString Filename = QFileDialog::getOpenFileName(this);
    if (!Filename.isEmpty())
        loadFile(Filename);
}
//---------------------------------------------------------------
// load AISC Shape Database
void MainWindow::loadAISC()
{
    // resource name
    QString Filename = ":/MyResources/aisc-shapes-database-v15.0.csv";

    // open file
    QFile mFile(Filename);
    if (!mFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(Filename), mFile.errorString()));
        return;
    }

    // creat csv model
    //AISCshapes = new QTableWidget(this);
    AISCshapes = new QStandardItemModel(this);

    // read file
    QTextStream mText(&mFile);
    while (!mText.atEnd())
    {
        // read line-by-line
        QString line = mText.readLine();

        // add to list
        QList<QStandardItem *> newItem;
        for (QString item : line.split(","))
        {
            newItem.append(new QStandardItem(item));
        }

        // add to model
        AISCshapes->insertRow(AISCshapes->rowCount(),newItem);
    }

    // properties
    QList<QStandardItem *> AISCprop = AISCshapes->takeRow(0);
    for (int i = 1; i < AISCprop.size(); i++)
    {
        propList.append(AISCprop.at(i)->text());
    }

    // sections
    QList<QStandardItem *> AISCsxn = AISCshapes->takeColumn(0);
    for (int i = 0; i < AISCsxn.size(); i++)
    {
        sxnList.append(AISCsxn.at(i)->text());
    }

    // headers
    AISCshapes->setHorizontalHeaderLabels(propList);
    AISCshapes->setVerticalHeaderLabels(sxnList);

    // close file
    mFile.close();
}

// AISC button
void MainWindow::addAISC_clicked()
{
    QTableView *table = new QTableView;
    table->setModel(AISCshapes);
    table->setWindowTitle("AISC Shape Database");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->selectionModel()->selectedRows();
    table->show();
    int index = inSxn->findText(sxn);
    if (index != -1) {
        table->selectRow(index);
    }

    // size
    table->resize(0.8*wSize.width, 0.8*wSize.height);

    // connect signals / slots
    connect(table->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(theAISC_sectionClicked(int)));
}

// select section in AISC table
void MainWindow::theAISC_sectionClicked(int row)
{
    inSxn->setCurrentIndex(row);
}
//---------------------------------------------------------------


// sxn Combo-Box
void MainWindow::inSxn_currentIndexChanged(int row)
{
    sxn = sxnList.at(row);

    // read sxn shape
    QString sxnString = AISCshapes->item(row,propList.indexOf("Type"))->text();

    // parse by enum class
    if (sxnString == "W" || sxnString == "M" || sxnString == "S" || sxnString == "HP")
        sxnType = sxnShape::W;
    else if (sxnString == "C" || sxnString == "MC")
        sxnType = sxnShape::C;
    else if (sxnString == "L" || sxnString == "2L")
        sxnType = sxnShape::L;
    else if (sxnString == "WT" || sxnString == "MT" || sxnString == "ST")
        sxnType = sxnShape::WT;
    else if (sxnString == "HSS")
        sxnType = sxnShape::HSS;
    else if (sxnString == "RND" || sxnString == "PIPE")
        sxnType = sxnShape::RND;

    // properties
    A = AISCshapes->item(row,propList.indexOf("A"))->text().toDouble();
    Ix = AISCshapes->item(row,propList.indexOf("Ix"))->text().toDouble();
    Zx = AISCshapes->item(row,propList.indexOf("Zx"))->text().toDouble();
    Sx = AISCshapes->item(row,propList.indexOf("Sx"))->text().toDouble();
    rx = AISCshapes->item(row,propList.indexOf("rx"))->text().toDouble();
    Iy = AISCshapes->item(row,propList.indexOf("Iy"))->text().toDouble();
    Zy = AISCshapes->item(row,propList.indexOf("Zy"))->text().toDouble();
    Sy = AISCshapes->item(row,propList.indexOf("Sy"))->text().toDouble();
    ry = AISCshapes->item(row,propList.indexOf("ry"))->text().toDouble();

    // parse by sxn type
    switch (sxnType)
    {
    case sxnShape::W:
    case sxnShape::M:
    case sxnShape::S:
    case sxnShape::HP:
        d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::C:
    case sxnShape::MC:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("h/tw"))->text().toDouble();
        break;

    case sxnShape::L:
    case sxnShape::dL:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("b"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("t"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("b/t"))->text().toDouble();
        break;

    case sxnShape::WT:
    case sxnShape::MT:
    case sxnShape::ST:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        d = AISCshapes->item(row,propList.indexOf("d"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("bf"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("tw"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("tf"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("bf/2tf"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    case sxnShape::HSS:
        d = AISCshapes->item(row,propList.indexOf("Ht"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("B"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("b/tdes"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("h/tdes"))->text().toDouble();
        break;

    case sxnShape::RND:
    case sxnShape::PIPE:
        d = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        bf = AISCshapes->item(row,propList.indexOf("OD"))->text().toDouble();
        tw = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        tf = AISCshapes->item(row,propList.indexOf("tdes"))->text().toDouble();
        bftf = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        htw = AISCshapes->item(row,propList.indexOf("D/t"))->text().toDouble();
        break;

    default:
        d = AISCshapes->item(0,propList.indexOf("d"))->text().toDouble();
        bf = AISCshapes->item(0,propList.indexOf("bf"))->text().toDouble();
        tw = AISCshapes->item(0,propList.indexOf("tw"))->text().toDouble();
        tf = AISCshapes->item(0,propList.indexOf("tf"))->text().toDouble();
        bftf = AISCshapes->item(0,propList.indexOf("bf/2tf"))->text().toDouble();
        htw = AISCshapes->item(0,propList.indexOf("h/tw"))->text().toDouble();
    };

    // call orientation
    inOrient_currentIndexChanged(inOrient->currentIndex());
}
// orientation Combo-Box
void MainWindow::inOrient_currentIndexChanged(int row)
{
    // if round section
    if (sxnType == sxnShape::RND) {
        inNbf->setEnabled(false);
        inNtf->setEnabled(false);
        inNd->setEnabled(true);
        inNtw->setEnabled(true);
        inNbf->setMinimum(0);
        inNtf->setMinimum(0);
        inNbf->setValue(0);
        inNtf->setValue(0);
    } else if (orient != inOrient->itemText(row)) {
        orient = inOrient->itemText(row);
        inNbf->setMinimum(1);
        inNtf->setMinimum(1);

        // switch fibers
        if (orient == "x-x") {
            inNd->setEnabled(true);
            inNtf->setEnabled(true);
            inNd->setValue(nbf);
            inNtf->setValue(ntw);
            inNbf->setValue(1);
            inNtw->setValue(1);
            inNbf->setEnabled(false);
            inNtw->setEnabled(false);
        } else if (orient == "y-y") {
            inNbf->setEnabled(true);
            inNtw->setEnabled(true);
            inNbf->setValue(nd);
            inNtw->setValue(ntf);
            inNtf->setValue(1);
            inNd->setValue(1);
            inNtf->setEnabled(false);
            inNd->setEnabled(false);
        }
    }

    // define section
    if (orient == "x-x")
    {
        I = Ix;
        Z = Zx;
        S = Sx;
        r = rx;
    }
    else if (orient == "y-y")
    {
        I = Iy;
        Z = Zy;
        S = Sy;
        r = ry;
    }

    // label
    dlabel->setText(QString("d = %1 in.").arg(d));
    bflabel->setText(QString("bf = %1 in.").arg(bf));
    twlabel->setText(QString("tw = %1 in.").arg(tw));
    tflabel->setText(QString("tf = %1 in.").arg(tf));
    Alabel->setText(QString("A = %1 in<sup>2</sup>").arg(A));
    Ilabel->setText(QString("I = %1 in<sup>4</sup>").arg(I));
    Zlabel->setText(QString("Z = %1 in<sup>4</sup>").arg(Z));
    Slabel->setText(QString("S = %1 in<sup>4</sup>").arg(S));
    rlabel->setText(QString("r = %1 in<sup>3</sup>").arg(r));

    // to do: user-defined section
}
// element model type
void MainWindow::inElType_currentIndexChanged(int row)
{
    elType = inElType->itemText(row);
    if (elType == "truss")
    {
        inNe->setEnabled(false);
        inNe->setValue(1);
        inNIP->setEnabled(false);
        inNIP->setValue(2);
    } else {
        inNe->setEnabled(true);
        inNIP->setEnabled(true);
    }
}

// element distribution
void MainWindow::inElDist_currentIndexChanged(int row)
{
    elDist = inElDist->itemText(row);
    buildModel();
    if (elDist == "user-defined") {
        QMessageBox::warning(this, "Warning","User-defined nodes not yet implemented.");
        // to do
    }
}

// integration method
void MainWindow::inIM_currentIndexChanged(int row)
{
    IM = inIM->itemText(row);
}

// integration method
void MainWindow::inShape_currentIndexChanged(int row)
{
    shape = inShape->itemText(row);
    buildModel();
}

// material model
void MainWindow::inMat_currentIndexChanged(int row)
{
    mat = inMat->itemText(row);
}
//---------------------------------------------------------------
// spin-box
// number of elements
void MainWindow::inNe_valueChanged(int var)
{
    // define new
    ne = var;
    buildModel();
}
// number of IPs
void MainWindow::inNIP_valueChanged(int var)
{
    NIP = var;
    buildModel();
}
// fibers across bf
void MainWindow::inNbf_valueChanged(int var)
{
    if (nbf != var){
        nbf = var;
    }
}
// fibers across tf
void MainWindow::inNtf_valueChanged(int var)
{
    if (ntf != var){
        ntf = var;
    }
}
// fibers across d
void MainWindow::inNd_valueChanged(int var)
{
    if (nd != var){
        nd = var;
    }
}
// fibers across tw
void MainWindow::inNtw_valueChanged(int var)
{
    if (ntw != var){
        ntw = var;
    }
}
//---------------------------------------------------------------
// double spin box
// wp length
void MainWindow::inL_valueChanged(double var)
{
    if (Lwp != var) {
        Lwp = var;
        buildModel();
    }
}
// camber
void MainWindow::inDelta_valueChanged(double var)
{
    if (delta != var) {
        delta = var/100;
        buildModel();
    }
    deltaL->setText(QString("                                                        = L/%1").arg(1/(delta)));
}
// Youngs mod
void MainWindow::inEs_valueChanged(double var)
{
    if (Es != var) {
        Es = var;
    }
}
// yield strength
void MainWindow::infy_valueChanged(double var)
{
    if (fy != var) {
        fy = var;
    }
}
// strain hardening
void MainWindow::inb_valueChanged(double var)
{
    if (b != var) {
        b = var/100;
    }
}
//---------------------------------------------------------------
// check boxes
void MainWindow::matOpt_checked(int state)
{
    if (state == Qt::Checked)
        inclOpt = true;
    else
        inclOpt = false;
}
void MainWindow::matFat_checked(int state)
{
    if (state == Qt::Checked)
        inclFat = true;
    else
        inclFat = false;
}
void MainWindow::matCFT_checked(int state)
{
    if (state == Qt::Checked)
        inclCFT = true;
    else
        inclCFT = false;
}
//---------------------------------------------------------------
// slider
void MainWindow::slider_valueChanged(int value)
{
    //pause = true;
    stepCurr = slider->value();

    double Dcurr = (*expD)[value];
    double tcurr = (*time)[value];
    tlabel->setText(QString("deformation = %1 in.").arg(Dcurr,0,'f',2));

    // update plots
    tPlot->moveDot(tcurr,Dcurr);

    // update deform
    dPlot->plotResponse(value);
    mPlot->plotResponse(value);
    pPlot->plotResponse(value);
    hPlot->plotResponse(value);
}

/*
// press slider
void MainWindow::slider_sliderPressed()
{
    movSlider = true;
}

// released slider
void MainWindow::slider_sliderReleased()
{
    movSlider = false;
}
*/

// pause
void MainWindow::stop_clicked()
{
    stop = true;
}

// play
void MainWindow::play_clicked()
{
    pause = false;

    // play loop
    do {
        //slider->setSliderPosition(currentStep);
        slider->setValue(stepCurr);
        QCoreApplication::processEvents();
        stepCurr++;

        if (stepCurr++ == numSteps)
            stepCurr = 0;

    } while (pause == false);
}

// pause
void MainWindow::pause_clicked()
{
    pause = true;
}

// zero
void MainWindow::restart_clicked()
{
    pause = true;
    slider->setValue(0);
}

//---------------------------------------------------------------
void MainWindow::zeroResponse()
{
    // plot original coordinates
    dPlot->setModel(&xc,&yc);
    dPlot->plotModel();
    mPlot->setModel(&xc);
    mPlot->plotModel();
    pPlot->setModel(&xc);
    pPlot->plotModel();

    // re-size response and initialize to zero.
    Ux->reSize(nn,numSteps);
    Uy->reSize(nn,numSteps);

    // force quantities
    q1->reSize(ne,numSteps);
    q2->reSize(ne,numSteps);
    q3->reSize(ne,numSteps);

    dPlot->setResp(Ux,Uy);
    mPlot->setResp(q2,q3);
    pPlot->setResp(q1,q1);

    hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));
    //for (int j=0; j<numSteps; j++)
    //    qDebug() << j<< (*Ux->data[nn-1])[j];

    slider->setValue(0);
}

// x coordinates
QVector<double> xCoord(const double L, const int ne, const QString elDist)
{
    // sub-element length
    double l = L/ne;

    // initialize
    QVector<double> x(ne+1);

    // evenly distribute nodes
    for (int j=1; j<=ne; j++) {
        x[j] = x[j-1] + l;
    }

    // take middle 2-3 elements concentrated around mid-length
    if (elDist == "concentrated" && ne > 5) {
        int a = (ne+1)/2-1-0.5*(ne % 2);
        int b = 3 + (ne % 2);
        x = x.mid(a,b);
        x.prepend(0);
        x.append(L);
    }

    // user-defined
    if (elDist == "user-defined") {

    }

    return x;
}

// x coordinates
QVector<double> yCoord(const QVector<double> x, const double p, const QString shape)
{
    int nn = x.size();
    double l = x[nn-1] - x[0];
    QVector<double> y(nn);

    // node closest to midpoint (in case ne = odd)
    double xo = x[nn/2-(nn-1)%2];

    // perturbation shape
    // perturb mid-node
    if (shape == "midpoint perturbation") {
        if (nn != 2) {
            if (nn % 2 == 1) {
                y[nn/2] = p;
            } else {
                y[nn/2-0.5] = p;
                y[nn/2+0.5] = p;
            }
        }

    // arrange in linear shape
    } else if (shape == "linear") {
        // y = p/(L/2)*x if x<L/2
        for (int j=1; j<=nn/2; j++) {
            y[j] = p/xo*x[j];
        }

        // y = 2*p - p/(L/2)*x if x>L/2
        for (int j=ceil(nn/2); j<nn-1; j++) {
            y[j] = l*p/xo-p/xo*x[j];
        }

    // arange nodes in parabolic shape
    } else if (shape == "parabolic") {
        // adjust perturbation to account for ne = odd
        double po = p*l/(4*xo*(1-xo/l));

        // y = 4*p/L*x*(1-x/L)
        for (int j=1; j<nn-1; j++) {
            y[j] = 4*po/l*x[j]*(1-x[j]/l);
        }

    // arrange nodes in sinusoidal shape
    } else if (shape == "sinusoidal") {
        // adjust perturbation to account for ne = odd
        double po = p/(sin(4*atan(1)/l*xo));

        // y = p*sin(pi/L*x);
        for (int j=1; j<nn-1; j++) {
            y[j] = po*sin(4*atan(1)/l*x[j]);
        }
    }

    return y;
}

// fibers
fiberPointer FibRect2D(fiberPointer theFibers, UniaxialMaterial *theMat, const double yi, const double l, const double h, const int nf)
{
    // geometry
    double Af = l*h/nf;
    double dy = l/nf;
    double y0 = (l-dy)/2;

    // add fiber
    for (int j=theFibers.fill, k=0; j<theFibers.fill+nf; j++, k++) {
        double y = yi - y0 + k*dy;
        Fiber *theFiber = new UniaxialFiber2d(j+1,*theMat,Af,y);
        theFibers.data[j] = theFiber;
    }
    theFibers.fill = theFibers.fill + nf;

    return theFibers;
}

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

// build model
void MainWindow::buildModel()
{
    // element lengths
    L = 0.9*Lwp;
    double Lc1 = 0.05*Lwp;
    double Lc2 = 0.05*Lwp;

    // camber
    double p = delta*L;

    // node coordinates - should these be pointers?
    // x-coordinates
    xc = xCoord(L, ne, elDist);

    // y-coordinates
    yc = yCoord(xc, p, shape);

    // add nodes for spring elements
    xc.prepend(0); xc.append(L);
    yc.prepend(0); yc.append(0);

    // add nodes for boundary condition elements
    xc.prepend(-Lc1); xc.append(L+Lc2);
    yc.prepend(0); yc.append(0);

    // nn
    nn = xc.size();

    // initialize response
    zeroResponse();
}

// build model
void MainWindow::doAnalysis()
{
    // running dialog
    QProgressDialog progressDialog("Running Analysis...", "Cancel", 0, INT_MAX, this);
    QProgressBar* bar = new QProgressBar(&progressDialog);
    bar->setRange(0,numSteps);
    bar->setValue(0);
    progressDialog.setBar(bar);
    progressDialog.setMinimumWidth(500);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    stop = false;

    // clear existing model
    theDomain.clearAll();
    OPS_clearAllUniaxialMaterial();
    ops_Dt = 0.0;

    // orientations
    //Vector x(3); x(0) = 1.0; x(1) = 0.0; x(2) = 0.0;
    //Vector y(3); y(0) = 0.0; y(1) = 1.0; y(2) = 0.0;

    // direction for spring elements
    //ID dir(2); dir[0] = 0; dir[1] = 2;

    // number of nodes
    //int nn = xc.size(); // + 2 for connections

    // MP constraints at springs - equal x, y
    static Matrix eqXY(2,2);
    eqXY.Zero(); eqXY(0,0)=1.0; eqXY(1,1)=1.0;
    // DOFs
    static ID rcDof(2);
    rcDof(0) = 0; rcDof(1) = 1;

    // if truss - lock connections
    if (elType == "truss")
    {
        eqXY.resize(3,3);
        eqXY.Zero();
        eqXY(0,0)=1.0;
        eqXY(1,1)=1.0;
        eqXY(2,2)=1.0;

        rcDof.resize(3);
        rcDof(0) = 0;
        rcDof(1) = 1;
        rcDof(2) = 2;
    }

    // nodes
    Node **theNodes = new Node *[nn];
    for (int j = 0; j < nn; j++) {
        // Node(int tag, int ndof, double Crd1, double Crd2, Vector *displayLoc = 0);
        Node *theNode = new Node(j+1, 3, xc[j], yc[j]);
        theNodes[j] = theNode;
        theDomain.addNode(theNode);

        // SP constraints
        if (j==0) {
            for (int dof = 0; dof < 3; dof++) {
                SP_Constraint *theSP = new SP_Constraint(j+1, dof, 0., true);
                theDomain.addSP_Constraint(theSP);
            }
        }
        if (j==nn-1) {
            for (int dof = 1; dof < 3; dof++) {
                SP_Constraint *theSP = new SP_Constraint(j+1, dof, 0., true);
                theDomain.addSP_Constraint(theSP);
            }
        }

        // MP constraints
        if (j==2) {
            MP_Constraint *theMP = new MP_Constraint(j+1, j, eqXY, rcDof, rcDof);
            theDomain.addMP_Constraint(theMP);
        }
        if (j==nn-2) {
            MP_Constraint *theMP = new MP_Constraint(j, j+1, eqXY, rcDof, rcDof);
            theDomain.addMP_Constraint(theMP);
        }
    }

    // materials
    UniaxialMaterial *theMat = 0;
    switch (inMat->currentIndex()) {
    case 0:
        // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
        theMat = new Steel01(1,fy,Es,b);
        break;

    case 1:
        // Steel02 (int tag, double fy, double E0, double b, double R0, double cR1, double cR2, double a1, double a2, double a3, double a4)
        theMat = new Steel02(1,fy,Es,b);
        break;

    case 2:
        // Steel4 ($matTag $f_y $E_0 < -asym > < -kin $b_k $R_0 $r_1 $r_2 < $b_kc $R_0c $r_1c $r_2c > > < -iso $b_i $rho_i $b_l $R_i $l_yp < $b_ic $rho_ic $b_lc $R_ic> > < -ult $f_u $R_u < $f_uc $R_uc > > < -init $sig_init > < -mem $cycNum >)
        theMat = new Steel4(1,fy,Es,0.003,25.,0.9,0.15, // isotropic
                                                      0.023,25.0,0.9,0.15,
                                                      0.0025,1.34,0.004,1.0,1.0, // kinematic
                                                      0.0045,0.77,0.004,1.0,
                                                      100000000.0*fy,50.,100000000.0*fy,50., //ult
                                                      50,0.0); // cycNum + sig_0
        break;

    default:
        QMessageBox::warning(this, "Warning","Material not defined.");
        return;
    }

    // fibers
    fiberPointer theFibers;
    int nf = 0;

    // UniaxialFiber2d (int tag, UniaxialMaterial &theMat, double Area, double position);
    switch (sxnType)
    {
    case sxnShape::W:
    case sxnShape::M:
    case sxnShape::S:
    case sxnShape::HP:
        if (orient == "x-x") {
            double d0 = d - 2*tf;
            nf = nd + 2*ntf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMat, -(d0+tf)/2,tf,bf,ntf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMat, (d0+tf)/2,tf,bf,ntf);
            // web
            theFibers = FibRect2D(theFibers, theMat, 0,d0,tw,nd);
        }
        else
        {
            double d0 = d - 2*tf;
            nf = ntw + 2*nbf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMat, 0,bf,tf,nbf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMat, 0,bf,tf,nbf);
            // web
            theFibers = FibRect2D(theFibers, theMat, 0,tw,d0,ntw);
        }
        break;

    case sxnShape::C:
    case sxnShape::MC:
    case sxnShape::L:
    case sxnShape::dL:
    case sxnShape::WT:
    case sxnShape::MT:
    case sxnShape::ST:
        QMessageBox::warning(this, "Warning","Section not yet implemented.");
        return;
        break;

    case sxnShape::HSS:
        if (orient == "x-x") {
            double d0 = d - 2*tf;
            //double b0 = bf - 2*tw;
            nf = 2*nd + 2*ntf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // FiberRect2D(theFibers, theMat, yi= patch center, l= length, h= height, nf per patch)
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMat, -(d0+tf)/2,tf,bf,ntf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMat, (d0+tf)/2,tf,bf,ntf);
            // web
            theFibers = FibRect2D(theFibers, theMat, 0,d0,tw,nd);
            // web
            theFibers = FibRect2D(theFibers, theMat, 0,d0,tw,nd);
        }
        else
        {
            //double d0 = d - 2*tf;
            double b0 = bf - 2*tw;
            nf = 2*ntw + 2*nbf;
            theFibers.data = new Fiber *[nf];
            theFibers.fill = 0;

            // add fibers
            // flange-bottom
            theFibers = FibRect2D(theFibers, theMat, 0,b0,tf,nbf);
            // flange-top
            theFibers = FibRect2D(theFibers, theMat, 0,b0,tf,nbf);
            // web
            theFibers = FibRect2D(theFibers, theMat, -(b0+tw)/2,tw,d,ntw);
            // web
            theFibers = FibRect2D(theFibers, theMat, (b0+tw)/2,tw,d,ntw);
        }
        break;

    case sxnShape::RND:
    case sxnShape::PIPE:
        nf = ntw*nd;
        theFibers.data = new Fiber *[nf];
        theFibers.fill = nf;

        // geometry
        double dr = tw/ntw;
        double dt = 2*pi/nd;
        double r0 = d-tw;

        // add fibers
        for (int j=0, k=0; j<ntw; j++) {
            double Af = (pow(r0+(j+1)*dr,2)-pow(r0+j*dr,2))*dt/2;

            for (int m=0; m<nd; m++, k++) {
                double r = r0 + dr/2 + j*dr;
                double t = m*dt;
                double y = r*cos(t);
                Fiber *theFiber = new UniaxialFiber2d(k+1,*theMat,Af,y);
                theFibers.data[k] = theFiber;
            }
        }
        break;
    };

    // geometric transformation
    // CorotCrdTransf2d (int tag, const Vector &rigJntOffsetI, const Vector &rigJntOffsetJ)
    static Vector rigJnt(2); rigJnt.Zero();
    CrdTransf *theTransf = new CorotCrdTransf2d(1,rigJnt,rigJnt);

    // integration types
    //if (IM == )
    BeamIntegration *theIntegration = new LobattoBeamIntegration();

    // sections
    SectionForceDeformation **theSections = new SectionForceDeformation *[NIP];
    // add section
    for (int j=0; j < NIP; j++) {
        SectionForceDeformation *theSection = new FiberSection2d(1, nf, theFibers.data);
        theSections[j] = theSection;
    }

    /*
    // Print flexibility
    Matrix F = theSection[0].getInitialTangent();
    for (int j = 0; j<6; j++) {
        for (int k = 0; k<6; k++) {
            qDebug() << F(j,k);
        }
    }
    */

    // define main elements
    Element **theEls = new Element *[ne];
    double xi[10];

    for (int j=0, k=0; j<ne+4; j++) {

        if (j==0) {
            Element *theEl = new ElasticBeam2d(j+1, 10*A, Es, 10*I, j+1, j+2, *theTransf);
            theDomain.addElement(theEl);
            //theEls[k] = theEl;
            //theEl->Print(opserr);
            //k++;

        } else if (j==ne+3) {
            Element *theEl = new ElasticBeam2d(j+1, 10*A, Es, 10*I, j+1, j+2, *theTransf);
            theDomain.addElement(theEl);
            //theEls[k] = theEl;
            //theEl->Print(opserr);
            //k++;

        } else if (j==1) {
            /*
            // spring i
            theMatConn = new Steel01(2,fy,Es,b);
            Element *theEl = new ZeroLength(j+1, 2, j+1, j+2, x, y, theMatConn, dir, 0);
            theDomain.addElement(theEl);
            delete theMatConn; // each ele makes it's own copy
            */

        } else if (j==ne+2) {
            /*
            // spring j
            theMatConn = new Steel01(3,fy,Es,b);
            Element *theEl = new ZeroLength(j+1, 2, j+1, j+2, x, y, theMatConn, dir, 0);
            theDomain.addElement(theEl);
            delete theMatConn; // each ele makes it's own copy
            */

        } else {
            Element *theEl;

            // ForceBeamColumn2d (int tag, int nodeI, int nodeJ, int numSections, SectionForceDeformation **sec, BeamIntegration &beamIntegr, CrdTransf2d &coordTransf, double rho=0.0, int maxNumIters=10, double tolerance=1.0e-12)
            switch (inElType->currentIndex()) {
            case 0:
                theEl = new ForceBeamColumn2d(j+1, j+1, j+2, NIP, theSections, *theIntegration, *theTransf);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                theIntegration->getSectionLocations(NIP,1,xi);

                /*
                // recorders
                theResp[j] = "basicForces";
                theResp[j] = "plasticDeformation";
                theResp[j] = "basicDeformation"; // eps, theta1, theta2
                theResp[j] = "integrationPoints";
                theResp[j] = "section";

                // truss
                theResp[j] = "material";
                */

                break;

            case 1:
                // DispBeamColumn2d (int tag, int nd1, int nd2, int numSections, SectionForceDeformation **s, BeamIntegration &bi, CrdTransf2d &coordTransf, double rho=0.0)
                theEl = new DispBeamColumn2d(j+1, j+1, j+2, NIP, theSections, *theIntegration, *theTransf);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                theIntegration->getSectionLocations(NIP,1,xi);

                break;

            case 2:
                // CorotTruss (int tag, int dim, int Nd1, int Nd2, UniaxialMaterial &theMaterial, double A, double rho=0.0)
                theEl = new CorotTruss (j+1, 2, j+1, j+2, *theMat, A);
                theDomain.addElement(theEl);
                theEls[k] = theEl;
                k++;
                //theEl->Print(opserr);

                // IP locations
                xi[0] = 0; xi[1] = 1;

                break;
            }
        }
    }

    // get fiber locations
    double *yf = new double[nf];
    for (int j=0; j<nf; j++) {
        double yLoc, zLoc;
        theFibers.data[j]->getFiberLocation(yLoc, zLoc);
        yf[j] = yLoc;
    }

    // recorders
    int argc =  1;
    const char **argv = new const char *[argc];

    // pull basic force
    char text1[] = "basicForces";
    argv[0] = text1;
    Response **theBasicForce = new Response *[ne];
    for (int j=0; j<ne; j++) {
        theBasicForce[j]  = theEls[j]->setResponse(argv, argc, opserr);
    }

    // plastic def - epsP; theta1P; theta2p
    char text2[] = "plasticDeformation";
    argv[0] = text2;
    Response **thePlasticDef = new Response *[ne];
    for (int j=0; j<ne; j++) {
        thePlasticDef[j]  = theEls[j]->setResponse(argv, argc, opserr);
    }

    // section resp - GaussPointOutput; number; eta
    /*
    argc =  3;
    const char **argv = new const char *[argc];
    char text3[] = "section";
    char text4[] = "fiber";
    //char text5[] = "stressStrain";
    //char text6[] = "deformation"; // axial strain/curv
    argv[0] = text3;
    argv[1] = text4;
    Response **theSxn = new Response *[ne];
    for (int j=0; j<ne; j++) {
        for (int k=0; k<NIP; k++)
            for (int l=0; l<nf; l++)
            {
                //argv[1] = k;
                theSxn[j]  = theEls[j]->setResponse(argv, argc, opserr);
            }
            */

    /*
    // recorders
    theResp[j] = "basicForces";
    theResp[j] = "plasticDeformation";
    theResp[j] = "basicDeformation"; // eps, theta1, theta2
    theResp[j] = "integrationPoints";
    theResp[j] = "section";
    */

    // clean-up
    delete theMat;
    delete [] theFibers.data;
    delete [] theSections;
    delete theIntegration;
    delete theTransf;

    // load pattern
    //PathTimeSeries *theSeries = new PathTimeSeries(1,*expP,*time);
    LinearSeries *theSeries = new LinearSeries(1);
    LoadPattern *theLoadPattern = new LoadPattern(1);
    theLoadPattern->setTimeSeries(theSeries);

    // nodal load
    static Vector load(3); load.Zero(); load(0) = 1;
    NodalLoad *theLoad = new NodalLoad(1,nn,load);
    theLoadPattern->addNodalLoad(theLoad);

    // add to domain
    theDomain.addLoadPattern(theLoadPattern);

    // initial parameters
    double tol0 = 1.0e-8;
    double dU0 = 0.0;

    // solution algorithms
    //EquiSolnAlgo *theNewton = new NewtonRaphson();
    EquiSolnAlgo *theKrylov = new KrylovNewton();
    //EquiSolnAlgo *theLineSearch = new NewtonLineSearch();

    // analysis parameters
    AnalysisModel       *theModel = new AnalysisModel();
    CTestEnergyIncr     *theTest = new CTestEnergyIncr(tol0, 25, 0);
    StaticIntegrator    *theIntegr = new DisplacementControl(nn, 0, dU0, &theDomain, 1, dU0, dU0);
    ConstraintHandler   *theHandler = new PlainHandler();
    RCM                 *theRCM = new RCM();
    DOF_Numberer        *theNumberer = new DOF_Numberer(*theRCM);
#ifdef _FORTRAN_LIBS
    BandGenLinSolver    *theSolver = new BandGenLinLapackSolver();
    LinearSOE           *theSOE = new BandGenLinSOE(*theSolver);
#else
    ProfileSPDLinSolver *theSolver = new ProfileSPDLinDirectSolver();
    LinearSOE           *theSOE = new ProfileSPDLinSOE(*theSolver);
#endif

    // initialize analysis
    StaticAnalysis *theAnalysis = new StaticAnalysis (
        theDomain,*theHandler,*theNumberer,*theModel,*theKrylov,*theSOE,*theIntegr);
    theKrylov->setConvergenceTest(theTest);
    int ok = theAnalysis->analyze(1);
    //theDomain.Print(opserr);

    // re-size response and initialize to zero.
    zeroResponse();

    // initialize
    double tol = tol0;
    double dT0 = 0.01;
    double dT = dT0;
    double tcurr = 0.;
    double tfinal = (*time)[numSteps-1];

    // convergence parameters
    int numConv = 20;
    int tConv = 0;
    int t = 0;

    while (tcurr <= tfinal && stop == false)
    {
        double Ucurr = theNodes[nn-1]->getDisp()(0);
        double U = interpolate(*time, *expD, tcurr+dT, true);
        double dU = U-Ucurr;

        // set up integrator
        StaticIntegrator *theIntegrMod = new DisplacementControl(nn, 0, dU, &theDomain, 1, dU, dU);
        CTestEnergyIncr *theTestMod = new CTestEnergyIncr(tol, 25, 0);

        // analysis
        theAnalysis->setIntegrator(*theIntegrMod);
        theKrylov->setConvergenceTest(theTestMod);
        ok = theAnalysis->analyze(1);

        while (ok != 0 && stop == false)
        {
            // reset number of converged steps
            tConv = 0;
            // check cancelling
            if (progressDialog.wasCanceled())
                stop = true;

            // cut time step
            if (fabs(dT) > 1.0e-4) {
                dT = dT/2;
                qDebug() << "cut dT" << dT;
            }
            else {
                // increase tol (if dT is too small)
                dT = 1.0e-4;
                tol = tol*10;
                qDebug() << "incr tol" << tol;
            }

            // find dU
            double U = interpolate(*time, *expD, tcurr+dT, true);
            double dU = U-Ucurr;

            // modify step/tol
            StaticIntegrator *theIntegrTrial = new DisplacementControl(nn, 0, dU, &theDomain, 1, dU, dU);
            CTestEnergyIncr *theTestTrial = new CTestEnergyIncr(tol, 1000, 0);

            // analysis
            theAnalysis->setIntegrator(*theIntegrTrial);
            theKrylov->setConvergenceTest(theTestTrial);
            ok = theAnalysis->analyze(1);

            // convergence failure
            if (tol > 1.0e-2 && ok != 0)
            {
                stop = true;
                QMessageBox::warning(this, tr("Application"),
                                     tr("Analysis Failed. Results truncated accordingly."));
            }
        }

        // reset
        if (ok == 0)
        {
            // store time
            tcurr = tcurr + dT;
            // check cancelling
            if (progressDialog.wasCanceled())
                stop = true;

            if (tcurr >= (*time)[t])
            {
                progressDialog.setValue(progressDialog.value()+1);

                // store node response
                for (int j=0; j<nn; j++) {
                    const Vector &nodeU = theNodes[j]->getDisp();
                    (*Ux->data[j])[t] = nodeU(0);
                    (*Uy->data[j])[t] = nodeU(1);
                }
                // store basic force
                for (int j=0; j<ne; j++) {
                    //const Vector &q = theEls[j]->getResistingForce();
                    //(*q1->data[j])[t] = -q(0); // N1
                    //(*q2->data[j])[t] = q(2); // M1
                    //(*q3->data[j])[t] = -q(5); // M2

                    theBasicForce[j]->getResponse();
                    Information &info = theBasicForce[j]->getInformation();

                    if (elType == "truss") {
                        double theDouble = info.theDouble;
                        (*q1->data[j])[t] = theDouble;

                    } else {
                        Vector *theVector = info.theVector;
                        (*q1->data[j])[t] = (*theVector)[0];
                        (*q2->data[j])[t] = (*theVector)[1];
                        (*q3->data[j])[t] = -(*theVector)[2];
                    }
                }
                t++;
            }

            // reset
            tConv++;

            // reset tol
            if (tConv >= numConv && tol/10 >= tol0) {
                tConv = 0;
                tol = tol/10;
                qDebug() << "cut tol" << dT;
            }

            // reset dT
            if (tConv >= numConv && fabs(dT)*1.1 <= fabs(dT0)) {
                tConv = 0;
                dT = dT*1.1;
                qDebug() << "increase dT" << dT;
            }
        }
    }

    // set plot
    dPlot->setResp(Ux,Uy);
    pPlot->setResp(q1,q1);
    mPlot->setResp(q2,q3);
    hPlot->setResp(&(*Ux->data[nn-1]),&(*q1->data[0]));

    // close the dialog.
    progressDialog.close();

    // clean-up
    delete [] yf;
    delete [] theNodes;
    delete [] theEls;
    delete [] argv;
    for (int j=0; j<ne; j++)
         delete theBasicForce[j];
    delete [] theBasicForce;
    delete theAnalysis;
}

void MainWindow::setExp(Experiment *exp)
{
    numSteps = exp->getNumSteps();

    // re-size
    time->resize(numSteps);
    expP->resize(numSteps);
    expD->resize(numSteps);

    // extract from experiment
    time = exp->getTime();
    dt = exp->getdt();
    expP = exp->getDataP();
    expD = exp->getDataD();

    // update experiment plot
    tPlot->setData(expD,time);
    hPlot->setModel(expD,expP);
    hPlot->plotModel();

    // zero response
    zeroResponse();

    // set slider
    slider->setRange(0,numSteps-1);
    slider->setValue(0);
}

// layout functions
// header
/*
void MainWindow::createHeaderBox()
{
    HeaderWidget *header = new HeaderWidget();
    header->setHeadingText(tr("Brace Modeling Application"));
    largeLayout->addWidget(header);
}

// footer
void MainWindow::createFooterBox()
{
    FooterWidget *footer = new FooterWidget();
    largeLayout->addWidget(footer);
}
*/
// input panel
void MainWindow::createInputPanel()
{
    // units
    QString blank(tr("    "));
    QString kips(tr("k"  ));
    QString g(tr("g"  ));
    QString kipsInch(tr("k/in"));
    QString inch(tr("in. "));
    QString sec(tr("sec"));
    QString percent(tr("\%   "));
    QString ksi(tr("ksi   "));

    // Lists
    QStringList expList = { "" };
    QStringList elTypeList = { "force-based fiber", "displacement-based fiber", "truss" };
    QStringList distList = { "distributed", "concentrated", "user-defined" };
    QStringList IMList = { "Gauss-Lobatto" };
    QStringList shapeList = { "midpoint perturbation", "linear", "parabolic", "sinusoidal" };
    QStringList orientList = { "y-y", "x-x" };
    QStringList matList = { "uniaxial bilinear (steel01)",
                            "uniaxial Giuffre-Menegotto-Pinto (steel02)",
                            "uniaxial asymmetric Giuffre-Menegotto-Pinto (steel4)" }; // or put names???

    // boxes
    QGroupBox *inBox = new QGroupBox("Input");

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    QWidget *elTab = new QWidget;
    QWidget *sxnTab = new QWidget;
    QWidget *matTab = new QWidget;
    QWidget *connTab = new QWidget;
    QWidget *analyTab = new QWidget;

    // layouts
    inLay = new QVBoxLayout;
    QGridLayout *expLay = new QGridLayout();
    QGridLayout *elLay = new QGridLayout();
    QGridLayout *sxnLay = new QGridLayout();
    QGridLayout *matLay = new QGridLayout();
    QGridLayout *buttonLay = new QGridLayout();

    // dynamic labels
    deltaL = new QLabel;
    dlabel = new QLabel;
    bflabel = new QLabel;
    twlabel = new QLabel;
    tflabel = new QLabel;
    Alabel = new QLabel;
    Ilabel = new QLabel;
    rlabel = new QLabel;
    Zlabel = new QLabel;
    Slabel = new QLabel;

    // buttons
    QPushButton *addExp = new QPushButton("Browse");
    QPushButton *addAISC = new QPushButton("AISC Database");
    QPushButton *run = new QPushButton("run");
    QPushButton *stop = new QPushButton("stop");
    QPushButton *reset = new QPushButton("reset");

    // experiment bar
    inExp = addCombo(tr("Experiment: "),expList,&blank,expLay,0,0);
    expLay->addWidget(addExp,0,1);
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    inExp->setMinimumWidth(0.6*width/2);
    expLay->setColumnStretch(2,1);

    // element
    // col-1
    inElType = addCombo(tr("Element model: "),elTypeList,&blank,elLay,0,0);
    inL = addDoubleSpin(tr("workpoint length, Lwp: "),&inch,elLay,1,0);
    inNe = addSpin(tr("number of sub-elements, ne: "),&blank,elLay,2,0);
    inNIP = addSpin(tr("number of integration points, NIP: "),&blank,elLay,3,0);
    inDelta = addDoubleSpin(tr("camber: "),&percent,elLay,4,0);
    elLay->addWidget(deltaL,5,0);
    // col-2
    inElDist = addCombo(tr("sub-el distribution: "),distList,&blank,elLay,6,0);
    inIM = addCombo(tr("integration method: "),IMList,&blank,elLay,7,0);
    inShape = addCombo(tr("camber shape: "),shapeList,&blank,elLay,8,0);
    // stretch
    elLay->setColumnStretch(1,1);

    // section
    inSxn = addCombo(tr("Section: "),sxnList,&blank,sxnLay,0,0);
    sxnLay->addWidget(addAISC,0,1);
    inOrient = addCombo(tr("orientation: "),orientList,&blank,sxnLay,1,0);
    // fibers
    // col-1
    inNbf = addSpin(tr("nbf:"),&blank,sxnLay,2,0);
    inNtf = addSpin(tr("ntf:"),&blank,sxnLay,3,0);
    inNd = addSpin(tr("nd:"),&blank,sxnLay,4,0);
    inNtw = addSpin(tr("ntw:"),&blank,sxnLay,5,0);
    // add parameters
    // col-2
    sxnLay->addWidget(Alabel,1,1);
    sxnLay->addWidget(Ilabel,2,1);
    sxnLay->addWidget(Zlabel,3,1);
    sxnLay->addWidget(Slabel,4,1);
    sxnLay->addWidget(rlabel,5,1);
    // col-3
    sxnLay->addWidget(dlabel,1,2);
    sxnLay->addWidget(bflabel,2,2);
    sxnLay->addWidget(twlabel,3,2);
    sxnLay->addWidget(tflabel,4,2);
    // stretch
    sxnLay->setColumnStretch(3,1);

    // material
    inMat = addCombo(tr("Material model: "),matList,&blank,matLay,0,0,1,2);
    matOpt = addCheck(tr("Default inputs: "),&blank,matLay,1,1);
    matFat = addCheck(tr("Fatigue: "),&blank,matLay,2,1);
    matCFT = addCheck(tr("CFT: "),&blank,matLay,3,1);
    // material parameters
    inEs = addDoubleSpin(tr("E: "),&ksi,matLay,1,0);
    infy = addDoubleSpin(tr("fy: "),&ksi,matLay,2,0);
    inb = addDoubleSpin(tr("b: "),&percent,matLay,3,0);

    // optional parameters
    QGroupBox *optBox = new QGroupBox("Optional inputs");
    QGridLayout *optLay = new QGridLayout();

    // steel01
    // Steel01 (int tag, double fy, double E0, double b, double a1, double a2, double a3, double a4)
    QFrame *steel01Frame = new QFrame;
    QGridLayout *steel01Lay = new QGridLayout();
    ina1 = addDoubleSpin(tr("a1: "),&blank,steel01Lay,0,0);
    ina2 = addDoubleSpin(tr("a2: "),&blank,steel01Lay,1,0);
    ina3 = addDoubleSpin(tr("a3: "),&blank,steel01Lay,2,0);
    ina4 = addDoubleSpin(tr("a4: "),&blank,steel01Lay,3,0);
    steel01Frame->setLayout(steel01Lay);

    // steel02
    // Steel02 (int tag, double fy, double E0, double b, double R0, double cR1, double cR2, double a1, double a2, double a3, double a4)
    QFrame *steel02Frame = new QFrame;
    QGridLayout *steel02Lay = new QGridLayout();
    inR0 = addDoubleSpin(tr("R0: "),&blank,steel02Lay,0,0);
    inR1 = addDoubleSpin(tr("r1: "),&blank,steel02Lay,1,0);
    inR2 = addDoubleSpin(tr("r2: "),&blank,steel02Lay,2,0);
    steel02Frame->setLayout(steel02Lay);

    // steel4
    // Steel4 ($matTag $f_y $E_0 < -asym > < -kin $b_k $R_0 $r_1 $r_2 < $b_kc $R_0c $r_1c $r_2c > > < -iso $b_i $rho_i $b_l $R_i $l_yp < $b_ic $rho_ic $b_lc $R_ic> > < -ult $f_u $R_u < $f_uc $R_uc > > < -init $sig_init > < -mem $cycNum >)
    QGridLayout *steel4Lay = new QGridLayout();
    // kinematic hardening
    QGroupBox *kinBox = new QGroupBox("Kinematic Hardening");
    QGridLayout *kinLay = new QGridLayout();
    inbk = addDoubleSpin(tr("b: "),&blank,kinLay,1,0);
    inR0k = addDoubleSpin(tr("R0: "),&blank,kinLay,2,0);
    inr1 = addDoubleSpin(tr("r1: "),&blank,kinLay,3,0);
    inr2 = addDoubleSpin(tr("r2: "),&blank,kinLay,4,0);
    // compression
    inbkc = addDoubleSpin(tr("bc: "),&blank,kinLay,1,1);
    inRokc = addDoubleSpin(tr("R0c: "),&blank,kinLay,2,1);
    inr1c = addDoubleSpin(tr("r1c: "),&blank,kinLay,3,1);
    inr2c = addDoubleSpin(tr("r2c: "),&blank,kinLay,4,1);
    kinBox->setLayout(kinLay);
    steel4Lay->addWidget(kinBox,0,0);

    // isotropic hardening
    QGroupBox *isoBox = new QGroupBox("Isotropic Hardening");
    QGridLayout *isoLay = new QGridLayout();
    inbi = addDoubleSpin(tr("bi: "),&blank,isoLay,5,0);
    inrhoi = addDoubleSpin(tr("rhoi: "),&blank,isoLay,6,0);
    inbl = addDoubleSpin(tr("bl: "),&blank,isoLay,7,0);
    inRi = addDoubleSpin(tr("Ri: "),&blank,isoLay,8,0);
    inlyp = addDoubleSpin(tr("lyp: "),&blank,isoLay,9,0);
    // compression
    inbic = addDoubleSpin(tr("bic: "),&blank,isoLay,5,1);
    inrhoic = addDoubleSpin(tr("rhoic: "),&blank,isoLay,6,1);
    inblc = addDoubleSpin(tr("blc: "),&blank,isoLay,7,1);
    inRic = addDoubleSpin(tr("Ric: "),&blank,isoLay,8,1);
    isoBox->setLayout(isoLay);
    steel4Lay->addWidget(kinBox,1,0);

    // add layouts
    steel01Frame->setVisible(false);
    steel02Frame->setVisible(false);
    kinBox->setVisible(false);
    isoBox->setVisible(false);
    optLay->addWidget(steel01Frame,0,0);
    optLay->addWidget(steel02Frame,1,0);
    optLay->addLayout(steel4Lay,0,0);
    optBox->setLayout(optLay);
    matLay->addWidget(optBox,4,0);

    // fatigue parameters
    QGroupBox *fatBox = new QGroupBox("Fatigue");
    QGridLayout *fatLay = new QGridLayout();
    inm = addDoubleSpin(tr("m: "),&blank,fatLay,1,0);
    ine0 = addDoubleSpin(tr("e0: "),&blank,fatLay,2,0);
    inemax = addDoubleSpin(tr("emax: "),&blank,fatLay,3,0);
    fatBox->setLayout(fatLay);

    // add layouts
    fatBox->setVisible(false);
    matLay->addWidget(optBox,5,0);

    // stretch
    matLay->setColumnStretch(3,1);

    // el limits
    inL->setMaximum(10000);
    inL->setDecimals(2);
    inNe->setMaximum(100);
    inNIP->setMaximum(10);
    inDelta->setDecimals(3);
    inDelta->setMaximum(20);
    inDelta->setSingleStep(0.001);
    // set minimum input
    inNe->setMinimum(1);
    inNIP->setMinimum(1);
    inNbf->setMinimum(1);
    inNtf->setMinimum(1);
    inNd->setMinimum(1);
    inNtw->setMinimum(1);
    // sxn limits
    inNbf->setMaximum(100);
    inNtf->setMaximum(100);
    inNd->setMaximum(100);
    inNtw->setMaximum(100);
    // mat limits
    inEs->setMaximum(100000);
    inEs->setDecimals(1);
    inEs->setSingleStep(1000);
    infy->setMaximum(200);
    infy->setDecimals(1);
    inb->setMaximum(10);
    inb->setDecimals(2);
    inb->setSingleStep(0.01);

    // buttons
    buttonLay->addWidget(run,0,0);
    buttonLay->addWidget(stop,0,1);
    buttonLay->addWidget(reset,0,2);
    run->setStyleSheet("font: bold");
    stop->setStyleSheet("font: bold");
    reset->setStyleSheet("font: bold");
    buttonLay->setColumnStretch(3,1);

    // set tab layouts
    inLay->addLayout(expLay);
    elTab->setLayout(elLay);
    elLay->setRowStretch(elLay->rowCount(),1);
    sxnTab->setLayout(sxnLay);
    sxnLay->setRowStretch(sxnLay->rowCount(),1);
    matTab->setLayout(matLay);
    matLay->setRowStretch(matLay->rowCount(),1);

    // add layout to tab
    tabWidget->addTab(elTab, "Element");
    tabWidget->addTab(sxnTab, "Section");
    tabWidget->addTab(matTab, "Material");
    tabWidget->addTab(connTab, "Connection");
    tabWidget->addTab(analyTab, "Analysis");

    // add tab
    inLay->addWidget(tabWidget);
    //inLay->addStretch();

    // add buttons
    inLay->addLayout(buttonLay);

    // add to main layout
    inBox->setLayout(inLay);
    mainLayout->addWidget(inBox);
    largeLayout->addLayout(mainLayout);

    // connect signals / slots
    // buttons
    connect(addExp,SIGNAL(clicked()), this, SLOT(addExp_clicked()));
    connect(addAISC,SIGNAL(clicked()), this, SLOT(addAISC_clicked()));
    connect(reset,SIGNAL(clicked()), this, SLOT(reset()));
    connect(run,SIGNAL(clicked()), this, SLOT(doAnalysis()));
    connect(stop,SIGNAL(clicked()), this, SLOT(stop_clicked()));

    // Combo Box
    connect(inSxn,SIGNAL(currentIndexChanged(int)), this, SLOT(inSxn_currentIndexChanged(int)));
    connect(inOrient,SIGNAL(currentIndexChanged(int)), this, SLOT(inOrient_currentIndexChanged(int)));
    connect(inElType,SIGNAL(currentIndexChanged(int)), this, SLOT(inElType_currentIndexChanged(int)));
    connect(inElDist,SIGNAL(currentIndexChanged(int)), this, SLOT(inElDist_currentIndexChanged(int)));
    connect(inIM,SIGNAL(currentIndexChanged(int)), this, SLOT(inIM_currentIndexChanged(int)));
    connect(inShape,SIGNAL(currentIndexChanged(int)), this, SLOT(inShape_currentIndexChanged(int)));
    connect(inMat,SIGNAL(currentIndexChanged(int)), this, SLOT(inMat_currentIndexChanged(int)));

    // Spin box
    connect(inNe,SIGNAL(valueChanged(int)), this, SLOT(inNe_valueChanged(int)));
    connect(inNIP,SIGNAL(valueChanged(int)), this, SLOT(inNIP_valueChanged(int)));
    connect(inNbf,SIGNAL(valueChanged(int)), this, SLOT(inNbf_valueChanged(int)));
    connect(inNtf,SIGNAL(valueChanged(int)), this, SLOT(inNtf_valueChanged(int)));
    connect(inNd,SIGNAL(valueChanged(int)), this, SLOT(inNd_valueChanged(int)));
    connect(inNtw,SIGNAL(valueChanged(int)), this, SLOT(inNtw_valueChanged(int)));

    // double spin box
    connect(inL,SIGNAL(valueChanged(double)), this, SLOT(inL_valueChanged(double)));
    connect(inDelta,SIGNAL(valueChanged(double)), this, SLOT(inDelta_valueChanged(double)));
    connect(inEs,SIGNAL(valueChanged(double)), this, SLOT(inEs_valueChanged(double)));
    connect(infy,SIGNAL(valueChanged(double)), this, SLOT(infy_valueChanged(double)));
    connect(inb,SIGNAL(valueChanged(double)), this, SLOT(inb_valueChanged(double)));

    // check box
    connect(matOpt, SIGNAL(stateChanged(int)), this, SLOT(matOpt_checked(int)));
    connect(matCFT, SIGNAL(stateChanged(int)), this, SLOT(matCFT_checked(int)));
    connect(matFat, SIGNAL(stateChanged(int)), this, SLOT(matFat_checked(int)));
}

// output panel
void MainWindow::createOutputPanel()
{
    // 1) Basic Outputs, e.g. Disp, Periods
    // 2) MyGlWidget
    // 3) QCustomPlotWidget
    // 4) CurrentTime

    // boxes
    QGroupBox *outBox = new QGroupBox("Output");
    QGroupBox *dispBox = new QGroupBox("Deformed Shape");
    QGroupBox *tBox = new QGroupBox("Applied History");
    QGroupBox *hystBox = new QGroupBox("Hysteretic Response");

    // layouts
    outLay = new QGridLayout;
    QGridLayout *dispLay = new QGridLayout();
    QGridLayout *axialLay = new QGridLayout();
    QGridLayout *momLay = new QGridLayout();
    QGridLayout *curvLay = new QGridLayout();
    QGridLayout *fiberLay = new QGridLayout();
    QGridLayout *hystLay = new QGridLayout();
    QGridLayout *tLay = new QGridLayout();
    QGridLayout *buttonLay = new QGridLayout();

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    QWidget *axialTab = new QWidget;
    QWidget *momTab = new QWidget;
    QWidget *curvTab = new QWidget;
    QWidget *fiberTab = new QWidget;

    // buttons
    QPushButton *play = new QPushButton("play");
    QPushButton *pause = new QPushButton("pause");
    QPushButton *restart = new QPushButton("restart");

    // deformed shape plot
    dPlot = new deformWidget(tr("Length, Lwp"), tr("Deformation"));
    dispLay->addWidget(dPlot,0,0);

    // axial force plot
    pPlot = new responseWidget(tr("Length, Lwp"), tr("Axial Force"));
    axialLay->addWidget(pPlot,0,0);

    // moment plot
    mPlot = new responseWidget(tr("Length, Lwp"), tr("Moment"));
    momLay->addWidget(mPlot,0,0);

    // curvature plot
    kPlot = new responseWidget(tr("Length, Lwp"), tr("Curvature"));
    curvLay->addWidget(kPlot,0,0);

    // hysteretic plot
    hPlot = new hysteresisWidget(tr("Axial Deformation [in.]"), tr("Axial Force [kips]"));
    hystLay->addWidget(hPlot,0,0);

    // loading plot
    tPlot = new historyWidget(tr("Pseudo-time"), tr("Applied History"));
    tLay->addWidget(tPlot,0,0);

    // slider
    slider = new QSlider(Qt::Horizontal);
    tLay->addWidget(slider,1,0);

    // show time
    tlabel = new QLabel;
    tLay->addWidget(tlabel,2,0);
    //tLay->setColumnStretch(1, 1);

    // play
    buttonLay->addWidget(play,0,0);
    buttonLay->addWidget(pause,0,1);
    buttonLay->addWidget(restart,0,2);
    play->setStyleSheet("font: bold");
    pause->setStyleSheet("font: bold");
    restart->setStyleSheet("font: bold");
    buttonLay->setColumnStretch(3,1);

    // add displaced shape
    dispBox->setLayout(dispLay);
    outLay->addWidget(dispBox,0,0);

    // set tab layouts
    axialTab->setLayout(axialLay);
    momTab->setLayout(momLay);
    curvTab->setLayout(curvLay);
    fiberTab->setLayout(fiberLay);

    // add layout to tab
    tabWidget->addTab(axialTab, "Axial Force Diagram");
    tabWidget->addTab(momTab, "Moment Diagram");
    tabWidget->addTab(curvTab, "Curvature Diagram");
    tabWidget->addTab(fiberTab, "Section Response");

    // add tab
    outLay->addWidget(tabWidget,1,0);
    //outLay->addStretch();

    // add hysteretic
    hystBox->setLayout(hystLay);
    outLay->addWidget(hystBox,0,1,2,1);

    // add time
    tBox->setLayout(tLay);
    outLay->addWidget(tBox,2,0,1,2);
    outLay->addLayout(buttonLay,3,0);

    // add to main layout
    outBox->setLayout(outLay);
    mainLayout->addWidget(outBox);

    // signals/slots
    connect(play,SIGNAL(clicked()), this, SLOT(play_clicked()));
    connect(pause,SIGNAL(clicked()), this, SLOT(pause_clicked()));
    connect(restart,SIGNAL(clicked()), this, SLOT(restart_clicked()));
    //connect(slider, SIGNAL(sliderPressed()),  this, SLOT(slider_sliderPressed()));
    //connect(slider, SIGNAL(sliderReleased()), this, SLOT(slider_sliderReleased()));
    connect(slider, SIGNAL(valueChanged(int)),this, SLOT(slider_valueChanged(int)));
}

// label functions
// name(QLabel) + enter(QComboBox) + units(QLabel)
QCheckBox *addCheck(QString text, QString *unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QCheckBox *res = new QCheckBox();

    // props
    //res->setMinimumWidth(150);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(50);
        //unitLabel->setMaximumWidth(50);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QComboBox) + units(QLabel)
QComboBox *addCombo(QString text, QStringList items, QString *unitText,
                    QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QComboBox *res = new QComboBox();

    // add labels
    for (int i = 0; i < items.size(); i++)
    {
        res->addItem(items.at(i));
    }

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.3*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        Lay->addWidget(unitLabel);
        unitLabel->setMinimumWidth(50);
        unitLabel->setMaximumWidth(50);
    }

    if (ncol>1)
        Lay->addStretch(0);

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QDoubleSpinBox *addDoubleSpin(QString text,QString *unitText,
                QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QDoubleSpinBox *res = new QDoubleSpinBox();

    // props
    res->setMinimum(0.);
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.3*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(50);
        unitLabel->setMaximumWidth(50);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(0);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}

// name(QLabel) + enter(QDoubleSpinBox) + units(QLabel)
QSpinBox *addSpin(QString text, QString *unitText,
                  QGridLayout *gridLay,int row,int col, int nrow, int ncol)
{
    QHBoxLayout *Lay = new QHBoxLayout();
    QLabel *Label = new QLabel(text);
    QSpinBox *res = new QSpinBox();

    // props
    res->setKeyboardTracking(0);

    // width
    QRect rec = QApplication::desktop()->screenGeometry();
    //int height = 0.7*rec.height();
    int width = 0.7*rec.width();
    res->setMinimumWidth(0.3*width/2);

    // layout
    Lay->addWidget(Label);
    Lay->addStretch(0);
    Lay->addWidget(res);

    // unit text
    if (unitText != 0) {
        QLabel *unitLabel = new QLabel(*unitText);
        unitLabel->setMinimumWidth(50);
        unitLabel->setMaximumWidth(50);
        Lay->addWidget(unitLabel);
    }

    // main layout
    Lay->setSpacing(2);
    Lay->setMargin(2);
    gridLay->addLayout(Lay,row,col,nrow,ncol);

    return res;
}
