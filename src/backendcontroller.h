#pragma once

#include "launcherconfig.h"

#include <QObject>
#include <QProcess>
#include <QTimer>

class BackendController final : public QObject
{
    Q_OBJECT

public:
    explicit BackendController(QObject *parent = nullptr);
    ~BackendController() override;

    bool isRunning() const;
    void start(const LauncherConfig &config);
    void stop();

signals:
    void statusChanged(const QString &status);
    void processStarted(qint64 processId);
    void processFinished(int exitCode, bool crashed);
    void outputReceived(const QString &line);
    void errorOccurred(const QString &message);

private:
    void appendOutput(const QByteArray &data, const QString &streamName);
    void writeLog(const QString &message) const;

    QProcess process_;
    QTimer stopTimeoutTimer_;
    QString logFilePath_;
    bool stoppingByRequest_ = false;
};
