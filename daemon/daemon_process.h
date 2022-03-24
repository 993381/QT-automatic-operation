#ifndef CREATE_DAEMON_H
#define CREATE_DAEMON_H
#include <QCoreApplication>
#include "../QSingleInstance/qsingleinstance.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <syslog.h>

#include <QScopedPointer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>

// #include "../local_socket/local_server.h"
#include "../websocket/echoserver.h"

#define RUNNING_DIR /tmp
#define DTK_UI_TST "com.Dtk.UiTest"
#define DTK_UI_OBJECT "/com/DtkUiTest/Server"

class DaemonProcess : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.Interface.DtkTst")
public:
    DaemonProcess() {}
    ~DaemonProcess() {}
    void wsExit(std::string cause, int code) {
        std::cerr << cause << std::endl;
        exit(code);
    }
    bool demonize() {
        pid_t pid;
        if (getppid() == 1) {
            return false;
        }

        pid = fork();
        std::cout << pid << std::endl;

        if (pid < 0) {
            wsExit("fork error.", 1);
        }

        if (pid > 0) {
            return true;
        }

        int i;
        setsid();

        if(chdir("/") < 0) {
            wsExit("change dir error.", 1);
        }

        umask(0);

        // for (i=0; i < NOFILE; i++) {
        //     close(i);
        // }
        // i = open("/dev/null", O_RDWR);
        // dup(i);
        // dup(i);

        QSingleInstance::setUniqueString("dtk_auto_test_app-daemon");
        QSingleInstance instance;
        instance.setGlobal(true);
        instance.setStartupFunction([&instance]() {
            if (instance.isMaster()) {
                // DtkUiTest::LocalServer::instance()->listen("INJECTOR");
                static QScopedPointer<EchoServer> server1(new EchoServer(45535, false));
                static QScopedPointer<EchoServer> server2(new EchoServer(455356, false));
            }

            qDebug() << "Instance running! Start another with -quit as first argument to exit";
            return 0;
        });
        QObject::connect(&instance, &QSingleInstance::instanceMessage, [&](QStringList args){
            qDebug() << "New Instance:" << args;
            if(args.size() > 1 && args[1] == "-quit") {
                exit(0);
            }
        });

         int ret = instance.singleExec() == 0;
         exit(ret);
    }

//    bool registerDBus() {
//        // 建立到session bus的连接
//        QDBusConnection connection = QDBusConnection::sessionBus();

//        // 在session bus上注册名为com.Server.Server1的服务
//        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(DTK_UI_TST)) {
//            return false;
//        }
//        if (!connection.registerService(DTK_UI_TST))
//        {
//            qDebug() << "error1: " << connection.lastError().message();
//            return false;
//        }
//        // 注册名为/com/ObjectPath/CTestDbus的对象，把类Object所有槽函数和信号导出为object的method
//        if (!connection.registerObject(DTK_UI_OBJECT, this,
//              QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))
//        {
//            qDebug() << "error:" << connection.lastError().message();
//            return false;
//        }
//        return true;
//    }
};

#endif//CREATE_DAEMON_H
