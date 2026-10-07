#include "launcherupdater.h"

#include <QDir>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUrl>

LauncherUpdater::LauncherUpdater(QObject *parent)
    : QObject(parent)
{
}

LauncherUpdater::~LauncherUpdater() = default;

bool LauncherUpdater::isNewerVersion(const QString &remoteVersion, const QVersionNumber &currentVersion,
                                     QVersionNumber *parsedVersion)
{
    const QString text = remoteVersion.trimmed();
    static const QRegularExpression versionPattern(QStringLiteral("^[0-9]+\\.[0-9]+\\.[0-9]+$"));
    if (!versionPattern.match(text).hasMatch())
        return false;

    const QVersionNumber candidate = QVersionNumber::fromString(text);
    if (candidate.segmentCount() != 3)
        return false;

    if (parsedVersion)
        *parsedVersion = candidate;
    return QVersionNumber::compare(candidate, currentVersion) > 0;
}

void LauncherUpdater::checkForUpdate(const QString &versionUrl, const QString &installerUrl,
                                     const QVersionNumber &currentVersion)
{
    if (versionReply_)
        return;

    const QUrl url(versionUrl);
    if (url.scheme() != QStringLiteral("https") || url.host().isEmpty())
    {
        emit errorOccurred(tr("A URL de consulta de versão deve usar HTTPS."));
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(10000);
    versionReply_ = network_.get(request);
    connect(versionReply_, &QNetworkReply::finished, this, [this, installerUrl, currentVersion]
            {
        QNetworkReply *reply = versionReply_;
        versionReply_ = nullptr;
        if (!reply)
            return;

        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString response = QString::fromUtf8(reply->readAll()).trimmed();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkMessage = reply->errorString();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
            emit errorOccurred(tr("Não foi possível consultar atualizações: %1").arg(networkMessage));
            return;
        }

        QVersionNumber remoteVersion;
        if (!isNewerVersion(response, currentVersion, &remoteVersion)) {
            static const QRegularExpression versionPattern(QStringLiteral("^[0-9]+\\.[0-9]+\\.[0-9]+$"));
            if (!versionPattern.match(response).hasMatch()) {
                emit errorOccurred(tr("A versão remota não está no formato principal.atualizacao.contagem."));
            } else {
                emit statusChanged(tr("Launcher atualizado: versão %1").arg(currentVersion.toString()));
            }
            return;
        }

        const QUrl downloadUrl(installerUrl);
        if (downloadUrl.scheme() != QStringLiteral("https") || downloadUrl.host().isEmpty()) {
            emit errorOccurred(tr("A URL do instalador deve usar HTTPS."));
            return;
        }
        emit updateAvailable(remoteVersion, installerUrl); });
}

void LauncherUpdater::downloadInstaller(const QString &installerUrl)
{
    if (downloadReply_)
        return;

    const QUrl url(installerUrl);
    if (url.scheme() != QStringLiteral("https") || url.host().isEmpty())
    {
        emit errorOccurred(tr("A URL do instalador deve usar HTTPS."));
        return;
    }

    QString suffix = QFileInfo(url.path()).suffix();
#ifdef Q_OS_WIN
    if (suffix.compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0)
    {
        emit errorOccurred(tr("No Windows, a atualização deve apontar para um instalador .exe."));
        return;
    }
#else
    if (suffix.isEmpty())
        suffix = QStringLiteral("download");
#endif

    downloadFile_ = std::make_unique<QTemporaryFile>(
        QDir::tempPath() + QStringLiteral("/BMCMTrayLauncher-Update-XXXXXX.") + suffix);
    downloadFile_->setAutoRemove(true);
    if (!downloadFile_->open())
    {
        const QString message = downloadFile_->errorString();
        downloadFile_.reset();
        emit errorOccurred(tr("Não foi possível criar o arquivo temporário: %1").arg(message));
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    downloadReply_ = network_.get(request);
    emit statusChanged(tr("Baixando atualização..."));

    connect(downloadReply_, &QIODevice::readyRead, this, [this]
            {
        if (!downloadReply_ || !downloadFile_)
            return;
        const QByteArray chunk = downloadReply_->readAll();
        if (downloadFile_->write(chunk) != chunk.size()) {
            const QString message = downloadFile_->errorString();
            downloadReply_->abort();
            emit errorOccurred(tr("Falha ao gravar o instalador: %1").arg(message));
        } });
    connect(downloadReply_, &QNetworkReply::finished, this, [this]
            {
        QNetworkReply *reply = downloadReply_;
        downloadReply_ = nullptr;
        if (!reply)
            return;

        const QByteArray remaining = reply->readAll();
        bool writeSucceeded = downloadFile_ && downloadFile_->write(remaining) == remaining.size();
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkMessage = reply->errorString();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300 || !writeSucceeded) {
            const QString message = networkError != QNetworkReply::NoError
                                        ? networkMessage
                                        : (downloadFile_ ? downloadFile_->errorString() : tr("Resposta HTTP inválida."));
            downloadFile_.reset();
            emit errorOccurred(tr("Falha ao baixar o instalador: %1").arg(message));
            return;
        }

        const QString filePath = downloadFile_->fileName();
        downloadFile_->flush();
        downloadFile_->close();
        downloadFile_->setAutoRemove(false);
        downloadFile_.reset();
        emit statusChanged(tr("Instalador baixado."));
        emit installerDownloaded(filePath); });
}
