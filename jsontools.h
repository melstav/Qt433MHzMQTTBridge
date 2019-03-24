#ifndef JSONTOOLS_H
#define JSONTOOLS_H

#include <QString>
#include <QJsonObject>

bool getJSONString(const QJsonObject &json, const QString &valName, QString& valDest);
bool getJSONDouble(const QJsonObject &json, const QString &valName, double& valDest);
bool getJSONInt(const QJsonObject &json, const QString &valName, int& valDest);
bool getJSONBool(const QJsonObject &json, const QString &valName, bool& valDest);
bool getJSONArray(const QJsonObject &json, const QString &valName, QJsonArray& valDest);

#endif // JSONTOOLS_H
