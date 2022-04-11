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
#include "util.h"

#if QT_VERSION > QT_VERSION_CHECK(5, 11, 3)
#include <QScopeGuard>
#else
#include "scopeguard.h"
#endif

#include "steprecord.h"

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

QString paramGenerate(const ObjInfo &info, QWidget *widget) {
    qInfo() << "index: " << info.index << " find method: " << type2Str[info.type];
    QStringList params;
    if (info.type == byAccName) {
        params << "'byAcc'" << QString("'%1'").arg(widget->accessibleName());
    }
    if (info.type == byObjName) {
        params << "'byObj'" << QString("'%1'").arg(widget->objectName());
    }
    if (info.type == byClassName) {
        params << "'byClass'" << QString("'%1'").arg(widget->metaObject()->className());
    }
    if ((info.type == byAccName || info.type == byObjName) && !params.isEmpty()) {
        if (info.index != 0) {
            qInfo() << "Error, 标记过的控件不唯一: " << params;
        }
    }
    if (info.index != 0) {
        params << QString("%1").arg(info.index);
    }

    QString cmdParam;
    for (int i = 0; i < params.size(); ++i) {
        cmdParam += params.at(i);
        if (i != params.size() - 1) {
            cmdParam += ", ";
        }
    }
    return cmdParam;
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

        QString finalCmd; // 最终生成的可执行的 Js 文本

        QAbstractButton *button = qobject_cast<QAbstractButton *>(caller);
        if (button && methodSignature == "released()") {
            ObjInfo info = findUniqInfo(caller, button->text());
            if (info.index != -1) {
                qInfo() << "find method: " << type2Str[info.type] << info.value.toString();
                if (info.type == byButtonText) {
                    // 根据文本找对象
                    finalCmd = ((info.index == 0) ? QString("点击('%1')").arg(info.value.toString()) : QString("点击('%1', %2)").arg(info.value.toString()).arg(info.index));
                } else {
                    // 文本不唯一或不存在根据 acc、obj、class 名称找对象
                    QString cmdParam = paramGenerate(info, button);
                    finalCmd = QString("点击(%1)").arg(cmdParam);
                }
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
                        // 选择(text, index) or 选择(text, byAcc, accName, index)
                        finalCmd = ((info.index == 0) ? QString("选择('%1')").arg(info.value.toString()) : QString("选择('%1', %2)").arg(info.value.toString()).arg(info.index));
                    }
                }
            }
        }
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(caller);
        if (lineEdit && methodSignature == "editingFinished()") {
            qInfo() << "editingFinished ..........................";
            ObjInfo info = findUniqInfo(caller);
            QString cmdParam = paramGenerate(info, lineEdit);
            finalCmd = QString("输入('%1', %2)").arg(lineEdit->text()).arg(cmdParam);
        }

        // 追加显示生产的执行命令
        if (!finalCmd.isEmpty())
            StepRecord::instance()->append(finalCmd);
    }
}

