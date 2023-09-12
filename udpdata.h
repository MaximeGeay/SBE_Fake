#ifndef UDPDATA_H
#define UDPDATA_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>

class UDPData : public QObject
{
    Q_OBJECT
public:
    UDPData();
    ~UDPData();

public slots:
    void initCom(int nPortUDP);
    void setInterval(int nInterval);
    void close();
    int UdpPort();  
    bool writeData(QString sIp,int nPort,QString sData);


private slots:
    void readData();
    void dataTimeout();


signals:
    void dataReceived(QString);
    void errorString(QString);
    void timeout();

private:
    QUdpSocket* mUdpSocket;
    QTimer* mTimer;
    int mPortUdp=0;



};

#endif // UDPDATA_H
