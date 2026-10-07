#include "launcherconfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

LauncherConfig LauncherConfig::defaults()
{
    LauncherConfig config;
    config.pythonExecutable = QStringLiteral("python");
    config.workingDirectory = QDir::homePath();
    config.applicationUrl = QStringLiteral("http://127.0.0.1:8080");

#ifdef Q_OS_WIN
    config.serverArguments = QStringLiteral("-m waitress --listen=0.0.0.0:8080 wsgi:app");
#else
    config.serverArguments = QStringLiteral("-m gunicorn --bind 0.0.0.0:8080 wsgi:app");
#endif

    return config;
}

LauncherConfig LauncherConfig::load(const QString &filePath, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    QFile file(filePath);
    if (!file.exists())
        return defaults();

    if (!file.open(QIODevice::ReadOnly))
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return defaults();
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        if (errorMessage)
            *errorMessage = parseError.errorString();
        return defaults();
    }

    const QJsonObject object = document.object();
    LauncherConfig config = defaults();
    config.pythonExecutable = object.value(QStringLiteral("PythonExePath")).toString(config.pythonExecutable);
    config.workingDirectory = object.value(QStringLiteral("WorkingDirectory")).toString(config.workingDirectory);
    config.serverArguments = object.value(QStringLiteral("ServerArguments"))
                                 .toString(object.value(QStringLiteral("WaitressArguments"))
                                               .toString(config.serverArguments));
    config.applicationUrl = object.value(QStringLiteral("ApplicationUrl")).toString(config.applicationUrl);
    config.healthCheckUrl = object.value(QStringLiteral("HealthCheckUrl")).toString();
    config.restartOnFailure = object.value(QStringLiteral("RestartOnFailure")).toBool(config.restartOnFailure);
    config.restartDelaySeconds = object.value(QStringLiteral("RestartDelaySeconds")).toInt(config.restartDelaySeconds);
    config.autoOpenBrowser = object.value(QStringLiteral("AutoOpenBrowser")).toBool(config.autoOpenBrowser);
    config.autoStartWithSystem = object.value(QStringLiteral("AutoStartWithSystem")).toBool(config.autoStartWithSystem);
    config.enableAutoUpdateCheck = object.value(QStringLiteral("EnableAutoUpdateCheck")).toBool(config.enableAutoUpdateCheck);
    config.updateVersionUrl = object.value(QStringLiteral("GitHubRawVersionUrl")).toString();
    config.updateInstallerUrl = object.value(QStringLiteral("GitHubRawInstallerUrl")).toString();
    return config;
}

bool LauncherConfig::save(const QString &filePath, QString *errorMessage) const
{
    if (errorMessage)
        errorMessage->clear();

    const QFileInfo fileInfo(filePath);
    if (!QDir().mkpath(fileInfo.absolutePath()))
    {
        if (errorMessage)
            *errorMessage = QStringLiteral("Não foi possível criar a pasta de configuração.");
        return false;
    }

    QJsonObject object;
    object.insert(QStringLiteral("PythonExePath"), pythonExecutable);
    object.insert(QStringLiteral("WorkingDirectory"), workingDirectory);
    object.insert(QStringLiteral("ServerArguments"), serverArguments);
    object.insert(QStringLiteral("ApplicationUrl"), applicationUrl);
    object.insert(QStringLiteral("HealthCheckUrl"), healthCheckUrl);
    object.insert(QStringLiteral("RestartOnFailure"), restartOnFailure);
    object.insert(QStringLiteral("RestartDelaySeconds"), restartDelaySeconds);
    object.insert(QStringLiteral("AutoOpenBrowser"), autoOpenBrowser);
    object.insert(QStringLiteral("AutoStartWithSystem"), autoStartWithSystem);
    object.insert(QStringLiteral("EnableAutoUpdateCheck"), enableAutoUpdateCheck);
    object.insert(QStringLiteral("GitHubRawVersionUrl"), updateVersionUrl);
    object.insert(QStringLiteral("GitHubRawInstallerUrl"), updateInstallerUrl);

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    if (!file.commit())
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    return true;
}
