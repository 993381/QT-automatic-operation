#if 0
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QObject obj;
    obj.setObjectName(QStringLiteral("myTestObject"));

    return app.exec();
}
#endif

#if 1
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QtCore/private/qobject_p.h>
#include <QtCore/private/qmetaobject_p.h>
#include <QVector>
#include <QMutexLocker>
#include "signalspycallbackset.h"
#include <QWindow>
#include <QtCore/private/qhooks_p.h>

using namespace GammaRay;

static void signal_begin_callback(QObject *caller, int method_index, void **argv);
static void signal_end_callback(QObject *caller, int method_index);
static void slot_begin_callback(QObject *caller, int method_index, void **argv);
static void slot_end_callback(QObject *caller, int method_index);

int signalIndexToMethodIndex(const QMetaObject *metaObject, int signalIndex) {
    return QMetaObjectPrivate::signal(metaObject, signalIndex).methodIndex();
};


// instance 参考：void Probe::createProbe(bool findExisting)
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_lock, (QMutex::Recursive))

class SigSpy {
public:
    static SigSpy * instance() {
        static SigSpy *spy = new SigSpy;
        return spy;
    }
    SigSpy() {
        m_previousSignalSpyCallbackSet.signalBeginCallback
                = qt_signal_spy_callback_set.signal_begin_callback;
        m_previousSignalSpyCallbackSet.signalEndCallback
                = qt_signal_spy_callback_set.signal_end_callback;
        m_previousSignalSpyCallbackSet.slotBeginCallback
                = qt_signal_spy_callback_set.slot_begin_callback;
        m_previousSignalSpyCallbackSet.slotEndCallback
                = qt_signal_spy_callback_set.slot_end_callback;

        registerSignalSpyCallbackSet(m_previousSignalSpyCallbackSet); // daisy-chain existing callbacks

        // connect(this, SIGNAL(objectCreated(QObject*)), m_metaObjectRegistry, SLOT(objectAdded(QObject*)));
        // connect(this, SIGNAL(objectDestroyed(QObject*)), m_metaObjectRegistry, SLOT(objectRemoved(QObject*)));
    }
    static QMutex *objectLock()
    {
        return s_lock();
    }
    bool isValidObject(QObject *obj) const
    {
        return m_validObjects.contains(obj);
    }
    template<typename Func>
    static void executeSignalCallback(const Func &func)
    {
        std::for_each(SigSpy::instance()->m_signalSpyCallbacks.constBegin(),
                      SigSpy::instance()->m_signalSpyCallbacks.constEnd(),
                      func);
    }
    void registerSignalSpyCallbackSet(const SignalSpyCallbackSet &callbacks)
    {
        if (callbacks.isNull()) {
            return;
        }
        m_signalSpyCallbacks.push_back(callbacks);
        setupSignalSpyCallbacks();
        qInfo() << "setupSignalSpyCallbacks";
    }
    void setupSignalSpyCallbacks()
    {
        QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
        foreach (const auto &it, m_signalSpyCallbacks) {
            if (it.signalBeginCallback) cbs.signal_begin_callback = signal_begin_callback;
            if (it.signalEndCallback) cbs.signal_end_callback = signal_end_callback;
            if (it.slotBeginCallback) cbs.slot_begin_callback = slot_begin_callback;
            if (it.slotEndCallback) cbs.slot_end_callback = slot_end_callback;
        }
        qt_register_signal_spy_callbacks(cbs);
        qInfo() << "qt_register_signal_spy_callbacks";
    }
    QVector<SignalSpyCallbackSet> m_signalSpyCallbacks;
    SignalSpyCallbackSet m_previousSignalSpyCallbackSet;
    QSet<QObject *> m_validObjects;
};

static void signal_begin_callback(QObject *caller, int method_index, void **argv)
{
    // qInfo() << "signal_begin_callback";
    return;
    if (method_index == 0 /*|| Probe::instance()->filterObject(caller)*/)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
#endif
    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.signalBeginCallback)
            callbacks.signalBeginCallback(caller, method_index, argv);
    });
}

static void signal_end_callback(QObject *caller, int method_index)
{
    // qInfo() << "signal_end_callback";
    return;
    if (method_index == 0)
        return;

    QMutexLocker locker(SigSpy::objectLock());
    if (!SigSpy::instance()->isValidObject(caller)) // implies filterObject()
        return; // deleted in the slot
    locker.unlock();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
#endif
    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.signalEndCallback)
            callbacks.signalEndCallback(caller, method_index);
    });
}

static void slot_begin_callback(QObject *caller, int method_index, void **argv)
{
    // qInfo() << "slot_begin_callback";
    return;
    if (method_index == 0 /*|| Probe::instance()->filterObject(caller)*/)
        return;

    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.slotBeginCallback)
            callbacks.slotBeginCallback(caller, method_index, argv);
    });
}

static void slot_end_callback(QObject *caller, int method_index)
{
    qInfo() << "slot_end_callback" << caller; return;
    if (method_index == 0)
        return;

    QMutexLocker locker(SigSpy::objectLock());
    if (!SigSpy::instance()->isValidObject(caller)) // implies filterObject()
        return; // deleted in the slot
    locker.unlock();

    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.slotEndCallback)
            callbacks.slotEndCallback(caller, method_index);
    });
}

class ObjectDiscover {
public:
    static ObjectDiscover * instance() {
        static ObjectDiscover *discover = new ObjectDiscover;
        return discover;
    }
    ObjectDiscover() {}
    ~ObjectDiscover() {}
    void findExistingObjects()
    {
        discoverObject(QCoreApplication::instance());
        if (auto guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
            foreach (auto window, guiApp->allWindows()) {
                discoverObject(window);
            }
        }
    }
    void discoverObject(QObject *object)
    {
        if (!object)
            return;

        QMutexLocker lock(s_lock());
        if (m_validObjects.contains(object))
            return;

        objectAdded(object);
        foreach (QObject *child, object->children()) {
            discoverObject(child);
        }
    }
    void objectAdded(QObject *obj, bool fromCtor = false) {
        if (instance()->m_validObjects.contains(obj)) {
            return;
        }
        // make sure we already know the parent
        if (obj->parent() && !instance()->m_validObjects.contains(obj->parent()))
            objectAdded(obj->parent(), fromCtor);
        m_validObjects << obj;
    }
    void dumpObjects() {
        for (auto obj : m_validObjects) {
            qInfo() << obj->objectName() << obj->metaObject()->className();
        }
    }

    QSet<QObject *> m_validObjects;
};

struct Listener
{
    Listener()
        : trackDestroyed(true)
    {
    }

    bool trackDestroyed;
    QVector<QObject *> addedBeforeProbeInstance;
};

Q_GLOBAL_STATIC(Listener, s_listener)


static void (*gammaray_next_startup_hook)() = nullptr;
static void (*gammaray_next_addObject)(QObject *) = nullptr;
static void (*gammaray_next_removeObject)(QObject *) = nullptr;

extern "C" Q_DECL_EXPORT void gammaray_startup_hook()
{
    // Probe::startupHookReceived();
    // new ProbeCreator(ProbeCreator::Create);

    if (gammaray_next_startup_hook)
        gammaray_next_startup_hook();
}

extern "C" Q_DECL_EXPORT void gammaray_addObject(QObject *obj)
{
    puts("gammaray_addObject from Ctor");
    ObjectDiscover::instance()->objectAdded(obj, true); // fromCtor

    if (gammaray_next_addObject)
        gammaray_next_addObject(obj);
}

extern "C" Q_DECL_EXPORT void gammaray_removeObject(QObject *obj)
{
    ObjectDiscover::instance()->objectRemoved(obj);

    if (gammaray_next_removeObject)
        gammaray_next_removeObject(obj);
}

static void installQHooks()
{
    Q_ASSERT(qtHookData[QHooks::HookDataVersion] >= 1);
    Q_ASSERT(qtHookData[QHooks::HookDataSize] >= 6);

    gammaray_next_addObject
        = reinterpret_cast<QHooks::AddQObjectCallback>(qtHookData[QHooks::AddQObject]);
    gammaray_next_removeObject
        = reinterpret_cast<QHooks::RemoveQObjectCallback>(qtHookData[QHooks::RemoveQObject]);
    gammaray_next_startup_hook
        = reinterpret_cast<QHooks::StartupCallback>(qtHookData[QHooks::Startup]);

    qtHookData[QHooks::AddQObject] = reinterpret_cast<quintptr>(&gammaray_addObject);
    qtHookData[QHooks::RemoveQObject] = reinterpret_cast<quintptr>(&gammaray_removeObject);
    qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&gammaray_startup_hook);
}


// QHookData: https://www.msfconsole.cn/qtprivatehook/

int main(int argc, char **argv)
{
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    cbs.signal_end_callback = signal_end_callback;
    cbs.slot_begin_callback = slot_begin_callback;
    cbs.slot_end_callback = slot_end_callback;
    //    qt_register_signal_spy_callbacks(cbs);

    QApplication app(argc, argv);
    QMainWindow w;
    QPushButton *button = new QPushButton(QStringLiteral("Hello World!"), &w);
    button->setObjectName(QStringLiteral("myTestButton"));
    button->resize(400, 300);
    QObject::connect(button, &QPushButton::pressed, [&] {
        auto widget = new QWidget(&w);
        widget->show();
    });

    // QObject *obj = button;
    // qInfo() << obj->objectName() << obj->metaObject()->className();
    // for (int i = 0; i < obj->metaObject()->methodCount(); ++i) {
    //     qInfo() << obj->metaObject()->method(i).methodSignature()
    //             << obj->metaObject()->method(i).access()
    //             << obj->metaObject()->method(i).revision()
    //             << obj->metaObject()->method(i).typeName()
    //             << obj->metaObject()->method(i).tag()
    //             << obj->metaObject()->method(i).methodType();
    // }

    w.show();
    ObjectDiscover::instance()->findExistingObjects();
    ObjectDiscover::instance()->dumpObjects();

    return app.exec();
}
#endif

