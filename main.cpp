#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    if(styleFile.open(QFile::ReadOnly | QFile::Text)){
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    MainWindow window;
    window.setWindowTitle("3D Physics Engine - Mass-Spring Cube/Cloth");
    window.resize(1000, 800);
    window.show();

    return app.exec();
}
