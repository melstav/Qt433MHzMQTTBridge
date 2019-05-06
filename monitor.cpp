#include "monitor.h"
#include <QVariant>
#include <QDaemonLog>
#include <QTimer>
#include <QRegularExpression>
#include "jsontools.h"

void Monitor::MQTTSettings::readJSON(const QJsonObject &json) {
    getJSONString(json, "Host", hostname);
    getJSONString(json, "TopCategory", topCategory);
    getJSONString(json, "DefaultTopic", defaultTopic);
    getJSONString(json, "LastWillTopic", willTopic);
    getJSONString(json, "StartupMessage", startMessage);
    getJSONString(json, "DefaultMessage", defaultMessage);
    getJSONString(json, "LastWillMessage", willMessage);
    getJSONInt(json, "Port", port);
    getJSONInt(json, "MessageQoS", reinterpret_cast<int&>(msgQoS));
    getJSONInt(json, "LastWillQoS", reinterpret_cast<int&>(willQoS));
    getJSONBool(json, "MessageRetain", msgRetain);
    getJSONBool(json, "LastWillRetain", willRetain);
}

void Monitor::MQTTSettings::writeJSON(QJsonObject &json) const {
    json["Host"] = hostname;
    json["Port"] = int(port);
    json["TopCategory"] = topCategory;
    json["DefaultTopic"] = defaultTopic;
    json["LastWillTopic"] = willTopic;
    json["StartupMessage"] = startMessage;
    json["DefaultMessage"] = defaultMessage;
    json["LastWillMessage"] = willMessage;
    json["MessageQoS"] = msgQoS;
    json["LastWillQoS"] = willQoS;
    json["MessageRetain"] = msgRetain;
    json["LastWillRetain"] = willRetain;
}

void Monitor::SerialSettings::readJSON(const QJsonObject &json) {
    int t;
    getJSONString(json, "port", portName);
    getJSONInt(json, "baud", baudRate);
    getJSONInt(json, "dataBits", t);
    dataBits = QSerialPort::DataBits(t);
    getJSONInt(json, "parity", t);
    parity = QSerialPort::Parity(t);
    getJSONInt(json, "stopBits", t);
    stopBits = QSerialPort::StopBits(t);
    getJSONInt(json, "flowControl", t);
    flowControl = QSerialPort::FlowControl(t);
}

void Monitor::SerialSettings::writeJSON(QJsonObject &json) const {
    json["port"] = portName;
    json["baud"] = baudRate;
    json["dataBits"] = dataBits;
    json["parity"] = parity;
    json["stopBits"] = stopBits;
    json["flowControl"] = flowControl;
}


Monitor::Monitor(QObject *parent) :
    QObject(parent),
    serviceRunning(false),
    portConnected(false),
    mqttConnected(false),
    mqttSettings(
        {
            "192.168.1.30", 1883,
            "DoorSensor", "state", "state",
            "DoorSensor monitor Started",
            "Unknown message received: %1",
            "DoorSensor monitor died unexpectedly.",
            1, 1, true, true
        }
    ),
    serialSettings(
        {
            "COM9", 9600,
            QSerialPort::Data8,
            QSerialPort::NoParity,
            QSerialPort::OneStop,
            QSerialPort::NoFlowControl
        }
    )
{
    connect(&m_client, SIGNAL(connected()), SLOT(onMQTTConnected()));
    connect(&m_client, SIGNAL(disconnected()), SLOT(onMQTTDisconnected()));
    connect(&m_port, SIGNAL(readyRead()), SLOT(onSerialDataReady()));
    repeatTimer.setInterval(200 /* ms */);
    repeatTimer.setSingleShot(true);
    if (!loadConfig()) {
//        setupSwitches();
        saveConfig();
    }
}

Monitor::~Monitor() {
    saveConfig();
}

bool Monitor::loadConfig() {
    QFile loadFile(QStringLiteral("save.json"));

      if (!loadFile.open(QIODevice::ReadOnly)) {
          qDaemonLog(QString("Couldn't open config file %1 for reading.").arg(QFileInfo(loadFile).absoluteFilePath()));
          return false;
      }

      QByteArray saveData = loadFile.readAll();

      QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

      readJSON(loadDoc.object());

      return true;
}

bool Monitor::saveConfig() {
    QFile saveFile(QStringLiteral("save.json"));

      if (!saveFile.open(QIODevice::WriteOnly)) {
          qDaemonLog(QString("Couldn't open config file %1 for writing.").arg(QFileInfo(saveFile).absoluteFilePath()), QDaemonLog::WarningEntry);
          return false;
      }

      QJsonObject configObject;
      writeJSON(configObject);
      QJsonDocument saveDoc(configObject);
      saveFile.write(saveDoc.toJson());

      return true;
}

void Monitor::readJSON(const QJsonObject &json) {
    if (json.contains("Serial") && json["Serial"].isObject())
        serialSettings.readJSON(json["Serial"].toObject());
    if (json.contains("MQTT") && json["MQTT"].isObject())
        mqttSettings.readJSON(json["MQTT"].toObject());
    if (json.contains("Switches") && json["Switches"].isArray()) {
        QJsonArray tmpArray = json["Switches"].toArray();
        switches.clear();
        for (auto i = tmpArray.begin(); i != tmpArray.end(); ++i) {
            Switch* newSwitch = new Switch();
            connect(newSwitch, SIGNAL(postMessage(QString, QString)), SLOT(postedMessage(QString, QString)));
            newSwitch->readJSON(i->toObject());
            QStringList messages = newSwitch->knownMessages();
            for (auto j = messages.begin(); j != messages.end(); ++j) {
                switches.insert(*j, QVariant::fromValue(newSwitch));
            }
        }
    }
}

void Monitor::writeJSON(QJsonObject &json) const {
    QJsonObject tmpObject;
    serialSettings.writeJSON(tmpObject);
    json["Serial"] = tmpObject;
    tmpObject = QJsonObject();
    mqttSettings.writeJSON(tmpObject);
    json["MQTT"] = tmpObject;
    QJsonArray tmpArray;
    for (auto i = switches.begin(); i != switches.end(); ++i) {
        tmpObject = QJsonObject();
        i.value().value<Switch*>()->writeJSON(tmpObject);
        tmpArray.append(tmpObject);
    }
    json["Switches"] = tmpArray;
}

void Monitor::connectMQTT() {
    qDaemonLog() << QString("Connecting to MQTT Server %1:%2").arg(mqttSettings.hostname, mqttSettings.port);
    m_client.setHostname(mqttSettings.hostname);
    m_client.setPort(quint16(mqttSettings.port));
    m_client.setWillTopic(mqttSettings.topCategory + "/" + mqttSettings.defaultTopic);
    m_client.setWillMessage(mqttSettings.willMessage.toUtf8());
    m_client.setWillQoS(mqttSettings.willQoS);
    m_client.setWillRetain(mqttSettings.willRetain);
    m_client.connectToHost();
}

void Monitor::onMQTTConnected() {
    qDaemonLog() << "Connected to MQTT Server.";
    auto subscription = m_client.subscribe(mqttSettings.topCategory + "/" + mqttSettings.defaultTopic);
    if (!subscription) {
        qDaemonLog("Could not subscribe. Is there a valid connection?", QDaemonLog::WarningEntry);
        return;
    }

//    if (m_client.publish((mqttSettings.topCategory + "/" + mqttSettings.defaultTopic), QString(mqttSettings.startMessage).toUtf8(), 1, true) == -1)
//        qWarning("Could not publish the message.");
      postedMessage(mqttSettings.defaultTopic, mqttSettings.startMessage);

}

void Monitor::connectSerial() {
    m_port.setPortName(serialSettings.portName);
    bool rv = m_port.open(QIODevice::ReadWrite);
    rv &= m_port.setBaudRate(serialSettings.baudRate)
            && m_port.setDataBits(serialSettings.dataBits)
            && m_port.setParity(serialSettings.parity)
            && m_port.setStopBits(serialSettings.stopBits)
            && m_port.setFlowControl(serialSettings.flowControl);
    if (rv)
    {
        qDaemonLog() << "Serial port has been opened.";
    }
    else {
        qDaemonLog("Could not open serial port.", QDaemonLog::WarningEntry);
        m_port.close();
    }
}

void Monitor::disconnectSerial() {
    m_port.close();
}

void Monitor::onSerialDataReady() {
    readBuffer.append(m_port.readAll());
    int pos;

    while ((pos = readBuffer.indexOf('\n')) != -1) {
        QByteArray line = readBuffer.left(pos).trimmed();
        readBuffer.remove(0, pos+1);
        bool msgHandled = false;

        if ( (line != prevLine) || (!repeatTimer.isActive()) ) {
            prevLine = line;
            qDaemonLog() << QString("Received: %1").arg(QString(line));
            emit receivedMessage(line);
            if (switches.contains(line)) {
                msgHandled = true;
                switches.value(line).value<Switch*>()->processMessage(line);
            }
            else {
                // We don't have an exact match, so let's see if we have a message that only differs by one byte.
                QString re = line;
                re.chop(1);
                re += '?';
                QStringList keys = switches.keys(QRegularExpression(re));
                msgHandled = keys.length();
                for (auto i = keys.begin(); i != keys.end(); ++i) {
                    switches.value(*i).value<Switch*>()->processMessage(line);
                }
            }

            if (!msgHandled) {
                QString message = mqttSettings.defaultMessage.arg(QString(line));
                qDaemonLog() << message;
//                if (m_client.publish((mqttSettings.topCategory + "/" + mqttSettings.defaultTopic), message.toUtf8(), 1, true) == -1)
//                    qWarning("Could not publish the message.");
                postedMessage(mqttSettings.defaultTopic, message);
            }
        }
        repeatTimer.start();
    }
}

void Monitor::setupSwitches() {
    Switch* newSwitch = new Switch("Switch1", this);
    connect(newSwitch, SIGNAL(postMessage(QString, QString)), SLOT(postedMessage(QString, QString)));
    newSwitch->addMessage("1-24-abcd", "I got a Message!");
    switches.insert("1-24-abcd", QVariant::fromValue(newSwitch));
}

void Monitor::postedMessage(const QString& topic, const QString& message) {
    qDaemonLog() << QString("Message to '%1' : %2").arg(topic, message);
    if (m_client.publish((mqttSettings.topCategory + "/" + topic), message.toUtf8(), mqttSettings.msgQoS, mqttSettings.msgRetain) == -1)
        qDaemonLog("Could not publish the message.", QDaemonLog::WarningEntry);

}

void Monitor::onServiceStarted() {
    connectMQTT();
    connectSerial();
}

void Monitor::onServiceStopped() {
    disconnectSerial();
    m_client.disconnectFromHost();
}

void Monitor::onServiceInstalled() {

}

void Monitor::onServiceUninstalled() {

}

void Monitor::onMQTTDisconnected() {

}
