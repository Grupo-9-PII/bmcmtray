#include "mainwindow.h"

#include "app_version.h"
#include "autostartmanager.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>
#include <QStyle>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(std::make_unique<Ui::MainWindow>())
{
    ui_->setupUi(this);
    ui_->versionLabel->setText(QStringLiteral("Versão %1").arg(QString::fromLatin1(APP_VERSION_STRING)));

    configFilePath_ = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QStringLiteral("/appsettings.json");
    QString configError;
    config_ = LauncherConfig::load(configFilePath_, &configError);
    QString autostartError;
    config_.autoStartWithSystem = AutostartManager::isEnabled(&autostartError);
    if (!autostartError.isEmpty())
        ui_->logOutput->append(QStringLiteral("Aviso de autostart: %1").arg(autostartError));
    if (!configError.isEmpty())
    {
        ui_->logOutput->append(QStringLiteral("Falha ao carregar configuração: %1").arg(configError));
        ui_->statusLabel->setText(QStringLiteral("Configuração inválida; usando valores padrão"));
    }
    ui_->statusbar->showMessage(QStringLiteral("Configuração: %1").arg(configFilePath_));

    restartTimer_.setSingleShot(true);
    connect(ui_->startButton, &QPushButton::clicked, this, &MainWindow::startBackend);
    connect(ui_->stopButton, &QPushButton::clicked, this, &MainWindow::stopBackend);
    connect(ui_->restartButton, &QPushButton::clicked, this, &MainWindow::restartBackend);
    connect(ui_->settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(ui_->openSystemButton, &QPushButton::clicked, this, [this]
            {
        if (!QDesktopServices::openUrl(QUrl(config_.applicationUrl)))
            QMessageBox::warning(this, tr("Não foi possível abrir o BMCM"), tr("Verifique a URL configurada para o sistema.")); });

    connect(&backendController_, &BackendController::statusChanged, ui_->statusLabel, &QLabel::setText);
    connect(&backendController_, &BackendController::statusChanged, this, &MainWindow::updateTrayStatus);
    connect(&backendController_, &BackendController::outputReceived, ui_->logOutput, &QTextEdit::append);
    connect(&backendController_, &BackendController::errorOccurred, this, [this](const QString &message)
            {
        updateProcessActions(false);
        QMessageBox::critical(this, tr("Falha no backend"), message); });
    connect(&backendController_, &BackendController::processStarted, this, [this](qint64)
            {
        updateProcessActions(true);
        if (!config_.healthCheckUrl.isEmpty()) {
            readinessChecker_.start(QUrl(config_.healthCheckUrl));
        } else {
            const QString status = tr("Backend ativo; prontidão HTTP não verificada");
            ui_->statusLabel->setText(status);
            updateTrayStatus(status);
        }
        if (config_.autoOpenBrowser && config_.healthCheckUrl.isEmpty())
            QDesktopServices::openUrl(QUrl(config_.applicationUrl)); });
    connect(&backendController_, &BackendController::processFinished, this, [this](int exitCode, bool crashed)
            {
        readinessChecker_.stop();
        updateProcessActions(false);
        if (restartAfterStop_) {
            restartAfterStop_ = false;
            startBackend();
            return;
        }

        if (userRequestedStop_ || !config_.restartOnFailure)
            return;

        ui_->statusLabel->setText(QStringLiteral("Backend encerrou (código %1); reinício programado").arg(exitCode));
        ui_->logOutput->append(crashed ? tr("O backend encerrou inesperadamente.") : tr("O backend encerrou."));
        restartTimer_.start(config_.restartDelaySeconds * 1000); });
    connect(&restartTimer_, &QTimer::timeout, this, &MainWindow::startBackend);

    connect(&updater_, &LauncherUpdater::statusChanged, this, [this](const QString &message)
            { ui_->statusbar->showMessage(message, 10000); });
    connect(&updater_, &LauncherUpdater::errorOccurred, this, [this](const QString &message)
            {
        ui_->logOutput->append(message);
        QMessageBox::warning(this, tr("Atualização do launcher"), message); });
    connect(&updater_, &LauncherUpdater::updateAvailable, this,
            [this](const QVersionNumber &remoteVersion, const QString &installerUrl)
            {
                const auto result = QMessageBox::question(
                    this, tr("Atualização disponível"),
                    tr("A versão %1 está disponível (versão atual: %2).\nDeseja baixar o instalador de %3?")
                        .arg(remoteVersion.toString(), QString::fromLatin1(APP_VERSION_STRING), installerUrl),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
                if (result == QMessageBox::Yes)
                    updater_.downloadInstaller(installerUrl);
            });
    connect(&updater_, &LauncherUpdater::installerDownloaded, this, [this](const QString &filePath)
            {
        const auto result = QMessageBox::question(
            this, tr("Instalador baixado"),
            tr("O instalador foi salvo em:\n%1\n\nDeseja abri-lo agora?").arg(filePath),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (result == QMessageBox::Yes) {
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
                QMessageBox::warning(this, tr("Não foi possível abrir"), tr("Abra manualmente o instalador baixado."));
                QFile::remove(filePath);
            }
        } else {
            QFile::remove(filePath);
        } });

    connect(&readinessChecker_, &ReadinessChecker::checking, this, [this](const QString &message)
            {
        ui_->statusLabel->setText(message);
        updateTrayStatus(message); });
    connect(&readinessChecker_, &ReadinessChecker::ready, this, [this](const QUrl &url)
            {
        const QString status = tr("BMCM pronto: %1").arg(url.toString());
        ui_->statusLabel->setText(status);
        ui_->logOutput->append(status);
        updateTrayStatus(tr("BMCM pronto"));
        if (config_.autoOpenBrowser)
            QDesktopServices::openUrl(QUrl(config_.applicationUrl)); });
    connect(&readinessChecker_, &ReadinessChecker::timedOut, this, [this](const QUrl &url)
            {
        const QString status = tr("Processo ativo, mas o BMCM não respondeu em %1").arg(url.toString());
        ui_->statusLabel->setText(status);
        ui_->logOutput->append(status);
        updateTrayStatus(tr("Aguardando BMCM")); });

    trayMenu_.setTitle(tr("BMCM Tray Launcher"));
    trayStatusAction_ = trayMenu_.addAction(tr("Backend parado"));
    trayStatusAction_->setEnabled(false);
    trayMenu_.addSeparator();
    QAction *showAction = trayMenu_.addAction(tr("Abrir launcher"));
    QAction *startAction = trayMenu_.addAction(tr("Iniciar servidor"));
    QAction *restartAction = trayMenu_.addAction(tr("Reiniciar servidor"));
    QAction *stopAction = trayMenu_.addAction(tr("Parar servidor"));
    QAction *openAction = trayMenu_.addAction(tr("Abrir BMCM no navegador"));
    QAction *settingsAction = trayMenu_.addAction(tr("Configurar"));
    trayMenu_.addSeparator();
    QAction *quitAction = trayMenu_.addAction(tr("Sair"));
    connect(showAction, &QAction::triggered, this, &MainWindow::showMainWindow);
    connect(startAction, &QAction::triggered, this, &MainWindow::startBackend);
    connect(restartAction, &QAction::triggered, this, &MainWindow::restartBackend);
    connect(stopAction, &QAction::triggered, this, &MainWindow::stopBackend);
    connect(openAction, &QAction::triggered, ui_->openSystemButton, &QPushButton::click);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettings);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quitApplication);
    startAction->setObjectName(QStringLiteral("trayStartAction"));
    restartAction->setObjectName(QStringLiteral("trayRestartAction"));
    stopAction->setObjectName(QStringLiteral("trayStopAction"));
    trayIcon_.setContextMenu(&trayMenu_);
    trayIcon_.setToolTip(tr("BMCM Tray Launcher %1").arg(QString::fromLatin1(APP_VERSION_STRING)));
    trayIcon_.setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    connect(&trayIcon_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
            {
        if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger)
            showMainWindow(); });
    if (QSystemTrayIcon::isSystemTrayAvailable())
        trayIcon_.show();
    else
        ui_->logOutput->append(tr("Bandeja do sistema indisponível; o launcher permanecerá na janela."));

    updateProcessActions(false);
    QTimer::singleShot(0, this, &MainWindow::checkForLauncherUpdates);
}

MainWindow::~MainWindow() = default;

void MainWindow::startFromAutostart()
{
    if (trayIcon_.isVisible())
        hide();
    else
        showNormal();

    QTimer::singleShot(0, this, &MainWindow::startBackend);
}

void MainWindow::startBackend()
{
    restartTimer_.stop();
    userRequestedStop_ = false;
    updateProcessActions(false);
    backendController_.start(config_);
}

void MainWindow::stopBackend()
{
    restartTimer_.stop();
    restartAfterStop_ = false;
    userRequestedStop_ = true;
    backendController_.stop();
    if (!backendController_.isRunning())
        ui_->statusLabel->setText(tr("Backend parado"));
    updateProcessActions(backendController_.isRunning());
}

void MainWindow::restartBackend()
{
    restartTimer_.stop();
    userRequestedStop_ = true;
    if (backendController_.isRunning())
    {
        restartAfterStop_ = true;
        backendController_.stop();
    }
    else
    {
        restartAfterStop_ = false;
        startBackend();
    }
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(config_, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const LauncherConfig updatedConfig = dialog.config();
    const bool previousAutostart = AutostartManager::isEnabled();
    const bool autostartNeedsUpdate = previousAutostart != updatedConfig.autoStartWithSystem
                                      || updatedConfig.autoStartWithSystem;
    QString autostartError;
    if (autostartNeedsUpdate && !AutostartManager::setEnabled(updatedConfig.autoStartWithSystem, &autostartError))
    {
        QMessageBox::warning(this, tr("Falha no autostart"), autostartError);
        return;
    }

    QString errorMessage;
    if (!updatedConfig.save(configFilePath_, &errorMessage))
    {
        if (previousAutostart != updatedConfig.autoStartWithSystem)
            AutostartManager::setEnabled(previousAutostart);
        QMessageBox::critical(this, tr("Falha ao salvar"), errorMessage);
        return;
    }

    const bool restart = backendController_.isRunning();
    config_ = updatedConfig;
    ui_->statusbar->showMessage(tr("Configuração salva: %1").arg(configFilePath_), 5000);
    if (restart)
        restartBackend();
}

void MainWindow::checkForLauncherUpdates()
{
    if (!config_.enableAutoUpdateCheck)
        return;

    const QVersionNumber currentVersion(APP_VERSION_MAJOR, APP_VERSION_UPDATE, APP_VERSION_COUNT);
    updater_.checkForUpdate(config_.updateVersionUrl, config_.updateInstallerUrl, currentVersion);
}

void MainWindow::updateProcessActions(bool running)
{
    ui_->startButton->setEnabled(!running);
    ui_->restartButton->setEnabled(running);
    ui_->stopButton->setEnabled(running);
    ui_->openSystemButton->setEnabled(!config_.applicationUrl.isEmpty());
    if (QAction *startAction = trayMenu_.findChild<QAction *>(QStringLiteral("trayStartAction")))
        startAction->setEnabled(!running);
    if (QAction *restartAction = trayMenu_.findChild<QAction *>(QStringLiteral("trayRestartAction")))
        restartAction->setEnabled(running);
    if (QAction *stopAction = trayMenu_.findChild<QAction *>(QStringLiteral("trayStopAction")))
        stopAction->setEnabled(running);
}

void MainWindow::updateTrayStatus(const QString &status)
{
    if (trayStatusAction_)
        trayStatusAction_->setText(status);
    trayIcon_.setToolTip(QStringLiteral("BMCM Tray Launcher: %1").arg(status).left(127));
}

void MainWindow::showMainWindow()
{
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::quitApplication()
{
    quitRequested_ = true;
    stopBackend();
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon_.isVisible() && !quitRequested_)
    {
        hide();
        trayIcon_.showMessage(tr("BMCM Tray Launcher"), tr("O launcher continua ativo na bandeja do sistema."));
        event->ignore();
        return;
    }

    readinessChecker_.stop();
    restartTimer_.stop();
    if (backendController_.isRunning())
    {
        backendController_.stop();
        backendController_.disconnect(this);
    }
    trayIcon_.hide();
    event->accept();
}
