//#include <QCoreApplication>
#include <QDaemonApplication>
#include "monitor.h"

int main(int argc, char *argv[])
{
    QDaemonApplication a(argc, argv);
    QDaemonApplication::setApplicationName("433MHz MQTT Bridge");
    QDaemonApplication::setApplicationDescription("This application monitors an array of 433MHz sensors for known messages and relays them to an MQTT broker.");
    QDaemonApplication::setOrganizationDomain("melstav.lakotacreations.com");

    Monitor mon(&a);

//    QObject::connect(&a, &QDaemonApplication::daemonized, &mon, &SimpleDaemon::onDaemonReady);
    QObject::connect(&a, &QDaemonApplication::daemonized, &mon, &Monitor::onServiceStarted);
    QObject::connect(&a, &QDaemonApplication::started, &mon, &Monitor::onServiceStarted);
    QObject::connect(&a, &QDaemonApplication::stopped, &mon, &Monitor::onServiceStopped);
//    QObject::connect(&a, &QDaemonApplication::installed, &mon, &Monitor::onServiceInstalled);
//    QObject::connect(&a, &QDaemonApplication::uninstalled, &mon, &Monitor::onServiceUninstalled);

    return a.exec();
}
