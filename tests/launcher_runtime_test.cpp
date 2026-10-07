#include "backendcontroller.h"
#include "applicationinstance.h"
#include "autostartmanager.h"
#include "launcherconfig.h"
#include "launcherupdater.h"
#include "readinesschecker.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QTemporaryDir>
#include <QTimer>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("BMCMTrayLauncherQtTest"));
    QCoreApplication::setOrganizationName(QStringLiteral("BMCM"));

    QTemporaryDir temporaryDirectory;
    if (!temporaryDirectory.isValid())
    {
        std::cerr << "Could not create temporary test directory\n";
        return 1;
    }

    LauncherConfig expected = LauncherConfig::defaults();
    expected.pythonExecutable = QStringLiteral(TEST_CMAKE_EXECUTABLE);
    expected.workingDirectory = temporaryDirectory.path();
    expected.serverArguments = QStringLiteral("-E echo launcher-runtime-smoke");
    expected.applicationUrl = QStringLiteral("http://127.0.0.1:8080");
    expected.healthCheckUrl = QStringLiteral("http://127.0.0.1:8080/health");
    expected.autoStartWithSystem = true;
    expected.enableAutoUpdateCheck = true;
    expected.updateVersionUrl = QStringLiteral("https://updates.example.test/version.txt");
    expected.updateInstallerUrl = QStringLiteral("https://updates.example.test/launcher.exe");

    const QString configPath = temporaryDirectory.filePath(QStringLiteral("appsettings.json"));
    QString errorMessage;
    if (!expected.save(configPath, &errorMessage))
    {
        std::cerr << "Could not save launcher config: " << errorMessage.toStdString() << '\n';
        return 1;
    }

    const QVersionNumber currentVersion(1, 1, 6);
    QVersionNumber parsedVersion;
    if (!LauncherUpdater::isNewerVersion(QStringLiteral("1.1.7"), currentVersion, &parsedVersion) || parsedVersion != QVersionNumber(1, 1, 7) || LauncherUpdater::isNewerVersion(QStringLiteral("1.1.6"), currentVersion) || LauncherUpdater::isNewerVersion(QStringLiteral("1.1.5"), currentVersion) || LauncherUpdater::isNewerVersion(QStringLiteral("1.1.7-beta"), currentVersion) || LauncherUpdater::isNewerVersion(QStringLiteral("1.1"), currentVersion))
    {
        std::cerr << "Launcher update version comparison failed\n";
        return 1;
    }

    const LauncherConfig loaded = LauncherConfig::load(configPath, &errorMessage);
    if (!errorMessage.isEmpty() || loaded.pythonExecutable != expected.pythonExecutable || loaded.workingDirectory != expected.workingDirectory || loaded.serverArguments != expected.serverArguments || loaded.applicationUrl != expected.applicationUrl || loaded.healthCheckUrl != expected.healthCheckUrl || loaded.autoStartWithSystem != expected.autoStartWithSystem || loaded.enableAutoUpdateCheck != expected.enableAutoUpdateCheck || loaded.updateVersionUrl != expected.updateVersionUrl || loaded.updateInstallerUrl != expected.updateInstallerUrl)
    {
        std::cerr << "Launcher config save/load check failed\n";
        return 1;
    }

    ApplicationInstance firstInstance;
    ApplicationInstance secondInstance;
    if (!firstInstance.acquire() || secondInstance.acquire())
    {
        std::cerr << "Single-instance lock check failed\n";
        return 1;
    }

    QTcpServer healthServer;
    if (!healthServer.listen(QHostAddress::LocalHost, 0))
    {
        std::cerr << "Could not start local health test server\n";
        return 1;
    }
    QObject::connect(&healthServer, &QTcpServer::newConnection, &healthServer, [&healthServer]
                     {
        while (QTcpSocket *socket = healthServer.nextPendingConnection()) {
            QObject::connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
            QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket] {
                socket->readAll();
                socket->write("HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nok");
                socket->disconnectFromHost();
            });
        } });

    ReadinessChecker readinessChecker;
    QEventLoop readinessLoop;
    QTimer readinessTimeout;
    readinessTimeout.setSingleShot(true);
    bool healthCheckSucceeded = false;
    QObject::connect(&readinessChecker, &ReadinessChecker::ready, &application,
                     [&readinessLoop, &healthCheckSucceeded](const QUrl &)
                     {
                         healthCheckSucceeded = true;
                         readinessLoop.quit();
                     });
    QObject::connect(&readinessChecker, &ReadinessChecker::timedOut, &application,
                     [&readinessLoop](const QUrl &)
                     { readinessLoop.quit(); });
    QObject::connect(&readinessTimeout, &QTimer::timeout, &readinessLoop, &QEventLoop::quit);
    readinessTimeout.start(5000);
    readinessChecker.start(QUrl(QStringLiteral("http://127.0.0.1:%1/health")
                                    .arg(healthServer.serverPort())),
                           4000);
    readinessLoop.exec();
    readinessChecker.stop();
    if (!healthCheckSucceeded)
    {
        std::cerr << "HTTP readiness check failed for healthy local server\n";
        return 1;
    }

    const quint16 closedHealthPort = healthServer.serverPort();
    healthServer.close();
    ReadinessChecker unavailableChecker;
    QEventLoop unavailableLoop;
    QTimer unavailableTimeout;
    unavailableTimeout.setSingleShot(true);
    bool unavailableTimedOut = false;
    QObject::connect(&unavailableChecker, &ReadinessChecker::ready, &application,
                     [&unavailableLoop](const QUrl &)
                     { unavailableLoop.quit(); });
    QObject::connect(&unavailableChecker, &ReadinessChecker::timedOut, &application,
                     [&unavailableLoop, &unavailableTimedOut](const QUrl &)
                     {
                         unavailableTimedOut = true;
                         unavailableLoop.quit();
                     });
    QObject::connect(&unavailableTimeout, &QTimer::timeout, &unavailableLoop, &QEventLoop::quit);
    unavailableTimeout.start(3000);
    unavailableChecker.start(QUrl(QStringLiteral("http://127.0.0.1:%1/health")
                                      .arg(closedHealthPort)),
                             1000);
    unavailableLoop.exec();
    unavailableChecker.stop();
    if (!unavailableTimedOut)
    {
        std::cerr << "HTTP readiness timeout check failed for unavailable server\n";
        return 1;
    }

    BackendController backend;
    QStringList outputLines;
    QEventLoop eventLoop;
    QTimer timeout;
    timeout.setSingleShot(true);

    bool processFinished = false;
    int exitCode = -1;
    QObject::connect(&backend, &BackendController::outputReceived, &application,
                     [&outputLines](const QString &line)
                     { outputLines.append(line); });
    QObject::connect(&backend, &BackendController::processFinished, &application,
                     [&eventLoop, &processFinished, &exitCode](int code, bool)
                     {
                         processFinished = true;
                         exitCode = code;
                         eventLoop.quit();
                     });
    QObject::connect(&backend, &BackendController::errorOccurred, &application,
                     [&eventLoop, &errorMessage](const QString &message)
                     {
                         errorMessage = message;
                         eventLoop.quit();
                     });
    QObject::connect(&timeout, &QTimer::timeout, &eventLoop, &QEventLoop::quit);

    timeout.start(10000);
    backend.start(expected);
    eventLoop.exec();

    if (!errorMessage.isEmpty() || !processFinished || exitCode != 0 || !outputLines.join(QLatin1Char('\n')).contains(QStringLiteral("launcher-runtime-smoke")))
    {
        std::cerr << "Backend process check failed";
        if (!errorMessage.isEmpty())
            std::cerr << ": " << errorMessage.toStdString();
        std::cerr << '\n';
        return 1;
    }

    std::cout << "Launcher configuration and backend process checks passed\n";
    return 0;
}
