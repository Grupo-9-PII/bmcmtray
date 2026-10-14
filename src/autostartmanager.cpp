#include "autostartmanager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSaveFile>
#include <QStandardPaths>

#ifdef Q_OS_WIN
namespace
{
    QSettings startupSettings()
    {
        return QSettings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                         QSettings::NativeFormat);
    }
}
#else
namespace
{
    QString startupFilePath()
    {
        return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart/bmcm-tray-launcher-qt.desktop");
    }

    QString desktopExecValue(QString value)
    {
        value.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
        value.replace(QLatin1Char('"'), QStringLiteral("\\\""));
        value.replace(QLatin1Char('$'), QStringLiteral("\\$"));
        value.replace(QLatin1Char('`'), QStringLiteral("\\`"));
        value.replace(QLatin1Char('%'), QStringLiteral("%%"));
        return QStringLiteral("\"") + value + QStringLiteral("\"");
    }
}
#endif

bool AutostartManager::isEnabled(QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

#ifdef Q_OS_WIN
    QSettings settings = startupSettings();
    const bool enabled = settings.contains(QStringLiteral("BMCMTrayLauncherQt"));
    if (settings.status() != QSettings::NoError && errorMessage)
        *errorMessage = QStringLiteral("Não foi possível consultar o autostart do usuário.");
    return enabled;
#elif defined(Q_OS_LINUX)
    return QFile::exists(startupFilePath());
#else
    if (errorMessage)
        *errorMessage = QStringLiteral("Autostart ainda não é suportado nesta plataforma.");
    return false;
#endif
}

bool AutostartManager::setEnabled(bool enabled, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

#ifdef Q_OS_WIN
    QSettings settings = startupSettings();
    const QString key = QStringLiteral("BMCMTrayLauncherQt");
    if (enabled)
        settings.setValue(key, QStringLiteral("\"%1\" --autostart").arg(
                                   QDir::toNativeSeparators(QCoreApplication::applicationFilePath())));
    else
        settings.remove(key);
    settings.sync();

    if (settings.status() != QSettings::NoError)
    {
        if (errorMessage)
            *errorMessage = QStringLiteral("Não foi possível atualizar o autostart do usuário.");
        return false;
    }
    return true;
#elif defined(Q_OS_LINUX)
    const QString path = startupFilePath();
    if (!enabled)
    {
        if (!QFile::exists(path))
            return true;
        if (!QFile::remove(path))
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("Não foi possível remover o atalho de autostart: %1").arg(path);
            return false;
        }
        return true;
    }

    const QFileInfo fileInfo(path);
    if (!QDir().mkpath(fileInfo.absolutePath()))
    {
        if (errorMessage)
            *errorMessage = QStringLiteral("Não foi possível criar a pasta de autostart do usuário.");
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    const QString executablePath = QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath();
    if (!QFileInfo(executablePath).isExecutable())
    {
        if (errorMessage)
            *errorMessage = QStringLiteral("O executável do launcher não existe ou não pode ser executado: %1")
                                .arg(executablePath);
        return false;
    }

    const QString desktopEntry = QStringLiteral(
                                     "[Desktop Entry]\n"
                                     "Type=Application\n"
                                     "Version=1.0\n"
                                     "Name=BMCM Tray Launcher\n"
                                     "Comment=Inicia o launcher BMCM com o sistema.\n"
                                     "Exec=%1 --autostart\n"
                                     "Terminal=false\n"
                                     "NoDisplay=true\n"
                                     "StartupNotify=false\n"
                                     "X-GNOME-Autostart-enabled=true\n")
                                     .arg(desktopExecValue(executablePath));
    const QByteArray contents = desktopEntry.toUtf8();
    if (file.write(contents) != contents.size())
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    const QFile::Permissions permissions = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
                                          | QFile::ReadGroup | QFile::ExeGroup
                                          | QFile::ReadOther | QFile::ExeOther;
    if (!file.setPermissions(permissions))
    {
        if (errorMessage)
            *errorMessage = QStringLiteral("Não foi possível ajustar as permissões do atalho de autostart: %1")
                                .arg(path);
        return false;
    }
    if (!file.commit())
    {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }
    return true;
#else
    Q_UNUSED(enabled)
    if (errorMessage)
        *errorMessage = QStringLiteral("Autostart ainda não é suportado nesta plataforma.");
    return false;
#endif
}
