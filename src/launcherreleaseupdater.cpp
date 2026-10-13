#include "launcherreleaseupdater.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QUrl>

namespace
{
constexpr qint64 maxReleasePackageBytes = 512LL * 1024 * 1024;

QString updateErrorFilePath()
{
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return appData.isEmpty() ? QString() : QDir(appData).filePath(QStringLiteral("launcher-update-error.txt"));
}

QString updateHelperScript()
{
    return QStringLiteral(R"PS(param(
    [Parameter(Mandatory = $true)][int]$ProcessId,
    [Parameter(Mandatory = $true)][string]$Archive,
    [Parameter(Mandatory = $true)][string]$InstallDirectory,
    [Parameter(Mandatory = $true)][string]$Executable,
    [Parameter(Mandatory = $true)][string]$ErrorFile,
    [switch]$AutoStart
)

$ErrorActionPreference = 'Stop'
$stage = Join-Path $env:TEMP ('BMCMTrayLauncher-update-' + [guid]::NewGuid().ToString('N'))
$backup = Join-Path $stage 'backup'
$payload = Join-Path $stage 'payload'
$applied = [System.Collections.Generic.List[object]]::new()
$launchSucceeded = $false
$installRoot = [System.IO.Path]::GetFullPath($InstallDirectory)
$installRoot = $installRoot.TrimEnd([char[]]@('\', '/')) + [System.IO.Path]::DirectorySeparatorChar

try {
    while (Get-Process -Id $ProcessId -ErrorAction SilentlyContinue) {
        Start-Sleep -Milliseconds 300
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($Archive)
    try {
        $totalUncompressed = [int64]0
        $hasExecutable = $false
        foreach ($entry in $zip.Entries) {
            $relative = $entry.FullName.Replace('/', '\')
            if ([System.IO.Path]::IsPathRooted($relative) -or
                $relative.Split([char]'\') -contains '..' -or
                (($entry.ExternalAttributes -shr 16) -band 0xF000) -eq 0xA000) {
                throw "O pacote contém um caminho inseguro: $($entry.FullName)"
            }
            $totalUncompressed += $entry.Length
            if ($totalUncompressed -gt 1073741824) {
                throw 'O pacote excede o limite de extração de 1 GiB.'
            }
            if ($relative -eq 'BMCMTrayLauncherQt.exe') {
                $hasExecutable = $true
            }
        }
        if (-not $hasExecutable) {
            throw 'O pacote não contém BMCMTrayLauncherQt.exe na raiz.'
        }
    }
    finally {
        $zip.Dispose()
    }

    [System.IO.Compression.ZipFile]::ExtractToDirectory($Archive, $payload)
    $files = @(Get-ChildItem -LiteralPath $payload -File -Recurse)
    if ($files.Count -eq 0) {
        throw 'O pacote de atualização está vazio.'
    }

    foreach ($file in $files) {
        $relative = $file.FullName.Substring($payload.Length).TrimStart([char[]]@('\', '/'))
        $destination = [System.IO.Path]::GetFullPath((Join-Path $InstallDirectory $relative))
        if (-not $destination.StartsWith($installRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "O caminho de destino escapa da instalação: $relative"
        }

        $parent = Split-Path -Parent $destination
        while ($parent.StartsWith($installRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
            if (Test-Path -LiteralPath $parent -PathType Container) {
                $parentItem = Get-Item -LiteralPath $parent -Force
                if ($parentItem.Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
                    throw "O destino contém uma pasta redirecionada: $relative"
                }
            }
            $parent = Split-Path -Parent $parent
        }
        if (Test-Path -LiteralPath $destination -PathType Leaf) {
            $destinationItem = Get-Item -LiteralPath $destination -Force
            if ($destinationItem.Attributes -band [System.IO.FileAttributes]::ReparsePoint) {
                throw "O destino contém um arquivo redirecionado: $relative"
            }
        }

        $hadOriginal = Test-Path -LiteralPath $destination -PathType Leaf
        $backupPath = Join-Path $backup $relative
        if ($hadOriginal) {
            New-Item -ItemType Directory -Path (Split-Path -Parent $backupPath) -Force | Out-Null
            Copy-Item -LiteralPath $destination -Destination $backupPath
        }
        $applied.Add([pscustomobject]@{ Destination = $destination; Backup = $backupPath; HadOriginal = $hadOriginal })
        New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $destination -Force
    }

    Remove-Item -LiteralPath $ErrorFile -Force -ErrorAction SilentlyContinue
    if ($AutoStart) {
        Start-Process -FilePath $Executable -ArgumentList '--autostart' -WorkingDirectory $InstallDirectory
    }
    else {
        Start-Process -FilePath $Executable -WorkingDirectory $InstallDirectory
    }
    $launchSucceeded = $true
}
catch {
    for ($index = $applied.Count - 1; $index -ge 0; $index--) {
        $item = $applied[$index]
        if ($item.HadOriginal -and (Test-Path -LiteralPath $item.Backup -PathType Leaf)) {
            Copy-Item -LiteralPath $item.Backup -Destination $item.Destination -Force
        }
        elseif (-not $item.HadOriginal) {
            Remove-Item -LiteralPath $item.Destination -Force -ErrorAction SilentlyContinue
        }
    }
    New-Item -ItemType Directory -Path (Split-Path -Parent $ErrorFile) -Force | Out-Null
    Set-Content -LiteralPath $ErrorFile -Value $_.Exception.Message -Encoding UTF8
}
finally {
    Remove-Item -LiteralPath $Archive -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $PSCommandPath -Force -ErrorAction SilentlyContinue
}

if (-not $launchSucceeded -and (Test-Path -LiteralPath $Executable -PathType Leaf)) {
    if ($AutoStart) {
        Start-Process -FilePath $Executable -ArgumentList '--autostart' -WorkingDirectory $InstallDirectory
    }
    else {
        Start-Process -FilePath $Executable -WorkingDirectory $InstallDirectory
    }
}
)PS");
}
}

LauncherReleaseUpdater::LauncherReleaseUpdater(QObject *parent)
    : QObject(parent)
{
}

LauncherReleaseUpdater::~LauncherReleaseUpdater() = default;

bool LauncherReleaseUpdater::isNewerReleaseTag(const QString &tag, const QVersionNumber &currentVersion,
                                                QVersionNumber *releaseVersion)
{
    QString versionText = tag.trimmed();
    if (versionText.startsWith(QLatin1Char('v'), Qt::CaseInsensitive))
        versionText.remove(0, 1);

    static const QRegularExpression versionPattern(QStringLiteral("^[0-9]+\\.[0-9]+\\.[0-9]+$"));
    if (!versionPattern.match(versionText).hasMatch())
        return false;

    const QVersionNumber candidate = QVersionNumber::fromString(versionText);
    if (candidate.segmentCount() != 3 || QVersionNumber::compare(candidate, currentVersion) <= 0)
        return false;

    if (releaseVersion)
        *releaseVersion = candidate;
    return true;
}

QString LauncherReleaseUpdater::takePreviousUpdateError()
{
    const QString path = updateErrorFilePath();
    if (path.isEmpty())
        return {};

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    const QString message = QString::fromUtf8(file.readAll()).trimmed();
    file.close();
    QFile::remove(path);
    return message;
}

void LauncherReleaseUpdater::checkForUpdate(const QVersionNumber &currentVersion)
{
#ifndef Q_OS_WIN
    emit statusChanged(tr("A atualização automática por GitHub Release está disponível somente no Windows."));
    emit checkFinished();
    return;
#else
    if (releaseReply_ || downloadReply_)
        return;

    currentVersion_ = currentVersion;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/Grupo-9-PII/bmcmtray/releases/latest")));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader("User-Agent", "BMCMTrayLauncherQt");
    request.setTransferTimeout(15000);
    releaseReply_ = network_.get(request);
    connect(releaseReply_, &QNetworkReply::finished, this, [this]
            {
        QNetworkReply *reply = releaseReply_;
        releaseReply_ = nullptr;
        if (!reply)
            return;

        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray response = reply->readAll();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkMessage = reply->errorString();
        reply->deleteLater();

        if (statusCode == 404 && networkError != QNetworkReply::AuthenticationRequiredError) {
            finishCheck(tr("Nenhuma versão publicada do launcher foi encontrada no GitHub."));
            return;
        }
        if (networkError != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
            emit errorOccurred(tr("Não foi possível consultar as versões do launcher no GitHub: %1").arg(networkMessage));
            finishCheck();
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(response);
        if (!document.isObject()) {
            emit errorOccurred(tr("O GitHub retornou uma resposta inválida sobre a versão do launcher."));
            finishCheck();
            return;
        }

        const QJsonObject release = document.object();
        QVersionNumber remoteVersion;
        if (!isNewerReleaseTag(release.value(QStringLiteral("tag_name")).toString(), currentVersion_, &remoteVersion)) {
            finishCheck(tr("Launcher atualizado: versão %1").arg(currentVersion_.toString()));
            return;
        }

        const QJsonArray assets = release.value(QStringLiteral("assets")).toArray();
        for (const QJsonValue &assetValue : assets) {
            const QJsonObject asset = assetValue.toObject();
            if (asset.value(QStringLiteral("name")).toString() !=
                QStringLiteral("BMCMTrayLauncherQt-windows-x64.zip"))
                continue;

            const QUrl assetUrl(asset.value(QStringLiteral("browser_download_url")).toString());
            if (assetUrl.scheme() != QStringLiteral("https")
                || assetUrl.host().compare(QStringLiteral("github.com"), Qt::CaseInsensitive) != 0) {
                emit errorOccurred(tr("O Release do launcher contém uma URL de pacote inválida."));
                finishCheck();
                return;
            }
            downloadReleaseAsset(assetUrl);
            return;
        }

        emit errorOccurred(tr("A versão %1 foi publicada, mas não contém o pacote Windows esperado.")
                               .arg(remoteVersion.toString()));
        finishCheck(); });
#endif
}

void LauncherReleaseUpdater::downloadReleaseAsset(const QUrl &url)
{
    archiveFile_ = std::make_unique<QTemporaryFile>(
        QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
            .filePath(QStringLiteral("BMCMTrayLauncher-update-XXXXXX.zip")));
    if (!archiveFile_->open()) {
        const QString message = archiveFile_->errorString();
        archiveFile_.reset();
        emit errorOccurred(tr("Não foi possível criar o arquivo temporário da atualização: %1").arg(message));
        finishCheck();
        return;
    }
    archiveFile_->setAutoRemove(false);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "BMCMTrayLauncherQt");
    request.setTransferTimeout(120000);
    downloadReply_ = network_.get(request);
    emit statusChanged(tr("Baixando a atualização do launcher..."));
    connect(downloadReply_, &QIODevice::readyRead, this, [this]
            {
        if (!downloadReply_ || !archiveFile_)
            return;
        const QByteArray chunk = downloadReply_->readAll();
        if (archiveFile_->size() + chunk.size() > maxReleasePackageBytes
            || archiveFile_->write(chunk) != chunk.size()) {
            downloadReply_->abort();
        } });
    connect(downloadReply_, &QNetworkReply::finished, this, [this]
            {
        QNetworkReply *reply = downloadReply_;
        downloadReply_ = nullptr;
        if (!reply)
            return;

        const QByteArray remaining = reply->readAll();
        const bool writeSucceeded = archiveFile_
                                    && archiveFile_->size() + remaining.size() <= maxReleasePackageBytes
                                    && archiveFile_->write(remaining) == remaining.size();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkMessage = reply->errorString();
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300
            || !writeSucceeded || !archiveFile_ || archiveFile_->size() < 4) {
            const QString message = networkError != QNetworkReply::NoError
                                        ? networkMessage
                                        : tr("O pacote recebido está vazio, excede o limite ou não pôde ser gravado.");
            if (archiveFile_)
                QFile::remove(archiveFile_->fileName());
            archiveFile_.reset();
            emit errorOccurred(tr("Falha ao baixar a atualização do launcher: %1").arg(message));
            finishCheck();
            return;
        }

        archiveFile_->flush();
        archiveFile_->close();
        QFile archive(archiveFile_->fileName());
        if (!archive.open(QIODevice::ReadOnly) || !archive.read(2).startsWith("PK")) {
            const QString path = archiveFile_->fileName();
            archive.close();
            archiveFile_.reset();
            QFile::remove(path);
            emit errorOccurred(tr("O arquivo baixado não parece ser um ZIP válido."));
            finishCheck();
            return;
        }
        if (!startApplyHelper()) {
            const QString path = archiveFile_->fileName();
            archiveFile_.reset();
            QFile::remove(path);
            emit errorOccurred(tr("Não foi possível preparar a aplicação segura da atualização."));
            finishCheck();
            return;
        }

        emit statusChanged(tr("Atualização baixada; reiniciando o launcher para aplicá-la..."));
        emit updateReadyToApply(); });
}

void LauncherReleaseUpdater::finishCheck(const QString &status)
{
    if (!status.isEmpty())
        emit statusChanged(status);
    emit checkFinished();
}

bool LauncherReleaseUpdater::startApplyHelper()
{
    const QString powershell = QStandardPaths::findExecutable(QStringLiteral("powershell.exe"));
    const QString errorFile = updateErrorFilePath();
    if (powershell.isEmpty() || errorFile.isEmpty()
        || !QDir().mkpath(QFileInfo(errorFile).absolutePath()))
        return false;

    QTemporaryFile script(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                              .filePath(QStringLiteral("BMCMTrayLauncher-update-XXXXXX.ps1")));
    if (!script.open())
        return false;
    const QByteArray contents = updateHelperScript().toUtf8();
    if (script.write(contents) != contents.size() || !script.flush())
        return false;
    script.close();
    updateScriptPath_ = script.fileName();
    script.setAutoRemove(false);

    const QString executable = QCoreApplication::applicationFilePath();
    const QString installDirectory = QFileInfo(executable).absolutePath();
    QStringList arguments = {
        QStringLiteral("-NoProfile"),
        QStringLiteral("-NonInteractive"),
        QStringLiteral("-WindowStyle"),
        QStringLiteral("Hidden"),
        QStringLiteral("-ExecutionPolicy"),
        QStringLiteral("Bypass"),
        QStringLiteral("-File"),
        updateScriptPath_,
        QStringLiteral("-ProcessId"),
        QString::number(QCoreApplication::applicationPid()),
        QStringLiteral("-Archive"),
        archiveFile_->fileName(),
        QStringLiteral("-InstallDirectory"),
        installDirectory,
        QStringLiteral("-Executable"),
        executable,
        QStringLiteral("-ErrorFile"),
        errorFile};
    if (QCoreApplication::arguments().contains(QStringLiteral("--autostart")))
        arguments.append(QStringLiteral("-AutoStart"));
    if (!QProcess::startDetached(powershell, arguments, QDir::tempPath()))
    {
        QFile::remove(updateScriptPath_);
        updateScriptPath_.clear();
        return false;
    }
    return true;
}
