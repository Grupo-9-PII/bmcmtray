#include "backendcontroller.h"
#include "applicationinstance.h"
#include "autostartmanager.h"
#include "launcherconfig.h"
#include "launcherreleaseupdater.h"
#include "launcherupdater.h"
#include "readinesschecker.h"
#include "systemrepositoryupdater.h"

#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QHostAddress>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>

#include <iostream>

namespace
{
bool runCommand(const QString &program, const QStringList &arguments, const QString &workingDirectory,
                QString *errorMessage = nullptr)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(workingDirectory);
    process.start();
    if (!process.waitForStarted(5000) || !process.waitForFinished(15000)
        || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        if (errorMessage)
            *errorMessage = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        return false;
    }
    return true;
}

bool writeFile(const QString &path, const QByteArray &contents)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly | QIODevice::Truncate) && file.write(contents) == contents.size();
}

bool waitForSystemUpdate(SystemRepositoryUpdater &updater, const QString &repositoryDirectory,
                         const QString &repositoryUrl, bool *success, bool *updated, QString *message)
{
    QEventLoop eventLoop;
    QTimer timeout;
    timeout.setSingleShot(true);
    bool completed = false;
    const QMetaObject::Connection connection = QObject::connect(
        &updater, &SystemRepositoryUpdater::finished, &eventLoop,
        [&eventLoop, &completed, success, updated, message](bool result, bool changed, const QString &status)
        {
            completed = true;
            *success = result;
            *updated = changed;
            *message = status;
            eventLoop.quit();
        });
    QObject::connect(&timeout, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timeout.start(60000);
    updater.checkAndUpdate(repositoryDirectory, repositoryUrl);
    if (updater.isRunning())
        eventLoop.exec();
    QObject::disconnect(connection);
    return completed;
}
}

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
    expected.systemRepositoryUrl = QStringLiteral("https://github.com/example/sistemaBMCM-teste");
    expected.systemUpdateMode = QStringLiteral("archive");
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

    QVersionNumber releaseVersion;
    if (!LauncherReleaseUpdater::isNewerReleaseTag(QStringLiteral("v1.2.0"), currentVersion, &releaseVersion)
        || releaseVersion != QVersionNumber(1, 2, 0)
        || LauncherReleaseUpdater::isNewerReleaseTag(QStringLiteral("v1.1.6"), currentVersion)
        || LauncherReleaseUpdater::isNewerReleaseTag(QStringLiteral("v1.1.7-beta"), currentVersion)
        || LauncherReleaseUpdater::isNewerReleaseTag(QStringLiteral("latest"), currentVersion))
    {
        std::cerr << "GitHub launcher release version comparison failed\n";
        return 1;
    }

    const LauncherConfig loaded = LauncherConfig::load(configPath, &errorMessage);
    if (!errorMessage.isEmpty() || loaded.pythonExecutable != expected.pythonExecutable || loaded.workingDirectory != expected.workingDirectory || loaded.systemRepositoryUrl != expected.systemRepositoryUrl || loaded.systemUpdateMode != expected.systemUpdateMode || loaded.serverArguments != expected.serverArguments || loaded.applicationUrl != expected.applicationUrl || loaded.healthCheckUrl != expected.healthCheckUrl || loaded.autoStartWithSystem != expected.autoStartWithSystem || loaded.enableAutoUpdateCheck != expected.enableAutoUpdateCheck || loaded.updateVersionUrl != expected.updateVersionUrl || loaded.updateInstallerUrl != expected.updateInstallerUrl)
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

    const QString gitExecutable = QStandardPaths::findExecutable(QStringLiteral("git"));
    if (!gitExecutable.isEmpty())
    {
        QTemporaryDir gitDirectory;
        const QString bareRepository = gitDirectory.filePath(QStringLiteral("remote.git"));
        const QString sourceRepository = gitDirectory.filePath(QStringLiteral("source"));
        const QString installedRepository = gitDirectory.filePath(QStringLiteral("installed"));
        QString gitError;
        const QString gitInit = QStringLiteral("-c");
        const bool gitFixtureReady =
            runCommand(gitExecutable, {gitInit, QStringLiteral("init.defaultBranch=main"),
                                       QStringLiteral("init"), QStringLiteral("--bare"), bareRepository},
                       gitDirectory.path(), &gitError)
            && QDir().mkpath(sourceRepository)
            && runCommand(gitExecutable, {gitInit, QStringLiteral("init.defaultBranch=main"),
                                          QStringLiteral("init"), sourceRepository},
                          gitDirectory.path(), &gitError)
            && runCommand(gitExecutable, {QStringLiteral("config"), QStringLiteral("user.name"), QStringLiteral("Launcher Test")},
                          sourceRepository, &gitError)
            && runCommand(gitExecutable, {QStringLiteral("config"), QStringLiteral("user.email"), QStringLiteral("launcher-test@example.test")},
                          sourceRepository, &gitError)
            && writeFile(sourceRepository + QStringLiteral("/.gitignore"), "instance/\n")
            && writeFile(sourceRepository + QStringLiteral("/app.py"), "value = 'initial'\n")
            && runCommand(gitExecutable, {QStringLiteral("add"), QStringLiteral(".")}, sourceRepository, &gitError)
            && runCommand(gitExecutable, {QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("Initial backend")},
                          sourceRepository, &gitError)
            && runCommand(gitExecutable, {QStringLiteral("remote"), QStringLiteral("add"), QStringLiteral("origin"), bareRepository},
                          sourceRepository, &gitError)
            && runCommand(gitExecutable, {QStringLiteral("push"), QStringLiteral("-u"), QStringLiteral("origin"), QStringLiteral("main")},
                          sourceRepository, &gitError)
            && runCommand(gitExecutable, {QStringLiteral("clone"), QStringLiteral("--branch"), QStringLiteral("main"),
                                          bareRepository, installedRepository},
                          gitDirectory.path(), &gitError)
            && QDir().mkpath(installedRepository + QStringLiteral("/instance"))
            && writeFile(installedRepository + QStringLiteral("/instance/database.db"), "preserve-this-database");
        if (!gitFixtureReady)
        {
            std::cerr << "Could not prepare local Git updater test: " << gitError.toStdString() << '\n';
            return 1;
        }

        SystemRepositoryUpdater systemUpdater(QStringLiteral("main"));
        bool updateSucceeded = false;
        bool updateApplied = false;
        QString updateMessage;
        if (!waitForSystemUpdate(systemUpdater, installedRepository, sourceRepository,
                                 &updateSucceeded, &updateApplied, &updateMessage)
            || !updateSucceeded || updateApplied)
        {
            std::cerr << "System updater failed the already-current repository check: "
                      << updateMessage.toStdString() << '\n';
            return 1;
        }

        if (!writeFile(sourceRepository + QStringLiteral("/app.py"), "value = 'updated'\n")
            || !runCommand(gitExecutable, {QStringLiteral("add"), QStringLiteral("app.py")}, sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("Update backend")},
                           sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("push"), QStringLiteral("origin"), QStringLiteral("main")},
                           sourceRepository, &gitError))
        {
            std::cerr << "Could not publish local Git updater test update: " << gitError.toStdString() << '\n';
            return 1;
        }

        if (!waitForSystemUpdate(systemUpdater, installedRepository, sourceRepository,
                                 &updateSucceeded, &updateApplied, &updateMessage)
            || !updateSucceeded || !updateApplied)
        {
            std::cerr << "System updater failed to apply a fast-forward update: "
                      << updateMessage.toStdString() << '\n';
            return 1;
        }
        QFile updatedBackendFile(installedRepository + QStringLiteral("/app.py"));
        QFile preservedDatabase(installedRepository + QStringLiteral("/instance/database.db"));
        if (!updatedBackendFile.open(QIODevice::ReadOnly) || !preservedDatabase.open(QIODevice::ReadOnly))
        {
            std::cerr << "System updater could not read updated code or preserved database\n";
            return 1;
        }
        QByteArray updatedContent = updatedBackendFile.readAll();
        updatedContent.replace("\r\n", "\n");
        if (updatedContent != "value = 'updated'\n" || preservedDatabase.readAll() != "preserve-this-database")
        {
            std::cerr << "System updater did not update code while preserving the ignored database\n";
            return 1;
        }

        if (!writeFile(sourceRepository + QStringLiteral("/application.db"), "should-not-be-updated")
            || !runCommand(gitExecutable, {QStringLiteral("add"), QStringLiteral("application.db")}, sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("Track database file")},
                           sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("push"), QStringLiteral("origin"), QStringLiteral("main")},
                           sourceRepository, &gitError))
        {
            std::cerr << "Could not prepare protected database-file updater test: " << gitError.toStdString() << '\n';
            return 1;
        }
        if (!waitForSystemUpdate(systemUpdater, installedRepository, sourceRepository,
                                 &updateSucceeded, &updateApplied, &updateMessage)
            || updateSucceeded || updateApplied
            || QFile::exists(installedRepository + QStringLiteral("/application.db")))
        {
            std::cerr << "System updater did not reject an incoming database file\n";
            return 1;
        }

        if (!writeFile(installedRepository + QStringLiteral("/app.py"), "value = 'local edit'\n")
            || !writeFile(sourceRepository + QStringLiteral("/app.py"), "value = 'next update'\n")
            || !runCommand(gitExecutable, {QStringLiteral("add"), QStringLiteral("app.py")}, sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("commit"), QStringLiteral("-m"), QStringLiteral("Next backend update")},
                           sourceRepository, &gitError)
            || !runCommand(gitExecutable, {QStringLiteral("push"), QStringLiteral("origin"), QStringLiteral("main")},
                           sourceRepository, &gitError))
        {
            std::cerr << "Could not prepare dirty-working-tree updater test: " << gitError.toStdString() << '\n';
            return 1;
        }
        if (!waitForSystemUpdate(systemUpdater, installedRepository, sourceRepository,
                                 &updateSucceeded, &updateApplied, &updateMessage)
            || updateSucceeded || updateApplied)
        {
            std::cerr << "System updater did not reject local changes to tracked files\n";
            return 1;
        }
    }

    std::cout << "Launcher configuration and backend process checks passed\n";
    return 0;
}
