#ifndef CLIENT_D_H
#define CLIENT_D_H
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDebug>
#include "daemon_process.h"

namespace DtkUiTest {

class Client {
public:
    Client() {}
    ~Client() {}
    static Client *instance() {
        static Client *client = new Client;
        return client;
    }
    bool ensureDaemon() {
        // 如果服务端没有启动则创建守护进程并启动
        DaemonProcess daemon;
        if (!daemon.demonize()) {
            std::cout << "daemon create failed.";
            return false;
        }
        qDebug() << "daemon create success.";
        return true;
    }
};

}

#endif//CLIENT_D_H
