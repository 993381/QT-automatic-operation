#if 0

#include <QApplication>

#include "server.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Server server;

    server.listen("INJECTOR");

    return app.exec();
}
#endif
