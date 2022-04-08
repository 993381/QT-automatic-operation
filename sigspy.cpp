#include "sigspy.h"
#include "probe.h"
#include <QDebug>

#include <QMetaObject>
#include <QMetaMethod>
#include <QtCore/private/qobject_p.h>
#include <QtCore/private/qmetaobject_p.h>
#include <QLabel>
#include <QObject>
#include <QMetaType>
#include <QListView>
#include <QLineEdit>
#include <QModelIndex>

#include "objectlistmanager.h"
#include "signalspycallbackset.h"
#include "objectpath.h"
#include "util.h"

using namespace GammaRay;

int signalIndexToMethodIndex(const QMetaObject *metaObject, int signalIndex) {
    return QMetaObjectPrivate::signal(metaObject, signalIndex).methodIndex();
};

template<typename Func>
static void executeSignalCallback(const Func &func)
{
    static SignalSpyCallbackSet m_previousSignalSpyCallbackSet;
    if (m_previousSignalSpyCallbackSet.isNull()) {
#if QT_VERSION > QT_VERSION_CHECK(5, 11, 3)
        m_previousSignalSpyCallbackSet.signalBeginCallback
                = qt_signal_spy_callback_set.load()->signal_begin_callback;//.signal_begin_callback;
        m_previousSignalSpyCallbackSet.signalEndCallback
                = qt_signal_spy_callback_set.load()->signal_end_callback;
        m_previousSignalSpyCallbackSet.slotBeginCallback
                = qt_signal_spy_callback_set.load()->slot_begin_callback;
        m_previousSignalSpyCallbackSet.slotEndCallback
                = qt_signal_spy_callback_set.load()->slot_end_callback;
#else
        m_previousSignalSpyCallbackSet.signalBeginCallback
                = qt_signal_spy_callback_set.signal_begin_callback;//.signal_begin_callback;
        m_previousSignalSpyCallbackSet.signalEndCallback
                = qt_signal_spy_callback_set.signal_end_callback;
        m_previousSignalSpyCallbackSet.slotBeginCallback
                = qt_signal_spy_callback_set.slot_begin_callback;
        m_previousSignalSpyCallbackSet.slotEndCallback
                = qt_signal_spy_callback_set.slot_end_callback;
#endif
    }
    func(m_previousSignalSpyCallbackSet);
}

void signal_begin_callback(QObject *caller, int method_index_in, void **argv)
{
    if (!Probe::instance()) {
        return;
    }
    if (!Probe::instance()->focusObjects().contains(caller) || ObjectListManager::instance()->isInBlackList(caller, true)) {
        return;
    }
    if (caller->metaObject()->className() == QByteArrayLiteral("QWidgetLineControl")) {
        return;
    }
    QObjectList tmpList = Probe::instance()->focusObjects();
    if (tmpList.contains(caller) && caller != Probe::instance()) {
        const int &method_index = signalIndexToMethodIndex(caller->metaObject(), method_index_in);
        const QString methodSignature = caller->metaObject()->method(method_index).methodSignature();

        QAbstractButton * button = qobject_cast<QAbstractButton *>(caller);
        if (button && methodSignature == "released()") {
            ObjInfo info = findUniqInfo(caller, button->text());
            if (info.index != -1) {
                qInfo() << "find method: " << type2Str[info.type] << info.value.toString();
            }
        }
        QAbstractItemView *listView = qobject_cast<QAbstractItemView *>(caller);
        if (listView && methodSignature == "pressed(QModelIndex)") {
            for (int j = 0; j < caller->metaObject()->method(method_index).parameterCount(); ++j) {
                const QByteArray parameterType = caller->metaObject()->method(method_index).parameterTypes().at(j);
                qInfo() << "listView objectName: " << listView->objectName()  // 空的
                        << "parameterTypes: " << caller->metaObject()->method(method_index).parameterTypes()
                        << "parameterNames: " << caller->metaObject()->method(method_index).parameterNames()
                        << "parameterType: " << caller->metaObject()->method(method_index).parameterType(j);
                if (parameterType == "QModelIndex") {
                    QModelIndex *modelIndex = (QModelIndex *)argv[j+1];
                    ObjInfo info = findUniqInfo(caller, *modelIndex);
                    if (info.index != -1) {
                        qInfo() << "index: " << info.index << " find method: " << type2Str[info.type];
                        // 查找项(text, index) && 选中(text, index)  查询就返回找到结果的个数
                    }
                }
            }
        }
    }
}

void signal_begin_callback2(QObject *caller, int method_index_in, void **argv)
{
#if 1
    if (!Probe::instance()) {
        return;
    }
    if (!Probe::instance()->focusObjects().contains(caller) || ObjectListManager::instance()->isInBlackList(caller, true)) {
        return;
    }
    if (caller->metaObject()->className() == QByteArrayLiteral("QWidgetLineControl")) {
        return;
    }
    QObjectList tmpList = Probe::instance()->focusObjects();
    if (tmpList.contains(caller) && caller != Probe::instance()) {
        const int &method_index = signalIndexToMethodIndex(caller->metaObject(), method_index_in);
        const QString methodSignature = caller->metaObject()->method(method_index).methodSignature();

        QAbstractButton * button = qobject_cast<QAbstractButton *>(caller);
        if (button && methodSignature == "pressed()") {
            qInfo() << "xxxxxxxxxxxxxxxxxxxxx: "
                    <<  caller << " class name: " << caller->metaObject()->className() << " "
                     << " object name: " << caller->objectName() << " "
                     << " accessible name: " << button->accessibleName() << " ";

            ObjectPath path(ObjectPath::parseObjectPath(caller));       // 加入重复对象的检测
            path.dump();
            path.setRecordMethod("pressed()");
            if (!button->text().isEmpty()) {
                const QObjectList &result = findObjects(ByButtonText, button->text());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { button->text() }, "click()", "FindButtonByButtonText", uniq_index);
            } else if (!button->objectName().isEmpty()) {
                const QObjectList &result = findObjects(ByObjectName, button->objectName());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { button->objectName() }, "click()", "FindButtonByObjectName", uniq_index);
            } else if (!button->accessibleName().isEmpty()){
                qInfo() << "FindButtonByAccessibleName ................................1";
                const QObjectList &result = findObjects(ByAccessibleName, button->accessibleName());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    qInfo() << "FindButtonByAccessibleName ................................2";
                    uniq_index = result.indexOf(caller);
                    qInfo() << "FindButtonByAccessibleName ................................3";
                }
                path.setParameters(1, {"QString"}, { button->accessibleName() }, "click()", "FindButtonByAccessibleName", uniq_index);
            } else if (!button->toolTip().isEmpty()) {
                const QObjectList &result = findObjects(ByToolTip, button->toolTip());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { button->toolTip() }, "click()", "FindButtonByToolTip", uniq_index);
            } else {   // 啥都没有，只有图标的 button
                const QObjectList &result = findObjects(ByClassName, button->metaObject()->className());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { button->metaObject()->className() }, "click()", "FindButtonByButtonIndex", uniq_index);
            }
            ObjectPathManager::instance()->append(path);
            return;
        }

        QAbstractItemView *listView = qobject_cast<QAbstractItemView *>(caller);
        if (listView && methodSignature == "pressed(QModelIndex)") {
            ObjectPath path(ObjectPath::parseObjectPath(caller));       // 加入重复对象的检测
            path.setRecordMethod("pressed(QModelIndex)");
            //! 获取参数
            for (int j = 0; j < caller->metaObject()->method(method_index).parameterCount(); ++j) {
                const QByteArray parameterType = caller->metaObject()->method(method_index).parameterTypes().at(j);

                qInfo() << "parameterTypes: " << caller->metaObject()->method(method_index).parameterTypes()
                        << "parameterNames: " << caller->metaObject()->method(method_index).parameterNames()
                        << "parameterType: " << caller->metaObject()->method(method_index).parameterType(j);
                if (parameterType == "QModelIndex") {
                    QModelIndex *modelIndex = (QModelIndex *)argv[j+1];  // 根据 QMetaMethod::invoke 推断出参数从 1 开始
                    // 获取到点击的行、列以及显示的数据
                    qInfo() << "parameterValue: " <<  modelIndex->data().toString() << modelIndex << modelIndex->row() << " " << modelIndex->column();
                    const QString &itemText = modelIndex->data().toString();
                    int uniq_index = -1;
                    QObjectList objs;
                    if (!itemText.isEmpty()) {
                        objs = findObjects(ByItemText, itemText);
                    } else {
                        objs = findObjects(ByClassName, listView->metaObject()->className());
                    }
                    // 如果不唯一，要记住被点击项的全局索引
                    if (objs.size() > 1) {
                        uniq_index = objs.indexOf(caller);
                    } else if (objs.size() == 1){
                        uniq_index = 0;
                    } else {
                        qInfo() << "Fatal error, cannot find!";
                    }
                    // path.setParameters(1, {"QString"}, {modelIndex->data().toString()}, "selectListItemByText", "FindListItemByText", uniq_index);
                    if (itemText.isEmpty()) {
                        path.setParameters(3, { "QString", "int", "int" }
                                           , { listView->metaObject()->className(),  modelIndex->row(), modelIndex->column() }
                                           , "selectListItemByIndex", "FindListItemByIndex", uniq_index);
                    } else {
                        path.setParameters(3, { "QString", "int", "int" }
                                           , { modelIndex->data().toString(), modelIndex->row(), modelIndex->column() }
                                           , "selectListItemByText", "FindListItemByText", uniq_index);
                    }
                    ObjectPathManager::instance()->append(path);
                    // 再在这里查找一下这个是不是唯一的，否则用合适的方式存储，最后分析json序列生成代码
                }
            }
            return;
        }
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(caller);
        if (lineEdit && methodSignature == "editingFinished()") {
            ObjectPath path(ObjectPath::parseObjectPath(caller));       // 加入重复对象的检测
            path.dump();
            path.setRecordMethod("setText(QString)");
#if 0
            if (!lineEdit->objectName().isEmpty()) {
                const QObjectList &result = findObjects(ByObjectName, lineEdit->objectName());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { lineEdit->objectName() }, "click()", "FindLineEditByObjectName", uniq_index);
            } else if (!lineEdit->accessibleName().isEmpty()){
                qInfo() << "FindButtonByAccessibleName ................................1";
                const QObjectList &result = findObjects(ByAccessibleName, lineEdit->accessibleName());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    qInfo() << "FindButtonByAccessibleName ................................2";
                    uniq_index = result.indexOf(caller);
                    qInfo() << "FindButtonByAccessibleName ................................3";
                }
                path.setParameters(1, {"QString"}, { lineEdit->accessibleName() }, "click()", "FindLineEditByAccessibleName", uniq_index);
            } else if (!lineEdit->text().isEmpty()) {
                const QObjectList &result = findObjects(ByButtonText, lineEdit->text());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { lineEdit->text() }, "click()", "FindLineEditByButtonText", uniq_index);
            } else if (!lineEdit->toolTip().isEmpty()) {
                const QObjectList &result = findObjects(ByToolTip, lineEdit->toolTip());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(1, {"QString"}, { lineEdit->toolTip() }, "click()", "FindLineEditByToolTip", uniq_index);
            }
#endif
            {   // 啥都没有，只有图标的 button
                const QObjectList &result = findObjects(ByClassName, lineEdit->metaObject()->className());
                bool isUnique = result.size() == 1;
                int uniq_index = -1;
                if (!isUnique) {
                    uniq_index = result.indexOf(caller);
                }
                path.setParameters(4, {"ClassName", "Layer", "SilbingIndex",  "TextContent" }
                                   , { lineEdit->metaObject()->className()
                                       , ObjectPath::parseObjectInfo(caller).depth
                                       , ObjectPath::parseObjectInfo(caller).index
                                       , lineEdit->text()
                                   }
                                   , "editingFinished()", "FindLineEditByItemIndex", uniq_index);
            }
            ObjectPathManager::instance()->append(path);
            return;
        }
        if (caller) {
             const QString &method = caller->metaObject()->method(method_index).methodSignature();
             int methodType = caller->metaObject()->method(method_index).methodType();
             int access = caller->metaObject()->method(method_index).access();
             // 只关心公开的信号
             if (QMetaMethod::Access::Public != access && QMetaMethod::MethodType::Signal != methodType) {
                 return;
             }
             qInfo() << "method: " << method << " methodType: " << methodType;
             return;
        }
    }
#endif
    return;
#if 0
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    method_index = signalIndexToMethodIndex(caller->metaObject(), method_index);
#endif
    executeSignalCallback([=](const SignalSpyCallbackSet &callbacks) {
        if (callbacks.signalBeginCallback)
            callbacks.signalBeginCallback(caller, method_index, argv);
    });
#endif
}
# if 0
void signal_end_callback(QObject *caller, int method_index)
{
#if 0
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
#endif
