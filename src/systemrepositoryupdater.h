#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QTimer>

#include <memory>

class QTemporaryDir;
class QTemporaryFile;

class SystemRepositoryUpdater final : public QObject
{
    Q_OBJECT

public:
    explicit SystemRepositoryUpdater(const QString &branch = QStringLiteral("main"), QObject *parent = nullptr);
    ~SystemRepositoryUpdater() override;

    bool isRunning() const;
    void checkAndUpdate(const QString &repositoryDirectory, const QString &repositoryUrl,
                        const QString &updateMode = QStringLiteral("auto"));

signals:
    void statusChanged(const QString &status);
    void finished(bool success, bool updated, const QString &message);

private:
    enum class Step
    {
        RepositoryRoot,
        Branch,
        WorkingTree,
        TrackedFiles,
        Fetch,
        RevisionCounts,
        IncomingFiles,
        Merge,
        ExtractArchive
    };

    void runProcess(Step step, const QStringList &arguments);
    void startArchiveUpdate();
    void handleNetworkReply();
    void applyArchive();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void finish(bool success, bool updated, const QString &message);
    static bool isProtectedDataPath(const QByteArray &path);

    QString repositoryUrl_;
    QString branch_;
    QString repositoryDirectory_;
    QString gitExecutable_;
    QString archiveExtractor_;
    Step step_ = Step::RepositoryRoot;
    QProcess process_;
    QTimer processTimeout_;
    QTimer networkTimeout_;
    QNetworkAccessManager network_;
    QNetworkReply *networkReply_ = nullptr;
    std::unique_ptr<QTemporaryDir> archiveDirectory_;
    std::unique_ptr<QTemporaryFile> archiveFile_;
    QString updateMode_ = QStringLiteral("auto");
    QString repositoryOwner_;
    QString repositoryName_;
    QString latestCommit_;
    QString commitMarkerPath_;
    bool running_ = false;
    bool timedOut_ = false;
    bool archiveWriteFailed_ = false;
};
