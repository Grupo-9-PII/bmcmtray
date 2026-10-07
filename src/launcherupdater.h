#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVersionNumber>

#include <memory>

class QTemporaryFile;

class LauncherUpdater final : public QObject
{
    Q_OBJECT

public:
    explicit LauncherUpdater(QObject *parent = nullptr);
    ~LauncherUpdater() override;

    void checkForUpdate(const QString &versionUrl, const QString &installerUrl,
                        const QVersionNumber &currentVersion);
    void downloadInstaller(const QString &installerUrl);
    static bool isNewerVersion(const QString &remoteVersion, const QVersionNumber &currentVersion,
                               QVersionNumber *parsedVersion = nullptr);

signals:
    void updateAvailable(const QVersionNumber &version, const QString &installerUrl);
    void statusChanged(const QString &status);
    void errorOccurred(const QString &message);
    void installerDownloaded(const QString &filePath);

private:
    QNetworkAccessManager network_;
    QNetworkReply *versionReply_ = nullptr;
    QNetworkReply *downloadReply_ = nullptr;
    std::unique_ptr<QTemporaryFile> downloadFile_;
};
