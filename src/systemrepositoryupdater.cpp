#include "systemrepositoryupdater.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUrl>

SystemRepositoryUpdater::SystemRepositoryUpdater(const QString &branch, QObject *parent)
    : QObject(parent),
      branch_(branch)
{
    processTimeout_.setSingleShot(true);
    networkTimeout_.setSingleShot(true);
    connect(&process_, &QProcess::finished, this, &SystemRepositoryUpdater::handleProcessFinished);
    connect(&processTimeout_, &QTimer::timeout, this, [this]
            {
        if (process_.state() != QProcess::NotRunning) {
            timedOut_ = true;
            process_.kill();
        } });
    connect(&process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
            {
        if (error == QProcess::FailedToStart)
        {
            processTimeout_.stop();
            const QString program = step_ == Step::ExtractArchive ? QStringLiteral("tar") : QStringLiteral("Git");
            finish(false, false, tr("Não foi possível executar %1: %2").arg(program, process_.errorString()));
        } });
    connect(&networkTimeout_, &QTimer::timeout, this, [this]
            {
        if (networkReply_)
            networkReply_->abort(); });
    connect(&network_, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply)
            {
        if (reply != networkReply_)
            return;
        handleNetworkReply(); });
}

SystemRepositoryUpdater::~SystemRepositoryUpdater() = default;

bool SystemRepositoryUpdater::isRunning() const
{
    return running_;
}

void SystemRepositoryUpdater::checkAndUpdate(const QString &repositoryDirectory, const QString &repositoryUrl,
                                             const QString &updateMode)
{
    if (running_)
        return;

    repositoryUrl_ = repositoryUrl;
    updateMode_ = updateMode;
    if (repositoryUrl_.isEmpty())
    {
        emit finished(false, false, tr("Configure a URL do repositório do SistemaBMCM."));
        return;
    }

    repositoryDirectory_ = QFileInfo(repositoryDirectory).canonicalFilePath();
    if (repositoryDirectory_.isEmpty() || !QDir(repositoryDirectory_).exists())
    {
        emit finished(false, false, tr("O diretório configurado para o SistemaBMCM não existe."));
        return;
    }

    if (updateMode_ != QStringLiteral("auto") && updateMode_ != QStringLiteral("git")
        && updateMode_ != QStringLiteral("archive"))
    {
        emit finished(false, false, tr("O modo de atualização configurado não é válido."));
        return;
    }

    running_ = true;
    emit statusChanged(tr("Verificando atualizações do SistemaBMCM..."));

    if (updateMode_ == QStringLiteral("archive"))
    {
        startArchiveUpdate();
        return;
    }

    gitExecutable_ = QStandardPaths::findExecutable(QStringLiteral("git"));
    if (gitExecutable_.isEmpty())
    {
        if (updateMode_ == QStringLiteral("auto"))
            startArchiveUpdate();
        else
            finish(false, false, tr("O Git não foi encontrado. Instale o Git ou selecione o modo ZIP."));
        return;
    }

    runProcess(Step::RepositoryRoot, {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")});
}

void SystemRepositoryUpdater::startArchiveUpdate()
{
    step_ = Step::RepositoryRoot;
    const QUrl remoteUrl(repositoryUrl_);
    const QStringList remotePath = remoteUrl.path().split(QLatin1Char('/'), Qt::SkipEmptyParts);
    static const QRegularExpression repositoryComponent(QStringLiteral("^[A-Za-z0-9_.-]+$"));
    if (!remoteUrl.isValid() || remoteUrl.scheme() != QStringLiteral("https")
        || remoteUrl.host().compare(QStringLiteral("github.com"), Qt::CaseInsensitive) != 0
        || remoteUrl.port(-1) != -1 || !remoteUrl.userInfo().isEmpty()
        || !remoteUrl.query().isEmpty() || !remoteUrl.fragment().isEmpty()
        || remotePath.size() != 2 || !repositoryComponent.match(remotePath.value(0)).hasMatch())
    {
        finish(false, false, tr("O modo ZIP requer a URL HTTPS de um repositório no GitHub."));
        return;
    }
    repositoryOwner_ = remotePath.at(0);
    repositoryName_ = remotePath.at(1);
    if (repositoryName_.endsWith(QStringLiteral(".git"), Qt::CaseInsensitive))
        repositoryName_.chop(4);
    if (!repositoryComponent.match(repositoryName_).hasMatch())
    {
        finish(false, false, tr("A URL do repositório do SistemaBMCM não contém um nome válido."));
        return;
    }

    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (appData.isEmpty() || !QDir().mkpath(appData))
    {
        finish(false, false, tr("Não foi possível criar o diretório local para controlar atualizações."));
        return;
    }

    const QByteArray repositoryKey = QCryptographicHash::hash(
        QStringLiteral("%1/%2/%3").arg(repositoryOwner_, repositoryName_, branch_).toUtf8(),
        QCryptographicHash::Sha256).toHex();
    commitMarkerPath_ = QDir(appData).filePath(QStringLiteral("system-update-%1.sha").arg(QString::fromLatin1(repositoryKey)));

    QUrl apiUrl;
    apiUrl.setScheme(QStringLiteral("https"));
    apiUrl.setHost(QStringLiteral("api.github.com"));
    apiUrl.setPath(QStringLiteral("/repos/%1/%2/commits/%3")
                       .arg(repositoryOwner_, repositoryName_, QString::fromLatin1(QUrl::toPercentEncoding(branch_))));
    QNetworkRequest request(apiUrl);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader("User-Agent", "BMCMTrayLauncherQt");
    networkReply_ = network_.get(request);
    networkTimeout_.start(60000);
}

void SystemRepositoryUpdater::handleNetworkReply()
{
    networkTimeout_.stop();
    QNetworkReply *reply = networkReply_;
    networkReply_ = nullptr;
    const QByteArray response = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QString errorString = reply->errorString();
    reply->deleteLater();

    if (networkError != QNetworkReply::NoError)
    {
        finish(false, false, tr("Falha ao consultar/baixar atualizações do GitHub: %1").arg(errorString));
        return;
    }

    if (step_ != Step::ExtractArchive)
    {
        const QJsonDocument document = QJsonDocument::fromJson(response);
        const QString sha = document.object().value(QStringLiteral("sha")).toString();
        static const QRegularExpression validSha(QStringLiteral("^[0-9a-fA-F]{40}$"));
        if (!validSha.match(sha).hasMatch())
        {
            finish(false, false, tr("O GitHub não retornou um identificador válido para a branch %1.").arg(branch_));
            return;
        }
        latestCommit_ = sha.toLower();

        QFile marker(commitMarkerPath_);
        if (marker.open(QIODevice::ReadOnly)
            && QString::fromLatin1(marker.readAll()).trimmed() == latestCommit_)
        {
            finish(true, false, tr("O SistemaBMCM já está atualizado."));
            return;
        }

        archiveDirectory_ = std::make_unique<QTemporaryDir>(QDir::tempPath() + QStringLiteral("/bmcm-update-XXXXXX"));
        if (!archiveDirectory_->isValid())
        {
            finish(false, false, tr("Não foi possível criar uma pasta temporária para a atualização."));
            return;
        }
        archiveFile_ = std::make_unique<QTemporaryFile>(
            QDir(archiveDirectory_->path()).filePath(QStringLiteral("systembmcm-XXXXXX.zip")));
        if (!archiveFile_->open())
        {
            finish(false, false, tr("Não foi possível criar o arquivo temporário da atualização."));
            return;
        }

        QUrl archiveUrl;
        archiveUrl.setScheme(QStringLiteral("https"));
        archiveUrl.setHost(QStringLiteral("codeload.github.com"));
        archiveUrl.setPath(QStringLiteral("/%1/%2/zip/%3").arg(repositoryOwner_, repositoryName_, latestCommit_));
        QNetworkRequest request(archiveUrl);
        request.setRawHeader("User-Agent", "BMCMTrayLauncherQt");
        emit statusChanged(tr("Baixando arquivos atualizados do SistemaBMCM..."));
        networkReply_ = network_.get(request);
        connect(networkReply_, &QIODevice::readyRead, this, [this]
                {
            if (!archiveFile_ || !networkReply_)
                return;
            const QByteArray chunk = networkReply_->readAll();
            if (archiveFile_->write(chunk) != chunk.size()) {
                archiveWriteFailed_ = true;
                networkReply_->abort();
            } });
        archiveWriteFailed_ = false;
        networkTimeout_.start(120000);
        step_ = Step::ExtractArchive;
        return;
    }

    if (archiveWriteFailed_ || !archiveFile_ || archiveFile_->write(response) != response.size()
        || !archiveFile_->flush())
    {
        finish(false, false, tr("Não foi possível salvar o arquivo ZIP da atualização."));
        return;
    }
    archiveFile_->close();

    archiveExtractor_ = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if (archiveExtractor_.isEmpty())
    {
        finish(false, false, tr("O utilitário tar não foi encontrado; não é possível extrair a atualização ZIP."));
        return;
    }
    const QString extractedDirectory = QDir(archiveDirectory_->path()).filePath(QStringLiteral("extracted"));
    if (!QDir().mkpath(extractedDirectory))
    {
        finish(false, false, tr("Não foi possível preparar a extração da atualização."));
        return;
    }
    emit statusChanged(tr("Extraindo atualização do SistemaBMCM..."));
    step_ = Step::ExtractArchive;
    runProcess(Step::ExtractArchive, {
                                           QStringLiteral("--exclude=.env*"),
                                           QStringLiteral("--exclude=*/.env*"),
                                           QStringLiteral("--exclude=.venv"),
                                           QStringLiteral("--exclude=.venv/*"),
                                           QStringLiteral("--exclude=*/.venv"),
                                           QStringLiteral("--exclude=*/.venv/*"),
                                           QStringLiteral("--exclude=venv"),
                                           QStringLiteral("--exclude=venv/*"),
                                           QStringLiteral("--exclude=*/venv"),
                                           QStringLiteral("--exclude=*/venv/*"),
                                           QStringLiteral("--exclude=instance"),
                                           QStringLiteral("--exclude=instance/*"),
                                           QStringLiteral("--exclude=*/instance"),
                                           QStringLiteral("--exclude=*/instance/*"),
                                           QStringLiteral("--exclude=static/uploads"),
                                           QStringLiteral("--exclude=static/uploads/*"),
                                           QStringLiteral("--exclude=*/static/uploads"),
                                           QStringLiteral("--exclude=*/static/uploads/*"),
                                           QStringLiteral("-xf"),
                                           archiveFile_->fileName(),
                                           QStringLiteral("-C"),
                                           extractedDirectory});
}

void SystemRepositoryUpdater::applyArchive()
{
    const QString extractedDirectory = QDir(archiveDirectory_->path()).filePath(QStringLiteral("extracted"));
    const QStringList roots = QDir(extractedDirectory).entryList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    if (roots.size() != 1)
    {
        finish(false, false, tr("O arquivo ZIP do GitHub tem uma estrutura inesperada."));
        return;
    }

    const QString sourceRoot = QDir(extractedDirectory).filePath(roots.constFirst());
    const QString canonicalRoot = QFileInfo(repositoryDirectory_).canonicalFilePath();
    if (canonicalRoot.isEmpty())
    {
        finish(false, false, tr("Não foi possível validar a pasta do SistemaBMCM."));
        return;
    }
    QDirIterator iterator(sourceRoot, QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    int copiedFiles = 0;
    const auto isInsideRepository = [&canonicalRoot](const QString &canonicalPath)
    {
        const QString relativePath =
            QDir::fromNativeSeparators(QDir::cleanPath(QDir(canonicalRoot).relativeFilePath(canonicalPath)));
        return relativePath == QStringLiteral(".")
               || (!QDir::isAbsolutePath(relativePath) && relativePath != QStringLiteral("..")
                   && !relativePath.startsWith(QStringLiteral("../")));
    };
    while (iterator.hasNext())
    {
        const QString sourcePath = iterator.next();
        const QFileInfo sourceInfo = iterator.fileInfo();
        if (sourceInfo.isSymLink())
        {
            finish(false, false, tr("A atualização foi interrompida: o ZIP contém um link simbólico não permitido (%1).")
                                     .arg(sourceInfo.fileName()));
            return;
        }

        const QString relativePath = QDir::cleanPath(QDir(sourceRoot).relativeFilePath(sourcePath));
        if (relativePath == QStringLiteral(".") || relativePath.startsWith(QStringLiteral("../"))
            || QDir::isAbsolutePath(relativePath) || isProtectedDataPath(relativePath.toUtf8())
            || relativePath.startsWith(QStringLiteral(".git/"), Qt::CaseInsensitive))
            continue;

        const QString targetPath = QDir(repositoryDirectory_).filePath(relativePath);
        const QFileInfo targetInfo(targetPath);
        const QString targetDirectory = targetInfo.absolutePath();
        QString existingParent = targetDirectory;
        while (!QFileInfo::exists(existingParent))
        {
            const QString parent = QFileInfo(existingParent).absolutePath();
            if (parent == existingParent)
            {
                finish(false, false, tr("Não foi possível validar a pasta de destino %1.").arg(targetDirectory));
                return;
            }
            existingParent = parent;
        }
        const QString canonicalExistingParent = QFileInfo(existingParent).canonicalFilePath();
        if (canonicalExistingParent.isEmpty() || !isInsideRepository(canonicalExistingParent))
        {
            finish(false, false,
                   tr("A atualização foi interrompida: o caminho %1 aponta para fora da pasta do SistemaBMCM.")
                       .arg(relativePath));
            return;
        }
        if (!QDir().mkpath(targetDirectory))
        {
            finish(false, false, tr("Não foi possível criar a pasta %1 durante a atualização.").arg(targetDirectory));
            return;
        }

        const QString canonicalParent = QFileInfo(targetDirectory).canonicalFilePath();
        if (canonicalParent.isEmpty() || !isInsideRepository(canonicalParent))
        {
            finish(false, false,
                   tr("A atualização foi interrompida: o caminho %1 aponta para fora da pasta do SistemaBMCM.")
                       .arg(relativePath));
            return;
        }

        QFile source(sourcePath);
        QSaveFile target(targetPath);
        if (!source.open(QIODevice::ReadOnly) || !target.open(QIODevice::WriteOnly))
        {
            finish(false, false, tr("Não foi possível atualizar o arquivo %1.").arg(relativePath));
            return;
        }
        QByteArray buffer(1024 * 1024, Qt::Uninitialized);
        while (true)
        {
            const qint64 bytesRead = source.read(buffer.data(), buffer.size());
            if (bytesRead < 0)
            {
                target.cancelWriting();
                finish(false, false, tr("Falha ao ler %1 durante a atualização.").arg(relativePath));
                return;
            }
            if (bytesRead == 0)
                break;
            if (target.write(buffer.constData(), bytesRead) != bytesRead)
            {
                target.cancelWriting();
                finish(false, false, tr("Falha ao gravar %1 durante a atualização.").arg(relativePath));
                return;
            }
        }
        target.setPermissions(sourceInfo.permissions());
        if (!target.commit())
        {
            finish(false, false, tr("Não foi possível finalizar a atualização do arquivo %1.").arg(relativePath));
            return;
        }
        ++copiedFiles;
    }

    if (copiedFiles == 0)
    {
        finish(false, false, tr("O arquivo ZIP não continha arquivos de aplicação atualizáveis."));
        return;
    }

    QSaveFile marker(commitMarkerPath_);
    if (!marker.open(QIODevice::WriteOnly) || marker.write(latestCommit_.toLatin1()) != latestCommit_.size()
        || !marker.commit())
    {
        finish(false, true, tr("Os arquivos foram atualizados, mas não foi possível salvar o identificador da versão."));
        return;
    }
    finish(true, true, tr("SistemaBMCM atualizado (%1 arquivos). Os arquivos locais de dados foram preservados.")
                            .arg(copiedFiles));
}

void SystemRepositoryUpdater::runProcess(Step step, const QStringList &arguments)
{
    step_ = step;
    process_.setProgram(step == Step::ExtractArchive ? archiveExtractor_ : gitExecutable_);
    process_.setArguments(arguments);
    process_.setWorkingDirectory(repositoryDirectory_);
    timedOut_ = false;
    process_.start();
    processTimeout_.start(60000);
}

void SystemRepositoryUpdater::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    processTimeout_.stop();
    const QByteArray output = process_.readAllStandardOutput().trimmed();
    const QByteArray errorOutput = process_.readAllStandardError().trimmed();
    const bool succeeded = exitStatus == QProcess::NormalExit && exitCode == 0;

    if (!succeeded)
    {
        if (timedOut_)
        {
            finish(false, false, tr("A verificação/atualização do SistemaBMCM excedeu o tempo limite."));
            return;
        }
        if (step_ == Step::RepositoryRoot && updateMode_ == QStringLiteral("auto"))
        {
            startArchiveUpdate();
            return;
        }
        const QString detail = QString::fromLocal8Bit(errorOutput);
        const QString operation = step_ == Step::ExtractArchive
                                      ? tr("O tar não conseguiu extrair a atualização")
                                      : tr("O Git não conseguiu concluir a verificação/atualização");
        finish(false, false, detail.isEmpty()
                                 ? operation + QLatin1Char('.')
                                 : tr("%1: %2").arg(operation, detail));
        return;
    }

    switch (step_)
    {
    case Step::RepositoryRoot:
    {
        const QString rootPath = QFileInfo(QString::fromLocal8Bit(output)).canonicalFilePath();
        if (rootPath.isEmpty() || QDir::cleanPath(rootPath) != QDir::cleanPath(repositoryDirectory_))
        {
            if (updateMode_ == QStringLiteral("auto"))
                startArchiveUpdate();
            else
                finish(false, false, tr("O diretório configurado não é a raiz de um clone Git do SistemaBMCM."));
            return;
        }
        runProcess(Step::Branch, {QStringLiteral("branch"), QStringLiteral("--show-current")});
        return;
    }
    case Step::Branch:
        if (QString::fromLocal8Bit(output) != branch_)
        {
            finish(false, false, tr("O clone do SistemaBMCM não está na branch %1.").arg(branch_));
            return;
        }
        runProcess(Step::WorkingTree, {QStringLiteral("status"), QStringLiteral("--porcelain"), QStringLiteral("--untracked-files=no")});
        return;
    case Step::WorkingTree:
        if (!output.isEmpty())
        {
            finish(false, false, tr("A atualização foi cancelada: há alterações locais em arquivos versionados do SistemaBMCM."));
            return;
        }
        runProcess(Step::TrackedFiles, {QStringLiteral("ls-files"), QStringLiteral("-z")});
        return;
    case Step::TrackedFiles:
    {
        for (const QByteArray &path : output.split('\0'))
        {
            if (!path.isEmpty() && isProtectedDataPath(path))
            {
                finish(false, false, tr("A atualização foi bloqueada porque o repositório versiona um arquivo/diretório de dados do BMCM (%1).")
                                         .arg(QString::fromUtf8(path)));
                return;
            }
        }
        runProcess(Step::Fetch, {QStringLiteral("fetch"), QStringLiteral("--no-tags"), repositoryUrl_,
                                 QStringLiteral("%1:refs/remotes/bmcm-update/%1").arg(branch_)});
        return;
    }
    case Step::Fetch:
        runProcess(Step::RevisionCounts,
                   {QStringLiteral("rev-list"), QStringLiteral("--left-right"), QStringLiteral("--count"),
                    QStringLiteral("HEAD...refs/remotes/bmcm-update/%1").arg(branch_)});
        return;
    case Step::RevisionCounts:
    {
        const QList<QByteArray> counts = output.simplified().split(' ');
        if (counts.size() != 2)
        {
            finish(false, false, tr("Não foi possível comparar as versões local e remota do SistemaBMCM."));
            return;
        }
        bool aheadOk = false;
        bool behindOk = false;
        const qulonglong ahead = counts.at(0).toULongLong(&aheadOk);
        const qulonglong behind = counts.at(1).toULongLong(&behindOk);
        if (!aheadOk || !behindOk)
        {
            finish(false, false, tr("O Git retornou uma comparação inválida das versões."));
            return;
        }
        if (ahead > 0)
        {
            finish(false, false, tr("A atualização foi cancelada: o clone local possui commits que não estão no repositório configurado."));
            return;
        }
        if (behind == 0)
        {
            finish(true, false, tr("O SistemaBMCM já está atualizado."));
            return;
        }
        runProcess(Step::IncomingFiles,
                   {QStringLiteral("diff"), QStringLiteral("--name-only"), QStringLiteral("-z"),
                    QStringLiteral("HEAD"), QStringLiteral("refs/remotes/bmcm-update/%1").arg(branch_)});
        return;
    }
    case Step::IncomingFiles:
        for (const QByteArray &path : output.split('\0'))
        {
            if (!path.isEmpty() && isProtectedDataPath(path))
            {
                finish(false, false, tr("A atualização foi bloqueada porque a nova versão altera um arquivo/diretório de dados do BMCM (%1).")
                                         .arg(QString::fromUtf8(path)));
                return;
            }
        }
        emit statusChanged(tr("Atualizando arquivos do SistemaBMCM..."));
        runProcess(Step::Merge, {QStringLiteral("merge"), QStringLiteral("--ff-only"),
                                 QStringLiteral("refs/remotes/bmcm-update/%1").arg(branch_)});
        return;
    case Step::Merge:
        finish(true, true, tr("SistemaBMCM atualizado com sucesso. Os arquivos de dados locais foram preservados."));
        return;
    case Step::ExtractArchive:
        applyArchive();
        return;
    }
}

void SystemRepositoryUpdater::finish(bool success, bool updated, const QString &message)
{
    processTimeout_.stop();
    if (!running_)
    {
        emit finished(success, updated, message);
        return;
    }
    running_ = false;
    archiveFile_.reset();
    archiveDirectory_.reset();
    emit finished(success, updated, message);
}

bool SystemRepositoryUpdater::isProtectedDataPath(const QByteArray &path)
{
    QString normalizedPath = QDir::fromNativeSeparators(QString::fromUtf8(path)).toLower();
    if (normalizedPath == QStringLiteral(".env")
    || normalizedPath.startsWith(QStringLiteral(".env."))
        || normalizedPath.startsWith(QStringLiteral(".env/"))
        || normalizedPath == QStringLiteral("instance")
        || normalizedPath.startsWith(QStringLiteral("instance/"))
        || normalizedPath == QStringLiteral("venv")
        || normalizedPath.startsWith(QStringLiteral("venv/"))
        || normalizedPath == QStringLiteral(".venv")
        || normalizedPath.startsWith(QStringLiteral(".venv/"))
        || normalizedPath == QStringLiteral("static/uploads")
        || normalizedPath.startsWith(QStringLiteral("static/uploads/")))
        return true;

    static const QStringList protectedSuffixes = {
        QStringLiteral(".db"), QStringLiteral(".sqlite"), QStringLiteral(".sqlite3"),
        QStringLiteral(".db-wal"), QStringLiteral(".db-shm"), QStringLiteral(".db-journal"),
        QStringLiteral(".sqlite-wal"), QStringLiteral(".sqlite-shm"), QStringLiteral(".sqlite-journal")};
    for (const QString &suffix : protectedSuffixes)
    {
        if (normalizedPath.endsWith(suffix))
            return true;
    }
    return false;
}
