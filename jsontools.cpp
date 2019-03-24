#include <QJsonObject>
#include <QJsonArray>
#include "jsontools.h"

bool getJSONString(const QJsonObject &json, const QString &valName, QString& valDest) {
    if (json.contains(valName) && json[valName].isString()) {
        valDest = json[valName].toString();
        return true;
    }
    return false;
}

bool getJSONDouble(const QJsonObject &json, const QString &valName, double& valDest) {
    if (json.contains(valName) && json[valName].isDouble()) {
        valDest = json[valName].toDouble();
        return true;
    }
    return false;
}

bool getJSONInt(const QJsonObject &json, const QString &valName, int& valDest) {
    if (json.contains(valName) && json[valName].isDouble()) {
        valDest = json[valName].toInt();
        return true;
    }
    return false;
}

bool getJSONBool(const QJsonObject &json, const QString &valName, bool& valDest) {
    if (json.contains(valName) && json[valName].isBool()) {
        valDest = json[valName].toBool();
        return true;
    }
    return false;
}

bool getJSONArray(const QJsonObject &json, const QString &valName, QJsonArray& valDest) {
    if (json.contains(valName) && json[valName].isArray()) {
        valDest = json[valName].toArray();
        return true;
    }
    return false;
}
