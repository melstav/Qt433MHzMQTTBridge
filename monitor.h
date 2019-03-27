#ifndef MONITOR_H
#define MONITOR_H

#include <QObject>
#include <QMqttClient>
#include <QtSerialPort>
#include <QTimer>
#include "switch.h"

class Monitor : public QObject
{
    Q_OBJECT
public:
    class MQTTSettings {
    public:
        QString hostname;
        unsigned short port;
        QString topCategory;
        QString defaultTopic, willTopic;
        QString startMessage, defaultMessage, willMessage;
        unsigned char msgQoS, willQoS;
        bool msgRetain, willRetain;

        void readJSON(const QJsonObject &json);
        void writeJSON(QJsonObject &json) const;
    };

    class SerialSettings {
    public:
        QString portName;
        int baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;

        void readJSON(const QJsonObject &json);
        void writeJSON(QJsonObject &json) const;
    };

    explicit Monitor(QObject *parent = nullptr);
    ~Monitor();

    bool loadConfig();
    bool saveConfig();

    void readJSON(const QJsonObject &json);
    void writeJSON(QJsonObject &json) const;

protected:
    QTimer repeatTimer;

    MQTTSettings mqttSettings;
    SerialSettings serialSettings;

    QMqttClient m_client;
    QSerialPort m_port;
    QByteArray readBuffer, prevLine;

    /* Use the messages received from the serial port as keys into this map.
     * Values in the map should be pointers to Switch classes.
     */
//    QMap<QString, Switch> switches;
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
    void postedMessage(const QString& topic, const QString& message);

};

#endif // MONITOR_H
