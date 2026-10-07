#include "applicationinstance.h"

#include <QDir>
#include <QCryptographicHash>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QThread>

ApplicationInstance::ApplicationInstance(QObject *parent)
    : QObject(parent),
      serverName_(QStringLiteral("BMCMTrayLauncherQt-%1").arg(QString::fromLatin1(QCryptographicHash::hash(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toUtf8(), QCryptographicHash::Sha256).toHex().left(16)))),
      lockFile_(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/instance.lock"))
{
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    lockFile_.setStaleLockTime(30000);
    connect(&server_, &QLocalServer::newConnection, this, [this]
            {
        while (QLocalSocket *socket = server_.nextPendingConnection()) {
            connect(socket, &QLocalSocket::readyRead, socket, [this, socket] {
                const QByteArray command = socket->readAll().trimmed();
                if (command == QByteArrayLiteral("activate"))
                    emit activationRequested();
                socket->disconnectFromServer();
            });
            connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
        } });
}

bool ApplicationInstance::acquire()
{
    if (!lockFile_.tryLock(0))
    {
        for (int attempt = 0; attempt < 10; ++attempt)
        {
            QLocalSocket socket;
            socket.connectToServer(serverName_);
            if (socket.waitForConnected(100))
            {
                socket.write("activate\n");
                socket.waitForBytesWritten(500);
                socket.disconnectFromServer();
                return false;
            }
            QThread::msleep(50);
        }
        return false;
    }

    QLocalServer::removeServer(serverName_);
    return server_.listen(serverName_);
}
