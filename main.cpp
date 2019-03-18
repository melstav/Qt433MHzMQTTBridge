#include <QCoreApplication>
#include <QtDebug>
#include "monitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Monitor mon(&a);

#if 0
    QMqttClient::ClientState mystate = m_client.state();
    m_client.requestPing();

    if (m_client.publish(QString("DoorSensor/state"), QString("DoorSensor monitor Started").toUtf8()) == -1)
        qWarning("Could not publish the message.");
#endif

    mon.connectMQTT();
    mon.connectSerial();

    return a.exec();
}
