#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("MIREA AES Visualizer");
    QApplication::setOrganizationName("MIREA");

    MainWindow w;
    w.show();

    return app.exec();
}

