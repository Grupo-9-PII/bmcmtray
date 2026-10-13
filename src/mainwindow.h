#pragma once

#include "backendcontroller.h"
#include "launcherconfig.h"
#include "launcherreleaseupdater.h"
#include "launcherupdater.h"
#include "readinesschecker.h"
#include "systemrepositoryupdater.h"

#include <QAction>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

#include <memory>

namespace Ui
{
    class MainWindow;
}

class MainWindow final : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void startFromAutostart();

private:
    void startBackend();
    void stopBackend();
    void restartBackend();
    void openSettings();
    void updateProcessActions(bool running);
    void updateTrayStatus(const QString &status);
    void showMainWindow();
    void quitApplication();
    void checkForLauncherUpdates();
    void checkForSystemUpdates();

    std::unique_ptr<Ui::MainWindow> ui_;
    BackendController backendController_;
    LauncherReleaseUpdater releaseUpdater_;
    LauncherUpdater updater_;
    SystemRepositoryUpdater systemUpdater_;
    ReadinessChecker readinessChecker_;
    LauncherConfig config_;
    QString configFilePath_;
    QTimer restartTimer_;
    bool userRequestedStop_ = false;
    bool restartAfterStop_ = false;
    bool quitRequested_ = false;
    bool systemUpdatePending_ = true;
    bool startAfterSystemUpdate_ = false;
    QSystemTrayIcon trayIcon_;
    QMenu trayMenu_;
    QAction *trayStatusAction_ = nullptr;

protected:
    void closeEvent(QCloseEvent *event) override;
};
