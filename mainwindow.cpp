#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QSerialPortInfo>
#include <QSettings>
#include <QtMath>
#include <QDebug>
#include <QTime>
#define version "SBE Fake 0.5"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(version);
    mUdpSBE=new UDPData();
    mSensor=new SensorDialog;
    mTabDatas=new QVector<stMVPDatas>;
    mFenImmersion=new fen_Immersion;

    QSettings settings;

    QObject::connect(ui->btn_Start,&QPushButton::clicked,this,&MainWindow::clickOnStart);
    //QObject::connect(mUdpSBE,&UDPData::dataReceived,this,&MainWindow::readData);
    QObject::connect(mUdpSBE,&UDPData::errorString,this,&MainWindow::errorMsg);
    QObject::connect(ui->btn_Refresh,&QPushButton::clicked,this,&MainWindow::majInfo);
    QObject::connect(ui->btn_Connect,&QPushButton::clicked,this,&MainWindow::clickOnConnect);

    QObject::connect(mSensor,&SensorDialog::dataReceived,this,&MainWindow::readData);
    QObject::connect(mSensor,&SensorDialog::errorString,this,&MainWindow::errorMsg);
    QObject::connect(ui->actionQuitter,&QAction::triggered,this,&MainWindow::close);
    QObject::connect(ui->actionImmersion,&QAction::triggered,mFenImmersion,&fen_Immersion::show);



    mPortIn=settings.value("PortSBE",22112).toInt();
    mPortName=settings.value("PortName","").toString();
    mBaudrate=settings.value("BaudRate","4800").toString();
    mFilterSize=settings.value("FilterSize",10).toInt();
    mTypeConnec=SensorDialog::Serie;

    majInfo();
    ui->sp_PortOut->setValue(settings.value("PortOut",4001).toInt());
    ui->le_IPDiff->setText(settings.value("IpDiff","172.16.128.255").toString());
    ui->btn_Start->setEnabled(false);
    ui->sp_Filtre->setValue(mFilterSize);
    ui->cb_Baudrate->setCurrentIndex(ui->cb_Baudrate->findText(mBaudrate));
    //ui->cb_Serial->setCurrentIndex(ui->cb_Serial->findText(mPortName));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readData(QString sTrame)
{
    ui->l_TrameIn->setText("Trame reçue: "+sTrame);
      stMVPDatas uneData;
      stMVPDatas moyData;

        /*
sXXXX.XbsXX.XXXbsXX.XXXb<CR><LF>
0001.9 00.000 24.362 [0D][0A]
P(dbar) C(mS/cm) T(degC)
*/
       if(sTrame.count(" ")==5 && sTrame.contains(QRegExp("[\\n\\r]")))
        {
            ui->l_Cpt->setText(QString("%1").arg(mNbReceive));

            uneData.pression=sTrame.section(" ",1,1).toDouble();
            uneData.conductivite=sTrame.section(" ",3,3).toDouble();
            uneData.conductivite=uneData.conductivite/10;
            uneData.temperature=sTrame.section(" ",5,5).remove(QRegExp("[\\n\\r]")).toDouble();
            uneData.salinite=salinity(uneData.conductivite,uneData.temperature,uneData.pression);
            uneData.celerite=sndVel(uneData.salinite,uneData.temperature,uneData.pression);
            uneData.dispo=true;

            moyData=filtreMoy(uneData);

            ui->l_Pression->setText(QString("Pression mesurée: %1 dBar;\t\tmoyenne: %2 dBar")
                                    .arg(QString::number(uneData.pression,'f',3))
                                    .arg(QString::number(moyData.pression,'f',3)));
            mFenImmersion->setValue(moyData.pression);

            ui->l_Cond->setText(QString("Conductivité mesurée: %1 S/m;\t\tmoyenne: %2 S/m")
                                    .arg(QString::number(uneData.conductivite,'f',3))
                                    .arg(QString::number(moyData.conductivite,'f',3)));
            ui->l_Temp->setText(QString("Température mesurée: %1 °C;\t\tmoyenne: %2 °C")
                                    .arg(QString::number(uneData.temperature,'f',3))
                                    .arg(QString::number(moyData.temperature,'f',3)));
            ui->l_Sal->setText(QString("Salinité calculée: %1 psu;\t\t\tmoyenne: %2 psu")
                                    .arg(QString::number(uneData.salinite,'f',3))
                                    .arg(QString::number(moyData.salinite,'f',3)));
            ui->l_Cel->setText(QString("Célérité calculée: %1 m/s:\t\tmoyenne: %2 m/s")
                                    .arg(QString::number(uneData.celerite,'f',2))
                                    .arg(QString::number(moyData.celerite,'f',2)));
            ui->l_Heure->setText(QString("Heure de la dernière trame reçue: %1").arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

            if(mNbReceive>=mFilterSize)
            {
                mNbReceive=0;
                if(mDiffStatus)
                    diffKSSIS(moyData);

            }

            mNbReceive++;
        }

    }


void MainWindow::clickOnStart()
{
    QSettings settings;



    if(ui->btn_Start->isChecked())
    {
        ui->btn_Start->setText("Arrêter");
        ui->le_IPDiff->setEnabled(false);
        ui->sp_PortOut->setEnabled(false);
        mDiffStatus=true;


    }
    else
    {
        ui->btn_Start->setText("Diffuser");
        ui->le_IPDiff->setEnabled(true);
        ui->sp_PortOut->setEnabled(true);
        mDiffStatus=false;


    }

}

void MainWindow::clickOnConnect()
{
    //affConnec(ui->btn_Connect->isChecked());
    if(ui->btn_Connect->isChecked())
    {
        initCOM();
    }
    else
    {
        mSensor->setDisconnected();
        affConnec(true);
        ui->statusbar->showMessage(QString("%1 déconnecté").arg(mPortName));
    }

}


void MainWindow::errorMsg(QString sMsg)
{
    ui->statusbar->showMessage(sMsg,30000);
}


void MainWindow::initCOM()
{
    mPortName=ui->cb_Serial->currentText();
    mBaudrate=ui->cb_Baudrate->currentText();
    mFilterSize=ui->sp_Filtre->value();

    mSensor->initCOM(mPortName,mBaudrate,mTypeConnec);
    if(mSensor->setConnected())
    {
        ui->statusbar->showMessage(QString("%1 est connecté").arg(mPortName));
        affConnec(false);
        QSettings settings;

        settings.setValue("PortName",mPortName);
        settings.setValue("BaudRate",mBaudrate);
        settings.setValue("FilterSize",mFilterSize);

    }
    else
    {
        ui->statusbar->showMessage(QString("%1 déconnecté").arg(mPortName));
        affConnec(true);
        majInfo();

    }
}


void MainWindow::diffKSSIS(MainWindow::stMVPDatas uneData)
{
    QString sKSSIS=QString("$KSSIS,80,%1,%2").arg(QString::number(uneData.celerite,'f',2)).arg(QString::number(uneData.temperature,'f',2));
    sKSSIS=sKSSIS+0x0D+0x0a;


    QString sIp=ui->le_IPDiff->text();
    int nPortOut=ui->sp_PortOut->value();

    if(mUdpSBE->writeData(sIp,nPortOut,sKSSIS))
    {
        QSettings settings;
        settings.setValue("IpDiff",sIp);
        settings.setValue("PortOut",nPortOut);
        ui->l_TrameOut->setText(QString("Trame émise: "+sKSSIS));

    }


}

void MainWindow::affConnec(bool bStatus)
{
    if(bStatus)
        ui->btn_Connect->setText("Connecter");
    else
        ui->btn_Connect->setText("Déconnecter");

    ui->cb_Serial->setEnabled(bStatus);
    ui->cb_Baudrate->setEnabled(bStatus);
    ui->btn_Refresh->setEnabled(bStatus);
    ui->btn_Start->setEnabled(!bStatus);
    ui->sp_Filtre->setEnabled(bStatus);
}

double MainWindow::salinity(double C, double T, double P)
{
    // C = conductivity S/m, T = temperature deg C ITPS-68, P = pressure in decibars
    const double A1 = 2.070e-5;
    const double A2 = -6.370e-10;
    const double A3 = 3.989e-15;
    const double B1 = 3.426e-2;
    const double B2 = 4.464e-4;
    const double B3 = 4.215e-1;
    const double B4 = -3.107e-3;
    const double C0 = 6.766097e-1;
    const double C1 = 2.00564e-2;
    const double C2 = 1.104259e-4;
    const double C3 = -6.9698e-7;
    const double C4 = 1.0031e-9;


    static double a[6] = { /* constants for salinity calculation */
    0.0080, -0.1692, 25.3851, 14.0941, -7.0261, 2.7081
    };
    static double b[6]={ /* constants for salinity calculation */
    0.0005, -0.0056, -0.0066, -0.0375, 0.0636, -0.0144
                       };

    double R=0, RT=0, RP=0, temp=0, sum1=0, sum2=0, result=0, val=0;
    int i;
    if (C <= 0.0)
    result = 0.0;
    else {
    C *= 10.0; /* convert Siemens/meter to mmhos/cm */
    R = C / 42.914;
    val = 1 + B1 * T + B2 * T * T + B3 * R + B4 * R * T;
    if (val) RP = 1 + (P * (A1 + P * (A2 + P * A3))) / val;
    val = RP * (C0 + (T * (C1 + T * (C2 + T * (C3 + T * C4)))));
    if (val) RT = R / val;
    if (RT <= 0.0) RT = 0.000001;
    sum1 = sum2 = 0.0;
    for (i = 0;i < 6;i++) {
    temp = pow(RT, (double)i/2.0);
    sum1 += a[i] * temp;
    sum2 += b[i] * temp;
    }
    val = 1.0 + 0.0162 * (T - 15.0);
    if (val)
    result = sum1 + sum2 * (T - 15.0) / val;
    else
    result = -99.;
    }
    return result;


}

double MainWindow::sndVel(double s, double t, double p0)
{
    double a, a0, a1, a2, a3;
    double b, b0, b1;
    double c, c0, c1, c2, c3;
    double p, sr, d, sv;
    p = p0 / 10.0; /* scale pressure to bars */
    if (s < 0.0) s = 0.0;
    sr = sqrt(s);
    d = 1.727e-3 - 7.9836e-6 * p;
    b1 = 7.3637e-5 + 1.7945e-7 * t;
    b0 = -1.922e-2 - 4.42e-5 * t;
    b = b0 + b1 * p;
    a3 = (-3.389e-13 * t + 6.649e-12) * t + 1.100e-10;
    a2 = ((7.988e-12 * t - 1.6002e-10) * t + 9.1041e-9) * t - 3.9064e-7;
    a1 = (((-2.0122e-10 * t + 1.0507e-8) * t - 6.4885e-8) * t - 1.2580e-5) * t + 9.4742e-5;
    a0 = (((-3.21e-8 * t + 2.006e-6) * t + 7.164e-5) * t -1.262e-2) * t + 1.389;
    a = ((a3 * p + a2) * p + a1) * p + a0;
    c3 = (-2.3643e-12 * t + 3.8504e-10) * t - 9.7729e-9;
    c2 = (((1.0405e-12 * t -2.5335e-10) * t + 2.5974e-8) * t - 1.7107e-6) * t + 3.1260e-5;
    c1 = (((-6.1185e-10 * t + 1.3621e-7) * t - 8.1788e-6) * t + 6.8982e-4) * t + 0.153563;
    c0 = ((((3.1464e-9 * t - 1.47800e-6) * t + 3.3420e-4) * t - 5.80852e-2) * t + 5.03711) * t +
    1402.388;
    c = ((c3 * p + c2) * p + c1) * p + c0;
    sv = c + (a + b * sr + d * s) * s;
    return sv;
}

MainWindow::stMVPDatas MainWindow::filtreMoy(MainWindow::stMVPDatas lastMVP)
{

    if(mTabDatas->size()<mFilterSize)
    {
        mTabDatas->append(lastMVP);
        return lastMVP;
    }
    else
    {
        mTabDatas->removeFirst();
        mTabDatas->append(lastMVP);
        stMVPDatas moyMVP;
        stMVPDatas uneData;
        QVectorIterator<stMVPDatas>it(*mTabDatas);
        int i=0;
        while(it.hasNext())
        {
            uneData=it.next();
            moyMVP.pression=moyMVP.pression+uneData.pression;
            moyMVP.conductivite=moyMVP.conductivite+uneData.conductivite;
            moyMVP.temperature=moyMVP.temperature+uneData.temperature;
            moyMVP.salinite=moyMVP.salinite+uneData.salinite;
            moyMVP.celerite=moyMVP.celerite+uneData.celerite;

            i++;
        }
        moyMVP.pression=moyMVP.pression/i;
        moyMVP.conductivite=moyMVP.conductivite/i;
        moyMVP.temperature=moyMVP.temperature/i;
        moyMVP.salinite=moyMVP.salinite/i;
        moyMVP.celerite=moyMVP.celerite/i;

        return moyMVP;

    }
}


void MainWindow::majInfo()
{
    QStringList portList;
    ui->cb_Serial->clear();

    const auto infos = QSerialPortInfo::availablePorts();

        for (const QSerialPortInfo &info : infos)
        {
            portList.append(info.portName());
            this->ui->cb_Serial->addItem(info.portName());
        }

        QStringListIterator it (portList);
        bool bFound=it.findNext(mPortName);
        if(bFound)
        {
            ui->cb_Serial->setCurrentIndex(ui->cb_Serial->findText(mPortName));
        }





}

