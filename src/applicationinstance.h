#pragma once

#include <QObject>
#include <QLocalServer>
#include <QLockFile>

class ApplicationInstance final : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationInstance(QObject *parent = nullptr);
    bool acquire();

signals:
    void activationRequested();

private:
    QString serverName_;
    QLockFile lockFile_;
    QLocalServer server_;
};
