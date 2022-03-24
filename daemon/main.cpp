#include "client_d.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QTimer>
// #include "../local_socket/local_client.h"
#include "../websocket/echoclient.h"

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
    // 执行单句的脚本命令
    QCommandLineOption exec_opt({"e", "exec"}, "Execute script command.");
    // 执行指定的脚本文件
    QCommandLineOption script_opt({"f", "file"}, "Execute script file.");
    // QCommandLineOption app_opt({"a", "app"}, "Name of the app you want to launch.");
    parser.setApplicationDescription("DTK automatic test software.");
    parser.addHelpOption();
    parser.addVersionOption();
    //    parser.addOption(daemon_opt);
    parser.addOption(launch_opt);
    //    parser.addOption(arg_opt);
    //    parser.addOption(exec_opt);
    //    parser.addOption(script_opt);
    // parser.addOption(app_opt);
    //    parser.addPositionalArgument("script", "The js script file to execute.");
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
                // client->sendTextMessage("isOnline?dde-control-center");
            }
        }
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
    });

    // return instance.singleExec();
    // 加入 quit 的功能
    // appIsOnline/selectApp/getTestResult

    return app.exec();
}
