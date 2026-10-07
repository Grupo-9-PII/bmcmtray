/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QLabel *statusLabel;
    QHBoxLayout *actionsLayout;
    QPushButton *startButton;
    QPushButton *restartButton;
    QPushButton *stopButton;
    QPushButton *settingsButton;
    QPushButton *openSystemButton;
    QTextEdit *logOutput;
    QLabel *versionLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(520, 220);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainLayout = new QVBoxLayout(centralwidget);
        mainLayout->setObjectName("mainLayout");
        titleLabel = new QLabel(centralwidget);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 20px; font-weight: 600;"));

        mainLayout->addWidget(titleLabel);

        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");

        mainLayout->addWidget(statusLabel);

        actionsLayout = new QHBoxLayout();
        actionsLayout->setObjectName("actionsLayout");
        startButton = new QPushButton(centralwidget);
        startButton->setObjectName("startButton");

        actionsLayout->addWidget(startButton);

        restartButton = new QPushButton(centralwidget);
        restartButton->setObjectName("restartButton");

        actionsLayout->addWidget(restartButton);

        stopButton = new QPushButton(centralwidget);
        stopButton->setObjectName("stopButton");

        actionsLayout->addWidget(stopButton);

        settingsButton = new QPushButton(centralwidget);
        settingsButton->setObjectName("settingsButton");

        actionsLayout->addWidget(settingsButton);

        openSystemButton = new QPushButton(centralwidget);
        openSystemButton->setObjectName("openSystemButton");

        actionsLayout->addWidget(openSystemButton);


        mainLayout->addLayout(actionsLayout);

        logOutput = new QTextEdit(centralwidget);
        logOutput->setObjectName("logOutput");
        logOutput->setReadOnly(true);

        mainLayout->addWidget(logOutput);

        versionLabel = new QLabel(centralwidget);
        versionLabel->setObjectName("versionLabel");
        versionLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        mainLayout->addWidget(versionLabel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "BMCM Tray Launcher", nullptr));
        titleLabel->setText(QCoreApplication::translate("MainWindow", "BMCM Tray Launcher", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "Backend parado", nullptr));
        startButton->setText(QCoreApplication::translate("MainWindow", "Iniciar", nullptr));
        restartButton->setText(QCoreApplication::translate("MainWindow", "Reiniciar", nullptr));
        stopButton->setText(QCoreApplication::translate("MainWindow", "Parar", nullptr));
        settingsButton->setText(QCoreApplication::translate("MainWindow", "Configurar", nullptr));
        openSystemButton->setText(QCoreApplication::translate("MainWindow", "Abrir BMCM", nullptr));
        logOutput->setPlaceholderText(QCoreApplication::translate("MainWindow", "Sa\303\255da e erros do backend aparecer\303\243o aqui.", nullptr));
        versionLabel->setText(QCoreApplication::translate("MainWindow", "Vers\303\243o", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
