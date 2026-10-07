#pragma once

#include "launcherconfig.h"

#include <QDialog>

#include <memory>

namespace Ui
{
    class SettingsDialog;
}

class SettingsDialog final : public QDialog
{
public:
    explicit SettingsDialog(const LauncherConfig &config, QWidget *parent = nullptr);
    ~SettingsDialog() override;

    LauncherConfig config() const;

private:
    void browsePythonExecutable();
    void browseWorkingDirectory();
    void validateAndAccept();
    void updateAutoUpdateFields();

    std::unique_ptr<Ui::SettingsDialog> ui_;
};
