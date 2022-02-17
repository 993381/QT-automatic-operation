#include "sigspy.h"
#include "probe.h"
#include <QDebug>

#include <QMetaObject>
#include <QMetaMethod>
#include <QtCore/private/qobject_p.h>
#include <QtCore/private/qmetaobject_p.h>
#include <QObject>
#include <QMetaType>

#include "signalspycallbackset.h"
using namespace GammaRay;

int signalIndexToMethodIndex(const QMetaObject *metaObject, int signalIndex) {
    return QMetaObjectPrivate::signal(metaObject, signalIndex).methodIndex();
};

template<typename Func>
static void executeSignalCallback(const Func &func)
{
    static SignalSpyCallbackSet m_previousSignalSpyCallbackSet;
    if (m_previousSignalSpyCallbackSet.isNull()) {
        m_previousSignalSpyCallbackSet.signalBeginCallback
                = qt_signal_spy_callback_set.signal_begin_callback;
        m_previousSignalSpyCallbackSet.signalEndCallback
                = qt_signal_spy_callback_set.signal_end_callback;
        m_previousSignalSpyCallbackSet.slotBeginCallback
                = qt_signal_spy_callback_set.slot_begin_callback;
        m_previousSignalSpyCallbackSet.slotEndCallback
                = qt_signal_spy_callback_set.slot_end_callback;
    }
    func(m_previousSignalSpyCallbackSet);
}

void signal_begin_callback(QObject *caller, int method_index, void **argv)
{
#if 1
    if (!Probe::instance()) {
        return;
    }
    // if (Probe::instance()->focusObject() == caller) {
    //     qInfo() << "signal_begin_callback: " <<  caller->metaObject()->method(method_index).methodSignature().data();
    // }
    // if (!Probe::instance()->focusObjects().size()) return;
    QObjectList tmpList = Probe::instance()->focusObjects();
    // if (tmpList.contains(caller)) {
    if (tmpList.contains(caller) && caller != Probe::instance()) {
        method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
        qInfo() << "signal_begin_callback: " <<  caller << caller->metaObject()->className() << " " << caller->metaObject()->method(method_index).methodSignature().data();
        for (int j = 0; j < caller->metaObject()->method(method_index).parameterCount(); ++j) {
            const QByteArray parameterType = caller->metaObject()->method(method_index).parameterTypes().at(j);
            qInfo() << "parameterType: " << parameterType;
            if (parameterType == "bool") {
                qInfo() << "parameterValue: " << *(bool *)argv;
            }
        }
    }
    // if (Probe::instance()->isObjectCreationQueued(caller)) {
    //    printf("signal_begin_callback: %s\n", caller->metaObject()->method(method_index).methodSignature().data());
    // }
#endif
    return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
#endif
    executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.signalBeginCallback)
            callbacks.signalBeginCallback(caller, method_index, argv);
    });
}

void signal_end_callback(QObject *caller, int method_index)
{
#if 1
    if (!Probe::instance()) {
        return;
    }
    // if (Probe::instance()->focusObject() == caller) {
    //     qInfo() << "signal_begin_callback: " <<  caller->metaObject()->method(method_index).methodSignature().data();
    // }
    if (!Probe::instance()->focusObjects().size()) return;
    // QObjectList tmpList = Probe::instance()->focusObjects();
    // if (tmpList.contains(caller)) {
    if (/*caller->inherits("QObject") && */caller != Probe::instance()) {
        //  qInfo() << "signal_end_callback: " <<  caller << caller->metaObject()->className() << " " << caller->metaObject()->method(method_index).methodSignature().data();

    }
#endif
    // qInfo() << "signal_end_callback";
    return;
    //    if (method_index == 0)
    //        return;

    //    QMutexLocker locker(Probe::objectLock());
    //    if (!SigSpy::instance()->isValidObject(caller)) // implies filterObject()
    //        return; // deleted in the slot
    //    locker.unlock();

    //#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    //    method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
    //#endif
    //    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
    //        if (callbacks.signalEndCallback)
    //            callbacks.signalEndCallback(caller, method_index);
    //    });
}

void slot_begin_callback(QObject *caller, int method_index, void **argv)
{
    if (!Probe::instance()) {
        return;
    }
    if (Probe::instance()->isObjectCreationQueued(caller)) {
        printf("slot_begin_callback: %s\n", caller->metaObject()->method(method_index).methodSignature().data());
    }
    return;
    //    if (method_index == 0 /*|| Probe::instance()->filterObject(caller)*/)
    //        return;

    //    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
    //        if (callbacks.slotBeginCallback)
    //            callbacks.slotBeginCallback(caller, method_index, argv);
    //    });
}

void slot_end_callback(QObject *caller, int method_index)
{
    if (!Probe::instance()) {
        return;
    }
    if (Probe::instance()->isObjectCreationQueued(caller)) {
        printf("slot_end_callback: %s\n", caller->metaObject()->method(method_index).methodSignature().data());
    }
    return;
    //    if (method_index == 0)
    //        return;

    //    QMutexLocker locker(SigSpy::objectLock());
    //    if (!SigSpy::instance()->isValidObject(caller)) // implies filterObject()
    //        return; // deleted in the slot
    //    locker.unlock();

    //    SigSpy::executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
    //        if (callbacks.slotEndCallback)
    //            callbacks.slotEndCallback(caller, method_index);
    //    });
}

