#include "backendcontroller.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTimer>

BackendController::BackendController(QObject *parent)
    : QObject(parent)
{
    const QString dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dataDirectory);
    logFilePath_ = dataDirectory + QStringLiteral("/backend-launcher.log");
    stopTimeoutTimer_.setSingleShot(true);

    connect(&process_, &QProcess::started, this, [this]
            {
        stopTimeoutTimer_.stop();
        const qint64 processId = process_.processId();
        emit statusChanged(QStringLiteral("Processo ativo (PID %1)").arg(processId));
        emit processStarted(processId);
        writeLog(QStringLiteral("Backend iniciado com PID %1").arg(processId)); });
    connect(&process_, &QProcess::readyReadStandardOutput, this, [this]
            { appendOutput(process_.readAllStandardOutput(), QStringLiteral("STDOUT")); });
    connect(&process_, &QProcess::readyReadStandardError, this, [this]
            { appendOutput(process_.readAllStandardError(), QStringLiteral("STDERR")); });
    connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
            {
        if (error == QProcess::FailedToStart) {
            const QString message = process_.errorString();
            emit statusChanged(QStringLiteral("Falha ao iniciar backend"));
            emit errorOccurred(message);
            writeLog(QStringLiteral("Falha ao iniciar backend: %1").arg(message));
        } });
    connect(&process_, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus)
            {
        stopTimeoutTimer_.stop();
        const bool crashed = exitStatus == QProcess::CrashExit;
        emit statusChanged(crashed ? QStringLiteral("Backend encerrou inesperadamente")
                                   : QStringLiteral("Backend parado"));
        emit processFinished(exitCode, crashed);
        writeLog(QStringLiteral("Backend encerrado (código %1, falha=%2, solicitado=%3)")
                     .arg(exitCode)
                     .arg(crashed ? QStringLiteral("sim") : QStringLiteral("não"))
                     .arg(stoppingByRequest_ ? QStringLiteral("sim") : QStringLiteral("não")));
        stoppingByRequest_ = false; });
    connect(&stopTimeoutTimer_, &QTimer::timeout, this, [this]
            {
        if (isRunning()) {
            writeLog(QStringLiteral("Backend não encerrou no prazo; forçando encerramento"));
            process_.kill();
        } });
}

BackendController::~BackendController()
{
    if (isRunning())
    {
        process_.terminate();
        if (!process_.waitForFinished(3000))
        {
            process_.kill();
            process_.waitForFinished(1000);
        }
    }
}

bool BackendController::isRunning() const
{
    return process_.state() != QProcess::NotRunning;
}

void BackendController::start(const LauncherConfig &config)
{
    if (isRunning())
    {
        emit statusChanged(QStringLiteral("Backend já está em execução"));
        return;
    }

    if (config.pythonExecutable.trimmed().isEmpty())
    {
        emit errorOccurred(QStringLiteral("Configure o executável Python antes de iniciar o backend."));
        return;
    }

    if (!QDir(config.workingDirectory).exists())
    {
        emit errorOccurred(QStringLiteral("O diretório de trabalho não existe: %1").arg(config.workingDirectory));
        return;
    }

    stoppingByRequest_ = false;
    process_.setProgram(config.pythonExecutable);
    process_.setArguments(QProcess::splitCommand(config.serverArguments));
    process_.setWorkingDirectory(config.workingDirectory);
    process_.setProcessChannelMode(QProcess::SeparateChannels);
    emit statusChanged(QStringLiteral("Iniciando backend..."));
    writeLog(QStringLiteral("Iniciando: %1 %2 (cwd=%3)")
                 .arg(config.pythonExecutable, config.serverArguments, config.workingDirectory));
    process_.start();
}

void BackendController::stop()
{
    if (!isRunning())
        return;

    stoppingByRequest_ = true;
    emit statusChanged(QStringLiteral("Encerrando backend..."));
    process_.terminate();
    stopTimeoutTimer_.start(3000);
}

void BackendController::appendOutput(const QByteArray &data, const QString &streamName)
{
    const QString text = QString::fromLocal8Bit(data);
    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        const QString entry = QStringLiteral("%1: %2").arg(streamName, line.trimmed());
        writeLog(entry);
        emit outputReceived(entry);
    }
}

void BackendController::writeLog(const QString &message) const
{
    QFile file(logFilePath_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    const QByteArray entry = QStringLiteral("%1 %2\n")
                                 .arg(QDateTime::currentDateTime().toString(Qt::ISODate), message)
                                 .toUtf8();
    file.write(entry);
}
