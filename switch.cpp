#include "switch.h"
#include <QJsonArray>
#include "jsontools.h"

void Switch::SwitchMessage::readJSON(const QJsonObject &json) {
    int t;
    getJSONString(json, "InMessage", inMessage);
    getJSONString(json, "OutMessage", outMessage);
    getJSONInt(json, "TimerEffect", t);
    effect = Switch::TimerEffect(t);
}

void Switch::SwitchMessage::writeJSON(QJsonObject &json) const {
    json["InMessage"] = inMessage;
    json["OutMessage"] = outMessage;
    json["TimerEffect"] = effect;
}

Switch::Switch(const QString &name, QObject *parent) :
    QObject(parent),
    delayMessage(nullptr),
    mName(name),
    mTimerMessage("The timer has elapsed.")
{
    delayTimerLen = repeatTimerLen = 0;
    myTimer.setInterval(1 * 60 * 1000);
    myTimer.setSingleShot(false);
    connect(&myTimer, SIGNAL(timeout()), SLOT(sendTimerMessage()));
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
    myTimer.setInterval(timerPeriod);
}

bool Switch::addMessage(const QString& inMessage, const QString& outMessage, TimerEffect effect) {
    if (messages.contains(inMessage)) { return false; }

    SwitchMessage* msg = new SwitchMessage(inMessage, outMessage, effect);
    messages.insert(const_cast<QString&>(inMessage), msg);
    return true;
}

void Switch::readJSON(const QJsonObject &json) {
    getJSONString(json, "Name", mName);
    getJSONString(json, "TimerMessage", mTimerMessage);
    getJSONInt(json, "TimerLen", repeatTimerLen);
    myTimer.setInterval(repeatTimerLen);
    if (!getJSONInt(json, "DelayLen", delayTimerLen)) { delayTimerLen = 0; }
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
    json["TimerLen"] = repeatTimerLen;
    json["DelayLen"] = delayTimerLen;
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
        switch(m->effect) {
        case TimerStart:
            if (delayTimerLen > 0) {
                delayMessage = m;
                myTimer.start(delayTimerLen);
            }
            else {
                delayMessage = nullptr;
                emit postMessage(mName, m->outMessage);
                myTimer.start(repeatTimerLen);
            }
            break;

        case TimerStop:
            if (!delayMessage) {
                emit postMessage(mName, m->outMessage);
            }
            else {
                delayMessage = nullptr;
            }
            myTimer.stop();
            break;

        case TimerNoEffect:
            emit postMessage(mName, m->outMessage);
            break;
        }
    }
    else {
        emit postMessage(mName, QString("%1 received an unknown message (%2)").arg(mName, msg));
    }
}

void Switch::sendTimerMessage() {
    if (delayMessage) {
        emit postMessage(mName, delayMessage->outMessage);
        if (delayMessage->effect == TimerStart) {
            myTimer.start(repeatTimerLen);
        }
        delayMessage = nullptr;
    }
    else {
        emit postMessage(mName, mTimerMessage);
    }
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
