#pragma once

#include <QString>

struct LauncherConfig
{
    QString pythonExecutable;
    QString workingDirectory;
    QString serverArguments;
    QString applicationUrl;
    QString healthCheckUrl;
    bool restartOnFailure = true;
    int restartDelaySeconds = 5;
    bool autoOpenBrowser = false;
    bool autoStartWithSystem = false;
    bool enableAutoUpdateCheck = false;
    QString updateVersionUrl;
    QString updateInstallerUrl;

    static LauncherConfig defaults();
    static LauncherConfig load(const QString &filePath, QString *errorMessage = nullptr);
    bool save(const QString &filePath, QString *errorMessage = nullptr) const;
};
