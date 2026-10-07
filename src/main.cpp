#include "mainwindow.h"
#include "applicationinstance.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName("BMCM Tray Launcher");
    QCoreApplication::setOrganizationName("BMCM");

    ApplicationInstance instance;
    if (!instance.acquire())
        return 0;

    MainWindow window;
    QObject::connect(&instance, &ApplicationInstance::activationRequested, &window, [&window]
                     {
        window.showNormal();
        window.raise();
        window.activateWindow();
        window.show(); });
    if (application.arguments().contains(QStringLiteral("--autostart")))
        window.startFromAutostart();
    else
        window.show();

    return application.exec();
}
