#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("3D Physics Engine - Mass-Spring Cube/Cloth");
    window.resize(1000, 800);
    window.show();

    return app.exec();
}
