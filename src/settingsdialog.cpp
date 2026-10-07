#include "settingsdialog.h"

#include "ui_settingsdialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

SettingsDialog::SettingsDialog(const LauncherConfig &config, QWidget *parent)
    : QDialog(parent),
      ui_(std::make_unique<Ui::SettingsDialog>())
{
    ui_->setupUi(this);
    ui_->pythonExecutableEdit->setText(config.pythonExecutable);
    ui_->workingDirectoryEdit->setText(config.workingDirectory);
    ui_->serverArgumentsEdit->setText(config.serverArguments);
    ui_->applicationUrlEdit->setText(config.applicationUrl);
    ui_->healthCheckUrlEdit->setText(config.healthCheckUrl);
    ui_->restartOnFailureCheckBox->setChecked(config.restartOnFailure);
    ui_->restartDelaySpinBox->setValue(config.restartDelaySeconds);
    ui_->autoOpenBrowserCheckBox->setChecked(config.autoOpenBrowser);
    ui_->autoStartWithSystemCheckBox->setChecked(config.autoStartWithSystem);
    ui_->enableAutoUpdateCheckBox->setChecked(config.enableAutoUpdateCheck);
    ui_->updateVersionUrlEdit->setText(config.updateVersionUrl);
    ui_->updateInstallerUrlEdit->setText(config.updateInstallerUrl);

    connect(ui_->browsePythonButton, &QPushButton::clicked, this, &SettingsDialog::browsePythonExecutable);
    connect(ui_->browseWorkingDirectoryButton, &QPushButton::clicked, this, &SettingsDialog::browseWorkingDirectory);
    connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::validateAndAccept);
    connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui_->enableAutoUpdateCheckBox, &QCheckBox::toggled, this, &SettingsDialog::updateAutoUpdateFields);
    updateAutoUpdateFields();
}

SettingsDialog::~SettingsDialog() = default;

LauncherConfig SettingsDialog::config() const
{
    LauncherConfig result;
    result.pythonExecutable = ui_->pythonExecutableEdit->text().trimmed();
    result.workingDirectory = ui_->workingDirectoryEdit->text().trimmed();
    result.serverArguments = ui_->serverArgumentsEdit->text().trimmed();
    result.applicationUrl = ui_->applicationUrlEdit->text().trimmed();
    result.healthCheckUrl = ui_->healthCheckUrlEdit->text().trimmed();
    result.restartOnFailure = ui_->restartOnFailureCheckBox->isChecked();
    result.restartDelaySeconds = ui_->restartDelaySpinBox->value();
    result.autoOpenBrowser = ui_->autoOpenBrowserCheckBox->isChecked();
    result.autoStartWithSystem = ui_->autoStartWithSystemCheckBox->isChecked();
    result.enableAutoUpdateCheck = ui_->enableAutoUpdateCheckBox->isChecked();
    result.updateVersionUrl = ui_->updateVersionUrlEdit->text().trimmed();
    result.updateInstallerUrl = ui_->updateInstallerUrlEdit->text().trimmed();
    return result;
}

void SettingsDialog::browsePythonExecutable()
{
#ifdef Q_OS_WIN
    const QString filter = tr("Executável Python (*.exe);;Todos os arquivos (*)");
#else
    const QString filter = tr("Todos os arquivos (*)");
#endif
    const QString path = QFileDialog::getOpenFileName(this, tr("Selecionar Python"), ui_->pythonExecutableEdit->text(), filter);
    if (!path.isEmpty())
        ui_->pythonExecutableEdit->setText(path);
}

void SettingsDialog::browseWorkingDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Selecionar diretório do BMCM"), ui_->workingDirectoryEdit->text());
    if (!path.isEmpty())
        ui_->workingDirectoryEdit->setText(path);
}

void SettingsDialog::validateAndAccept()
{
    const LauncherConfig values = config();
    if (values.pythonExecutable.isEmpty() || values.workingDirectory.isEmpty() || values.serverArguments.isEmpty())
    {
        QMessageBox::warning(this, tr("Configuração incompleta"), tr("Informe o Python, o diretório de trabalho e os argumentos do servidor."));
        return;
    }

    const QUrl url(values.applicationUrl);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https")) || url.host().isEmpty())
    {
        QMessageBox::warning(this, tr("URL inválida"), tr("Informe uma URL HTTP ou HTTPS válida para o sistema BMCM."));
        return;
    }

    if (!values.healthCheckUrl.isEmpty())
    {
        const QUrl healthUrl(values.healthCheckUrl);
        if (!healthUrl.isValid() || (healthUrl.scheme() != QStringLiteral("http") && healthUrl.scheme() != QStringLiteral("https")) || healthUrl.host().isEmpty())
        {
            QMessageBox::warning(this, tr("URL de saúde inválida"), tr("Informe uma URL HTTP/HTTPS válida ou deixe o campo vazio."));
            return;
        }
    }

    if (values.enableAutoUpdateCheck)
    {
        const QUrl versionUrl(values.updateVersionUrl);
        const QUrl installerUrl(values.updateInstallerUrl);
        if (versionUrl.scheme() != QStringLiteral("https") || versionUrl.host().isEmpty() || installerUrl.scheme() != QStringLiteral("https") || installerUrl.host().isEmpty())
        {
            QMessageBox::warning(this, tr("URLs de atualização inválidas"), tr("Informe URLs HTTPS válidas para a versão e o instalador."));
            return;
        }
    }

    accept();
}

void SettingsDialog::updateAutoUpdateFields()
{
    const bool enabled = ui_->enableAutoUpdateCheckBox->isChecked();
    ui_->updateVersionUrlEdit->setEnabled(enabled);
    ui_->updateInstallerUrlEdit->setEnabled(enabled);
}
