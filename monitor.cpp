#include "monitor.h"
#include <QtDebug>
#include <QTimer>

Monitor::Monitor(QObject *parent) :
    QObject(parent),
    m_topCategory("DoorSensor"),
    defaultMessage("Unknown message received: %1")
{
    connect(&m_client, SIGNAL(connected()), SLOT(onMQTTConnected()));
    connect(&m_port, SIGNAL(readyRead()), SLOT(onSerialDataReady()));
    repeatTimer.setInterval(200 /* ms */);
    repeatTimer.setSingleShot(true);
}

void Monitor::connectMQTT() {
    m_client.setHostname("192.168.1.30");
    m_client.setPort(1883);
    m_client.setWillTopic(m_topCategory + "/state");
    m_client.setWillMessage("DoorSensor monitor died unexpectedly.");
    m_client.setWillQoS(1);
    m_client.connectToHost();
}

void Monitor::onMQTTConnected() {
    qDebug() << "Connected to MQTT Server.";
#if 1
    auto subscription = m_client.subscribe(m_topCategory + "/state");
    if (!subscription) {
        qWarning("Could not subscribe. Is there a valid connection?");
        return;
    }
#endif

    if (m_client.publish((m_topCategory + "/state"), QString("DoorSensor monitor Started").toUtf8(), 1) == -1)
        qWarning("Could not publish the message.");
}

void Monitor::connectSerial() {
    m_port.setPortName("COM9");
    bool rv = m_port.open(QIODevice::ReadWrite);
    if (rv && m_port.setBaudRate(9600)
            && m_port.setDataBits(QSerialPort::Data8)
            && m_port.setParity(QSerialPort::NoParity)
            && m_port.setStopBits(QSerialPort::OneStop)
            && m_port.setFlowControl(QSerialPort::NoFlowControl)
            )
    {
        qDebug() << "Serial port has been opened.";
    }
    else {
        qWarning() << "Could not open serial port.";
        m_port.close();
    }
}

void Monitor::onSerialDataReady() {
    readBuffer.append(m_port.readAll());
    int pos;
    while ((pos = readBuffer.indexOf('\n')) != -1) {
        QByteArray line = readBuffer.left(pos).trimmed();
        QString subCategory, message;
        readBuffer.remove(0, pos+1);

        if ( (line != prevLine) || (!repeatTimer.isActive()) ) {
            prevLine = line;
            qDebug() << "Received: " << line;
            emit receivedMessage(line);
            if (switches.contains(line)) {

            }
            else {
                subCategory = "/state";
                message = defaultMessage;
            }
            message = message.arg(QString(line));
            qDebug() << message;
            if (m_client.publish((m_topCategory + "/state"), message.toUtf8(), 1) == -1)
                qWarning("Could not publish the message.");
        }
        repeatTimer.start();
    }
}
