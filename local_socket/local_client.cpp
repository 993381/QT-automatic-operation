#if 0
#include <QApplication>
#include "client.h"
#include <QTimer>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Client::instance()->ConnectToServer("INJECTOR");

    QTimer::singleShot(1000 * 3, []{
        qInfo() << "xxxxxxxxxxxxxxx";
        std::cerr << "xxxxxxxx error";
    });

    return app.exec();
}
#endif
