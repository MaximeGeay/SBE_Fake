#include "udpdata.h"

UDPData::UDPData()
{
    mUdpSocket=new QUdpSocket();
    mTimer=new QTimer();
    mTimer->setSingleShot(true);
    mTimer->setInterval(3000);
    QObject::connect(mUdpSocket,&QUdpSocket::readyRead,this,&UDPData::readData);

    QObject::connect(mTimer,&QTimer::timeout,this,&UDPData::dataTimeout);
}

UDPData::~UDPData()
{
    mUdpSocket->close();
}

void UDPData::initCom(int nPortUDP)
{

    if(mPortUdp!=0)
    {
        //close connection
        mUdpSocket->close();
    }

    if(nPortUDP!=0)
        mPortUdp=nPortUDP;

    if(mUdpSocket->bind(QHostAddress::Any,mPortUdp,QAbstractSocket::ReuseAddressHint))
    {
         emit errorString(QString("Connecté au port UDP %1").arg(mPortUdp));

    }
    else
    {
        emit errorString(mUdpSocket->errorString());


    }
}

void UDPData::setInterval(int nInterval)
{
    mTimer->setInterval(nInterval);
}

void UDPData::close()
{
    mUdpSocket->close();
    emit errorString(QString("Déconnecté du port UDP %1").arg(mPortUdp));
}

int UDPData::UdpPort()
{
    return mPortUdp;
}

void UDPData::readData()
{
    QByteArray datagram;
    while (mUdpSocket->hasPendingDatagrams())
    {
        datagram.resize(mUdpSocket->pendingDatagramSize());
        mUdpSocket-> readDatagram(datagram.data(),datagram.size()); // datagram.data est un char

    }
    QString sTrame=datagram.data();
    emit dataReceived(sTrame);
    mTimer->start();

}

void UDPData::dataTimeout()
{
    emit timeout();
}

bool UDPData::writeData(QString sIp, int nPort, QString sData)
{

    QByteArray datagram=sData.toLocal8Bit();
    qint64 res=mUdpSocket->writeDatagram(datagram.data(), datagram.size(),QHostAddress(sIp.remove(" ")),nPort);
    if(res==-1)
    {
        emit errorString("Trame non émise");
        return false;
    }
    else
    {
        emit errorString(QString("Trame émise %1").arg(sData));
        return true;
    }
}
