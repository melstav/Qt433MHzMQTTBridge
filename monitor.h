#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QMqttClient>
#include <QtSerialPort>
#include <QTimer>

class Monitor : public QObject
{
    Q_OBJECT
public:
    explicit Monitor(QObject *parent = nullptr);

protected:
    QTimer repeatTimer;
    QString m_topCategory;
    QMqttClient m_client;
    QSerialPort m_port;
    QByteArray readBuffer, prevLine;
    QString defaultMessage;

    /* Use the messages received from the serial port as keys into this map.
     * Values in the map should be pointers to Switch classes.
     */
    QVariantMap switches;

private:
    void setupSwitches();

signals:
    void receivedMessage(const QString& line);

public slots:
    void connectMQTT();
    void connectSerial();
    void onSerialDataReady();

protected slots:
    void onMQTTConnected();

};

#endif // MONITOR_H
