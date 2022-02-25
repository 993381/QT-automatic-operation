#include "hooks.h"
#include <QCoreApplication>
#include <QtCore/private/qhooks_p.h>
#include "probecreator.h"
#include "probe.h"

#define IF_NONNULL_EXEC(func, ...) { if (func) { func(__VA_ARGS__); } }

using namespace GammaRay;

static void gammaray_pre_routine()
{
    new ProbeCreator(ProbeCreator::Create | ProbeCreator::FindExistingObjects);
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
}

#undef IF_NONNULL_EXEC

