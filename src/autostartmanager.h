#pragma once

#include <QString>

class AutostartManager final
{
public:
    static bool isEnabled(QString *errorMessage = nullptr);
    static bool setEnabled(bool enabled, QString *errorMessage = nullptr);
};
