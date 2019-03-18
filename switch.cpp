#include "switch.h"

Switch::Switch(const QString &name, QObject *parent) :
    QObject(parent),
    mName(name),
    mTimerMessage("The timer has elapsed.")
{
    repeatTimer.setInterval(1 * 60 * 1000);
    repeatTimer.setSingleShot(false);
    connect(&repeatTimer, SIGNAL(timeout()), SLOT(timerMessage()));
}

void Switch::setTimer(const QString& timerMessage, int timerPeriod /*ms*/) {
    mTimerMessage = timerMessage;
    repeatTimer.setInterval(timerPeriod);
}

bool Switch::addMessage(const QString& inMessage, const QString& outMessage, TimerEffect effect) {
    if (messages.contains(inMessage)) { return false; }

    SwitchMessage msg(inMessage, outMessage, effect);
    messages.insert(inMessage, msg);
    return true;
}

void Switch::processMessage(const QString& msg) {
    const SwitchMessage blankMsg;
    if (messages.contains(msg)) {
        SwitchMessage m = messages.value(msg);
        emit postMessage(mName, m.outMessage);
        switch(m.effect) {
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

void Switch::timerMessage() {
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
