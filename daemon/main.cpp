#include "client_d.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QTimer>
// #include "../local_socket/local_client.h"
#include "../websocket/echoclient.h"
#include <dlfcn.h>
#include "../gdbinjector/gdb_injector.h"

// 示例程序
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    // 守护进程的启动、重启、状态
    QCommandLineOption daemon_opt({"d", "daemon"}, "Start daemons only.");
    // 显示程序状态，在线程序、守护进程状态
    QCommandLineOption info_opt({"i", "info"}, "Show applications infomation.");
    // 选中一个在线程序进行操作
    QCommandLineOption select_opt({"s", "select"}, "Select an application.");
    // 启动待执行的程序
    QCommandLineOption launch_opt({"l", "launch"}, "Launch application.", "/path/to/app");
    // 启动待执行的程序
    QCommandLineOption inject_opt({"j", "inject"}, "Inject into application.", "app pid", "0");
    // 执行单句的脚本命令
    QCommandLineOption exec_opt({"c", "code"}, "Execute script command.", "js script code");
    // 执行指定的脚本文件
    QCommandLineOption script_opt({"f", "file"}, "Execute script file.", "/path/to/javascript");
    // QCommandLineOption app_opt({"a", "app"}, "Name of the app you want to launch.");
    parser.setApplicationDescription("DTK automatic test software.");
    parser.addHelpOption();
    parser.addVersionOption();
    //    parser.addOption(daemon_opt);
    parser.addOption(launch_opt);
    parser.addOption(inject_opt);
    //    parser.addOption(arg_opt);
    parser.addOption(exec_opt);
    parser.addOption(script_opt);
    // parser.addOption(app_opt);
    parser.addPositionalArgument("args", "The args pass to process.");
    parser.process(app);
    // qDebug() << "app: " << parser.value("app");
    // qDebug() << parser.helpText();

    // daemon -l dde-control-center  -- -s
    // positionalArguments:  ("-s")，用两个 -- 分隔，后面就全是 positionalArguments
    const QStringList args = parser.positionalArguments();
    qInfo() << "positionalArguments: " << args;

    DtkUiTest::Client::instance()->ensureDaemon();

    QScopedPointer<EchoClient> client(new EchoClient(QUrl(QStringLiteral("ws://localhost:45536")), {"dtk-ui-test-client"}, true));
    client->handleMessage([&client, &parser, args](QString msg){
        qInfo() << "----------- " << msg;
        if (msg == "loginOn") {
            Q_EMIT client->loginSuccess();

            if (parser.isSet("l")) {
                // 启动应用，可能带有参数
                const QString &appName = parser.value("l");
                QString param = QString("launch:%1").arg(appName);
                qInfo() << "value: " << appName;
                if (args.size()) {
                    for (auto arg : args) {
                        param += QString(":%1").arg(arg);
                    }
                }
                client->sendTextMessage(param);
                return;  // -f 不支持和其它参数组合，最多再带个-l
                // client->sendTextMessage("isOnline?dde-control-center");
            }
            if (parser.isSet("f")) {
                client->sendTextMessage(QString("execute-script:%1").arg(parser.value("f")));
            } else if (parser.isSet("c")) {
                client->sendTextMessage(QString("execute-function:%1").arg(parser.value("c")));
            }
        }
        // handle reply
        // 注意这里不能用qApp->exit，只能退出嵌套，还会往下执行
        if (msg.startsWith("launch success")) {
            exit(0);
        }
        if (msg.startsWith("launch failed")) {
            qInfo() << "failed: " << msg;
            exit(1);
        }
        if (msg == "Already-Online") {
            qInfo() << "failed: " << msg;
            exit(0);
        }
        if (msg.startsWith("App-not-online")) {
            qInfo() << msg;
            exit(1);
        }

        // 处理单句或多句命令的执行结果，也需要根据总的执行结果退出程序
        if (msg.startsWith("Exec-c-failed:")) {
            qInfo() << "Exec-failed:" << msg;
            exit(1);        // 如果有一句失败了那么全都失败了
        }
        if (msg.startsWith("Exec-c-success")) {
            qInfo() << "Exec-success";
            // exit(0);
        }

        // 处理脚本文件执行结果，脚本都是异步执行的，只有总的执行结果获取到才算完成
        if (msg.startsWith("Exec-s-failed:")) {
            qInfo() << "Exec-failed:" << msg;
            exit(1);        // 脚本解析都失败了那么全都失败了
        }
        if (msg.startsWith("Exec-s-success")) {
            qInfo() << "Exec-success " << msg;
            // exit(0);
        }
        if (msg.startsWith("Exec-file--read-error")) {
            qInfo() << "Exec-file--read-error";
            exit(1);        // 脚本读取都失败了那么全都失败了
        }

        // 获取总的执行结果
        if (msg.startsWith("Exec-all-finished-success")) {
            qInfo() << "Exec-all-finished-success";
            exit(0);
        }
        if (msg.startsWith("Exec-all-finished-failed")) {
            qInfo() << "Exec-all-finished-failed";
            exit(1);
        }

        // 分两个阶段，注入前查询是否在线，在线就不用注入了，注入后查询是否注入成功
        if (msg.startsWith("Online-status-1")) {
            QStringList res = msg.split(":");
            if (res.at(1).toInt()) {
                qInfo() << "已注入";
                exit(0);
            } else {
                qInfo() << "未注入，即将注入";
                // 两秒内收到上面的消息的回复，后面的就不执行了
                const QString &appPid = parser.value("j");
                QString param = QString("inject:%1").arg(appPid);
                qInfo() << "value: " << appPid;
                QObject::connect(GdbInjector::instance(), &GdbInjector::injectFinished, [&]{
                    qInfo() << "gdb injectFinished!";
                    client->sendTextMessage(QString("isOnline-pid-2:%1").arg(parser.value("j")));
                });
                QObject::connect(GdbInjector::instance(), &GdbInjector::gdbStarted, []{
                    // bool status = process->waitForStarted(-1);
                    qInfo() << "gdb inject start!";
                });
                GdbInjector::instance()->attachInject(appPid.toInt());   // QProcess 还未处理
            }
        }
        if (msg.startsWith("Online-status-2")) {
            QStringList res = msg.split(":");
            if (res.at(1).toInt()) {
                qInfo() << "注入成功";
                exit(0);
            } else {
                qInfo() << "注入失败";
                exit(1);
            }
        }
    });

    QObject::connect(client.data(), &EchoClient::loginSuccess, [&]{
        if (parser.isSet("j")) {
            // 根据 PID 注入应用。怎么判断是否注入成功？
            // injectFinished 后查询一下服务端的在线状态。
            client->sendTextMessage(QString("isOnline-pid-1:%1").arg(parser.value("j")));
        }
    });


    // return instance.singleExec();
    // 加入 quit 的功能
    // appIsOnline/selectApp/getTestResult

    return app.exec();
}
