#ifndef SWITCH_H
#define SWITCH_H

#include <QObject>
#include <QString>
#include <QPair>
#include <QList>
#include <QTimer>
#include <QMap>


class Switch : public QObject
{
    Q_OBJECT
public:
    enum TimerEffect {
        TimerNoEffect = 0,
        TimerStart,
        TimerStop
    };

    class SwitchMessage {
    public:
        QString inMessage;
        QString outMessage;
        TimerEffect effect;

        SwitchMessage(const QString& _inMessage, const QString& _outMessage, TimerEffect _effect = TimerNoEffect);
        SwitchMessage(const SwitchMessage& other);
        SwitchMessage();
    };

    explicit Switch(const QString& name, QObject *parent = nullptr);
    void setTimer(const QString& timerMessage, int timerPeriod /*ms*/);
    bool addMessage(const QString& inMessage, const QString& outMessage, TimerEffect effect = TimerNoEffect);

protected:
    QTimer repeatTimer;
    QString mName, mTimerMessage;
    QMap<QString, SwitchMessage> messages;

signals:
    void postMessage(const QString& name, const QString& message);

public slots:
    void processMessage(const QString& msg);

protected slots:
    void timerMessage();

};

#endif // SWITCH_H
