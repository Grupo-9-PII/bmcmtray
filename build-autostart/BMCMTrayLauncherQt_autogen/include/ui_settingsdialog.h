/********************************************************************************
** Form generated from reading UI file 'settingsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *dialogLayout;
    QLabel *titleLabel;
    QFormLayout *settingsFormLayout;
    QLabel *pythonExecutableLabel;
    QHBoxLayout *pythonPathLayout;
    QLineEdit *pythonExecutableEdit;
    QPushButton *browsePythonButton;
    QLabel *workingDirectoryLabel;
    QHBoxLayout *workingDirectoryLayout;
    QLineEdit *workingDirectoryEdit;
    QPushButton *browseWorkingDirectoryButton;
    QLabel *serverArgumentsLabel;
    QLineEdit *serverArgumentsEdit;
    QLabel *applicationUrlLabel;
    QLineEdit *applicationUrlEdit;
    QLabel *healthCheckUrlLabel;
    QLineEdit *healthCheckUrlEdit;
    QCheckBox *restartOnFailureCheckBox;
    QHBoxLayout *restartDelayLayout;
    QLabel *restartDelayLabel;
    QSpinBox *restartDelaySpinBox;
    QSpacerItem *restartDelaySpacer;
    QCheckBox *autoOpenBrowserCheckBox;
    QCheckBox *autoStartWithSystemCheckBox;
    QCheckBox *enableAutoUpdateCheckBox;
    QFormLayout *updateUrlLayout;
    QLabel *updateVersionUrlLabel;
    QLineEdit *updateVersionUrlEdit;
    QLabel *updateInstallerUrlLabel;
    QLineEdit *updateInstallerUrlEdit;
    QSpacerItem *bottomSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName("SettingsDialog");
        SettingsDialog->setMinimumSize(QSize(620, 520));
        dialogLayout = new QVBoxLayout(SettingsDialog);
        dialogLayout->setObjectName("dialogLayout");
        titleLabel = new QLabel(SettingsDialog);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 18px; font-weight: 600;"));

        dialogLayout->addWidget(titleLabel);

        settingsFormLayout = new QFormLayout();
        settingsFormLayout->setObjectName("settingsFormLayout");
        pythonExecutableLabel = new QLabel(SettingsDialog);
        pythonExecutableLabel->setObjectName("pythonExecutableLabel");

        settingsFormLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, pythonExecutableLabel);

        pythonPathLayout = new QHBoxLayout();
        pythonPathLayout->setObjectName("pythonPathLayout");
        pythonExecutableEdit = new QLineEdit(SettingsDialog);
        pythonExecutableEdit->setObjectName("pythonExecutableEdit");

        pythonPathLayout->addWidget(pythonExecutableEdit);

        browsePythonButton = new QPushButton(SettingsDialog);
        browsePythonButton->setObjectName("browsePythonButton");

        pythonPathLayout->addWidget(browsePythonButton);


        settingsFormLayout->setLayout(0, QFormLayout::ItemRole::FieldRole, pythonPathLayout);

        workingDirectoryLabel = new QLabel(SettingsDialog);
        workingDirectoryLabel->setObjectName("workingDirectoryLabel");

        settingsFormLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, workingDirectoryLabel);

        workingDirectoryLayout = new QHBoxLayout();
        workingDirectoryLayout->setObjectName("workingDirectoryLayout");
        workingDirectoryEdit = new QLineEdit(SettingsDialog);
        workingDirectoryEdit->setObjectName("workingDirectoryEdit");

        workingDirectoryLayout->addWidget(workingDirectoryEdit);

        browseWorkingDirectoryButton = new QPushButton(SettingsDialog);
        browseWorkingDirectoryButton->setObjectName("browseWorkingDirectoryButton");

        workingDirectoryLayout->addWidget(browseWorkingDirectoryButton);


        settingsFormLayout->setLayout(1, QFormLayout::ItemRole::FieldRole, workingDirectoryLayout);

        serverArgumentsLabel = new QLabel(SettingsDialog);
        serverArgumentsLabel->setObjectName("serverArgumentsLabel");

        settingsFormLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, serverArgumentsLabel);

        serverArgumentsEdit = new QLineEdit(SettingsDialog);
        serverArgumentsEdit->setObjectName("serverArgumentsEdit");

        settingsFormLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, serverArgumentsEdit);

        applicationUrlLabel = new QLabel(SettingsDialog);
        applicationUrlLabel->setObjectName("applicationUrlLabel");

        settingsFormLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, applicationUrlLabel);

        applicationUrlEdit = new QLineEdit(SettingsDialog);
        applicationUrlEdit->setObjectName("applicationUrlEdit");

        settingsFormLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, applicationUrlEdit);

        healthCheckUrlLabel = new QLabel(SettingsDialog);
        healthCheckUrlLabel->setObjectName("healthCheckUrlLabel");

        settingsFormLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, healthCheckUrlLabel);

        healthCheckUrlEdit = new QLineEdit(SettingsDialog);
        healthCheckUrlEdit->setObjectName("healthCheckUrlEdit");

        settingsFormLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, healthCheckUrlEdit);


        dialogLayout->addLayout(settingsFormLayout);

        restartOnFailureCheckBox = new QCheckBox(SettingsDialog);
        restartOnFailureCheckBox->setObjectName("restartOnFailureCheckBox");

        dialogLayout->addWidget(restartOnFailureCheckBox);

        restartDelayLayout = new QHBoxLayout();
        restartDelayLayout->setObjectName("restartDelayLayout");
        restartDelayLabel = new QLabel(SettingsDialog);
        restartDelayLabel->setObjectName("restartDelayLabel");

        restartDelayLayout->addWidget(restartDelayLabel);

        restartDelaySpinBox = new QSpinBox(SettingsDialog);
        restartDelaySpinBox->setObjectName("restartDelaySpinBox");
        restartDelaySpinBox->setMaximum(3600);

        restartDelayLayout->addWidget(restartDelaySpinBox);

        restartDelaySpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        restartDelayLayout->addItem(restartDelaySpacer);


        dialogLayout->addLayout(restartDelayLayout);

        autoOpenBrowserCheckBox = new QCheckBox(SettingsDialog);
        autoOpenBrowserCheckBox->setObjectName("autoOpenBrowserCheckBox");

        dialogLayout->addWidget(autoOpenBrowserCheckBox);

        autoStartWithSystemCheckBox = new QCheckBox(SettingsDialog);
        autoStartWithSystemCheckBox->setObjectName("autoStartWithSystemCheckBox");

        dialogLayout->addWidget(autoStartWithSystemCheckBox);

        enableAutoUpdateCheckBox = new QCheckBox(SettingsDialog);
        enableAutoUpdateCheckBox->setObjectName("enableAutoUpdateCheckBox");

        dialogLayout->addWidget(enableAutoUpdateCheckBox);

        updateUrlLayout = new QFormLayout();
        updateUrlLayout->setObjectName("updateUrlLayout");
        updateVersionUrlLabel = new QLabel(SettingsDialog);
        updateVersionUrlLabel->setObjectName("updateVersionUrlLabel");

        updateUrlLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, updateVersionUrlLabel);

        updateVersionUrlEdit = new QLineEdit(SettingsDialog);
        updateVersionUrlEdit->setObjectName("updateVersionUrlEdit");

        updateUrlLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, updateVersionUrlEdit);

        updateInstallerUrlLabel = new QLabel(SettingsDialog);
        updateInstallerUrlLabel->setObjectName("updateInstallerUrlLabel");

        updateUrlLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, updateInstallerUrlLabel);

        updateInstallerUrlEdit = new QLineEdit(SettingsDialog);
        updateInstallerUrlEdit->setObjectName("updateInstallerUrlEdit");

        updateUrlLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, updateInstallerUrlEdit);


        dialogLayout->addLayout(updateUrlLayout);

        bottomSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        dialogLayout->addItem(bottomSpacer);

        buttonBox = new QDialogButtonBox(SettingsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Save);

        dialogLayout->addWidget(buttonBox);


        retranslateUi(SettingsDialog);

        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "Configura\303\247\303\265es do BMCM Tray Launcher", nullptr));
        titleLabel->setText(QCoreApplication::translate("SettingsDialog", "Configura\303\247\303\243o do ambiente BMCM", nullptr));
        pythonExecutableLabel->setText(QCoreApplication::translate("SettingsDialog", "Execut\303\241vel Python", nullptr));
        browsePythonButton->setText(QCoreApplication::translate("SettingsDialog", "Procurar...", nullptr));
        workingDirectoryLabel->setText(QCoreApplication::translate("SettingsDialog", "Diret\303\263rio de trabalho", nullptr));
        browseWorkingDirectoryButton->setText(QCoreApplication::translate("SettingsDialog", "Procurar...", nullptr));
        serverArgumentsLabel->setText(QCoreApplication::translate("SettingsDialog", "Argumentos do servidor", nullptr));
        applicationUrlLabel->setText(QCoreApplication::translate("SettingsDialog", "URL do sistema", nullptr));
        healthCheckUrlLabel->setText(QCoreApplication::translate("SettingsDialog", "URL de verifica\303\247\303\243o de sa\303\272de (opcional)", nullptr));
        healthCheckUrlEdit->setPlaceholderText(QCoreApplication::translate("SettingsDialog", "Ex.: http://127.0.0.1:8080/health", nullptr));
        restartOnFailureCheckBox->setText(QCoreApplication::translate("SettingsDialog", "Reiniciar o servidor automaticamente em caso de falha", nullptr));
        restartDelayLabel->setText(QCoreApplication::translate("SettingsDialog", "Atraso antes de reiniciar (segundos)", nullptr));
        autoOpenBrowserCheckBox->setText(QCoreApplication::translate("SettingsDialog", "Abrir o sistema no navegador ao iniciar o backend", nullptr));
        autoStartWithSystemCheckBox->setText(QCoreApplication::translate("SettingsDialog", "Iniciar o launcher automaticamente com o sistema", nullptr));
        enableAutoUpdateCheckBox->setText(QCoreApplication::translate("SettingsDialog", "Verificar atualiza\303\247\303\265es do launcher ao iniciar", nullptr));
        updateVersionUrlLabel->setText(QCoreApplication::translate("SettingsDialog", "URL HTTPS da vers\303\243o", nullptr));
        updateInstallerUrlLabel->setText(QCoreApplication::translate("SettingsDialog", "URL HTTPS do instalador", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
