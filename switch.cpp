#include "switch.h"
#include <QJsonArray>
#include "jsontools.h"

void Switch::SwitchMessage::readJSON(const QJsonObject &json) {
    getJSONString(json, "InMessage", inMessage);
    getJSONString(json, "OutMessage", outMessage);
    getJSONInt(json, "TimerEffect", reinterpret_cast<int&>(effect));
}

void Switch::SwitchMessage::writeJSON(QJsonObject &json) const {
    json["InMessage"] = inMessage;
    json["OutMessage"] = outMessage;
    json["TimerEffect"] = effect;
}
#if 0
Switch::Switch(QObject *parent) :
    QObject(parent),
    mTimerMessage("The timer has elapsed.")
{
    repeatTimer.setInterval(1 * 60 * 1000);
    repeatTimer.setSingleShot(false);
    connect(&repeatTimer, SIGNAL(timeout()), SLOT(sendTimerMessage()));
}
#endif

Switch::Switch(const QString &name, QObject *parent) :
    QObject(parent),
    mName(name),
    mTimerMessage("The timer has elapsed.")
{
    repeatTimer.setInterval(1 * 60 * 1000);
    repeatTimer.setSingleShot(false);
    connect(&repeatTimer, SIGNAL(timeout()), SLOT(sendTimerMessage()));
}

Switch::~Switch() {
    auto i = messages.begin();
    while (i != messages.end()) {
        delete *i;
        i = messages.erase(i);
    }
}
void Switch::setTimer(const QString& timerMessage, int timerPeriod /*ms*/) {
    mTimerMessage = timerMessage;
    repeatTimer.setInterval(timerPeriod);
}

bool Switch::addMessage(const QString& inMessage, const QString& outMessage, TimerEffect effect) {
    if (messages.contains(inMessage)) { return false; }

    SwitchMessage* msg = new SwitchMessage(inMessage, outMessage, effect);
    messages.insert(const_cast<QString&>(inMessage), msg);
    return true;
}

void Switch::readJSON(const QJsonObject &json) {
    int tmpInt;
    getJSONString(json, "Name", mName);
    getJSONString(json, "TimerMessage", mTimerMessage);
    getJSONInt(json, "TimerLen", tmpInt);
    repeatTimer.setInterval(tmpInt);
    if (json.contains("Messages") && json["Messages"].isArray()) {
        QJsonArray tmpArray = json["Messages"].toArray();
        messages.clear();
        for (auto i = tmpArray.begin(); i != tmpArray.end(); ++i) {
            SwitchMessage* newMessage = new SwitchMessage();
            newMessage->readJSON(i->toObject());
            messages.insert(newMessage->inMessage, newMessage);
        }
    }
}

void Switch::writeJSON(QJsonObject &json) const {
    json["Name"] = mName;
    json["TimerLen"] = repeatTimer.interval();
    json["TimerMessage"] = mTimerMessage;
    QJsonArray _messages;
    for (auto i = messages.begin(); i != messages.end(); ++i) {
        QJsonObject msg;
        (*i)->writeJSON(msg);
        _messages.append(msg);
    }
    json["Messages"] = _messages;
}

void Switch::processMessage(const QString& msg) {
    const SwitchMessage blankMsg;
    if (messages.contains(msg)) {
        SwitchMessage* m = messages.value(msg);
        emit postMessage(mName, m->outMessage);
        switch(m->effect) {
        case TimerStart:
            repeatTimer.start();
            break;
        case TimerStop:
            repeatTimer.stop();
            break;
        default:
            break;
        }
    }
    else {
        emit postMessage(mName, QString("%1 received an unknown message (%2)").arg(mName, msg));
    }
}

void Switch::sendTimerMessage() {
        emit postMessage(mName, mTimerMessage);
}

Switch::SwitchMessage::SwitchMessage(const QString& _inMessage, const QString& _outMessage, TimerEffect _effect):
    inMessage(_inMessage),
    outMessage(_outMessage),
    effect(_effect)
{}

Switch::SwitchMessage::SwitchMessage(const SwitchMessage& other):
    inMessage(other.inMessage),
    outMessage(other.outMessage),
    effect(other.effect)
{}

Switch::SwitchMessage::SwitchMessage() {}
