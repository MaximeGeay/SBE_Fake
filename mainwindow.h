#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "udpdata.h"
#include "sensordialog.h"
#include"fen_immersion.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct stMVPDatas{
        double pression=0;
        double conductivite=0;
        double temperature=0;
        double salinite=0;
        double celerite=0;
        bool dispo=false;

    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
     void readData(QString sTrame);
     void clickOnStart();
     void clickOnConnect();
     void errorMsg(QString sMsg);
     void majInfo();

private:
    Ui::MainWindow *ui;
    fen_Immersion *mFenImmersion;
    UDPData *mUdpSBE;
    void initCOM();
    void diffKSSIS(stMVPDatas uneData);
    void affConnec(bool bStatus);
    double salinity(double C, double T, double P);
    double sndVel(double s, double t, double p0); /* sound velocity Chen and Millero 1977 */
    stMVPDatas filtreMoy(stMVPDatas lastMVP);

    QString mPortName;
    QString mBaudrate;
    SensorDialog* mSensor;
    SensorDialog::ConnexionType mTypeConnec;
    QVector<stMVPDatas> *mTabDatas;
    int mFilterSize=0;
    int mNbReceive=0;

    int mPortIn;
    bool mDiffStatus=false;

};
#endif // MAINWINDOW_H
