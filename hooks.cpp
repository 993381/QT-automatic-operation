#include "hooks.h"
#include <QCoreApplication>
#include <QtCore/private/qhooks_p.h>
#include "probecreator.h"
#include "probe.h"
#include "uiacontroller.h"
#include "scriptengine/scriptengine.h"
#include "util.h"

#define IF_NONNULL_EXEC(func, ...) { if (func) { func(__VA_ARGS__); } }

using namespace GammaRay;

static void gammaray_pre_routine()
{
    new ProbeCreator(ProbeCreator::Create | ProbeCreator::FindExistingObjects);

    /*
        export LD_PRELOAD=`pwd`/libinjector.so
        export EXEC_JS_SCRIPT_PRE=1
        export SHOW_UIA_WINDOW=1
    */

    if (!qApp) {
        return;
    }
    if (QString(getenv("EXEC_JS_SCRIPT_PRE")) == "1") {
        QTimer::singleShot(1000, []{
            QByteArray testCase;
            if (fileReadWrite(TESTCASE_JS, testCase, true)) {
                auto result = ScriptEngine::instance()->syncRunJavaScript(testCase);
                if (!result.first) {
                    qInfo() << "error when load TESTER_JS";
                }
                ScriptEngine::instance()->syncRunJavaScript("Uia.startTest();");
            }
        });
        if (QString(getenv("SHOW_UIA_WINDOW_PRE")) == "1") {
            UiaController::instance()->createUiaWidget();
            UiaController::instance()->initOperationSequence();

            qInfo() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx singleShot1";
        }
    }
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

    QTimer::singleShot(1000, []{
        if (QString(getenv("EXEC_JS_SCRIPT")) == "1") {
            QByteArray testCase;
            if (fileReadWrite(TESTCASE_JS, testCase, true)) {
                auto result = ScriptEngine::instance()->syncRunJavaScript(testCase);
                if (!result.first) {
                    qInfo() << "error when load TESTER_JS";
                }
                ScriptEngine::instance()->syncRunJavaScript("Uia.startTest();");
            }
        }
        qInfo() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx singleShot2";
    });
}

#undef IF_NONNULL_EXEC

