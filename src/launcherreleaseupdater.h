#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QVersionNumber>

#include <memory>

class QTemporaryFile;

class LauncherReleaseUpdater final : public QObject
{
    Q_OBJECT

public:
    explicit LauncherReleaseUpdater(QObject *parent = nullptr);
    ~LauncherReleaseUpdater() override;

    void checkForUpdate(const QVersionNumber &currentVersion);
    static bool isNewerReleaseTag(const QString &tag, const QVersionNumber &currentVersion,
                                  QVersionNumber *releaseVersion = nullptr);
    static QString takePreviousUpdateError();

signals:
    void statusChanged(const QString &status);
    void errorOccurred(const QString &message);
    void checkFinished();
    void updateReadyToApply();

private:
    void downloadReleaseAsset(const QUrl &url);
    void finishCheck(const QString &status = {});
    bool startApplyHelper();

    QNetworkAccessManager network_;
    QNetworkReply *releaseReply_ = nullptr;
    QNetworkReply *downloadReply_ = nullptr;
    std::unique_ptr<QTemporaryFile> archiveFile_;
    QString updateScriptPath_;
    QVersionNumber currentVersion_;
};
