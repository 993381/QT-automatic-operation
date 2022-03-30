#include "hooks.h"
#include <QCoreApplication>
#include <QtCore/private/qhooks_p.h>
#include "probecreator.h"
#include "probe.h"
#include "uiacontroller.h"
#include "scriptengine/scriptengine.h"
#include "scriptengine/js_cpp_interface.h"
#include "util.h"

#include <unistd.h>
// #include "local_socket/local_client.h"
#include "websocket/echoclient.h"
#include "daemon/client_d.h"
// #include "cs/server.h"
// #include "dbus_register.h"


#define IF_NONNULL_EXEC(func, ...) { if (func) { func(__VA_ARGS__); } }

using namespace GammaRay;

static void gammaray_pre_routine()
{
    new ProbeCreator(ProbeCreator::Create | ProbeCreator::FindExistingObjects);

    /*
        export LD_PRELOAD=`pwd`/libinjector.so
        export EXEC_JS_SCRIPT_PRE=1
        export SHOW_UIA_WINDOW_PRE=1
    */

    if (!qApp) {
        return;
    }


    QTimer::singleShot(100, []{
        // 如果执行的是execv，要把窗口显示出来
        if (QString(qgetenv("DEBUG_APPS_PRE")) == "1") {
            for (auto w : qApp->topLevelWidgets()) {
                if (qobject_cast<QMainWindow*>(w)) {
                    w->show();
                }
            }
        }

        if (QString(getenv("SHOW_UIA_WINDOW_PRE")) == "1") {
            UiaController::instance()->createUiaWidget();
            UiaController::instance()->initOperationSequence();
        }
        if (QString(getenv("EXEC_JS_SCRIPT_PRE")) == "1") {
            // QByteArray testCase;
            // if (fileReadWrite(TESTCASE_JS, testCase, true)) {
            //     auto result = ScriptEngine::instance()->runScript(testCase);
            //     if (!result.first) {
            //         qInfo() << "error when load TESTER_JS";
            //     }
            //     ScriptEngine::instance()->syncRunJavaScript("TestMethod.startTest();");
            // }
        }
        // DbusRegister::instance()->create();

        // DtkUiTest::Client::instance()->ensureDaemon();

        //! TODO: 未启动服务端则弹窗警告
        static QScopedPointer <EchoClient> client(new EchoClient(QUrl(QStringLiteral("ws://localhost:45535")), {QString("appinfo:%1:%2").arg(std::to_string(getpid()).c_str()).arg(qAppName())}, true));
        // 应该用函数通知每一步的,用返回值判断总体的
        QObject::connect(ScriptEngine::instance()->interface(), &JsCppInterface::execFinished, client.data(), [&](QJSValue result){
            if(result.toBool()) {
                client->sendTextMessage("Exec-c-success");
            } else {
                client->sendTextMessage("Exec-c-failed");
            }
        }, Qt::ConnectionType::UniqueConnection);

        client->handleMessage([](QString msg){
            qInfo() << "----------- " << msg;
            // if (msg == "loginOn") {
            //     client->sendTextMessage("isOnline?dde-control-center");
            // }

            QStringList res = msg.split(":");

            if (msg.startsWith("Exec-script:")) {
                // QByteArray userCode;
                // if (fileReadWrite(res.at(1), userCode, true)) {
                //     // 分两步，获取错误码、获取执行结果
                //     auto result = ScriptEngine::instance()->runScript(userCode);
                //     if (result.toBool()) {
                //         client->sendTextMessage("Exec-s-success");
                //     } else {
                //         //! client->sendTextMessage("Exec-s-failed: " + result.second.toString().toLocal8Bit());
                //     }
                // } else {
                //     client->sendTextMessage("Exec-file--read-error");
                // }
            }
            if (msg.startsWith("Exec-function:")) {
                QString userCode = res.at(1);
                QJSValue result;
                qInfo() << "runScript start ...............";
                if (userCode.startsWith("test")) {
                    if (userCode == "test1") {
                        ScriptEngine::instance()->execTest({"删除帐户"});
                    } else if (userCode == "test2") {
                        ScriptEngine::instance()->execTest({"byAccName", "删除"});
                    }
                    qInfo() << "sendTextMessage start ...............";
                    client->sendTextMessage(QString("Exec-all-finished-success"));
                    qApp->processEvents();
                    qInfo() << "sendTextMessage end ...............";
                    return;
                } else {
                    result = ScriptEngine::instance()->runScript(userCode);
                }

                qInfo() << "runScript end .................";
                if (result.isError()) {
                    client->sendTextMessage(QString("Exec-all-finished-failed: ") + result.property("name").toString() + " " + result.property("message").toString());
                } else {
                    if (result.toBool()) {
                        client->sendTextMessage(QString("Exec-all-finished-success"));
                    } else {
                        client->sendTextMessage(QString("Exec-all-finished-failed: false"));
                    }
                }
            }
        });

        // LocalClient::instance()->setAppInfo({QString(std::to_string(getppid()).c_str()), qAppName()});
        // LocalClient::instance()->ConnectToServer("INJECTOR");
        // LocalClient::instance()->sendMessage(QString(std::to_string(getppid()).c_str()) + QString(" ") + qAppName());
    });
}
Q_COREAPP_STARTUP_FUNCTION(gammaray_pre_routine)

static void (*gammaray_next_startup_hook)() = nullptr;
static void (*gammaray_next_addObject)(QObject *) = nullptr;
static void (*gammaray_next_removeObject)(QObject *) = nullptr;

extern "C" Q_DECL_EXPORT void gammaray_startup_hook()
{
    puts("gammaray_startup_hook");
    Probe::startupHookReceived();
    new ProbeCreator(GammaRay::ProbeCreator::Create);

    IF_NONNULL_EXEC(gammaray_next_startup_hook);
}

extern "C" Q_DECL_EXPORT void gammaray_addObject(QObject *obj)
{
    Probe::objectAdded(obj, true);

    IF_NONNULL_EXEC(gammaray_next_addObject, obj);
}

extern "C" Q_DECL_EXPORT void gammaray_removeObject(QObject *obj)
{
    Probe::objectRemoved(obj);

    IF_NONNULL_EXEC(gammaray_next_removeObject, obj);
}

void installQHooks()
{
    Q_ASSERT(qtHookData[QHooks::HookDataVersion] >= 1);
    Q_ASSERT(qtHookData[QHooks::HookDataSize] >= 6);

    gammaray_next_addObject    = reinterpret_cast<QHooks::AddQObjectCallback>(qtHookData[QHooks::AddQObject]);
    gammaray_next_removeObject = reinterpret_cast<QHooks::RemoveQObjectCallback>(qtHookData[QHooks::RemoveQObject]);
    gammaray_next_startup_hook = reinterpret_cast<QHooks::StartupCallback>(qtHookData[QHooks::Startup]);

    qtHookData[QHooks::AddQObject] = reinterpret_cast<quintptr>(&gammaray_addObject);
    qtHookData[QHooks::RemoveQObject] = reinterpret_cast<quintptr>(&gammaray_removeObject);
    qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&gammaray_startup_hook);
}

bool Hooks::hooksInstalled()
{
    return qtHookData[QHooks::AddQObject] == reinterpret_cast<quintptr>(&gammaray_addObject);
}

void Hooks::installHooks()
{
    if (hooksInstalled())
        return;
    installQHooks();
}

extern "C" Q_DECL_EXPORT void gammaray_probe_attach()
{
    if (!qApp) {
        return;
    }
    new ProbeCreator(ProbeCreator::Create |
                     ProbeCreator::FindExistingObjects |
                     ProbeCreator::ResendServerAddress);

    // 可以在这里实现注入后执行。但是有socket了就不用了
}

#undef IF_NONNULL_EXEC

