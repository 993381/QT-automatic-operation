#ifndef JS_CPP_INTERFACE_H
#define JS_CPP_INTERFACE_H
#include <QObject>
#include <QEventLoop>
#include "../util.h"
#include "../gdbinjector/gdb_injector.h"

#include <QDebug>

class JsCppInterface : public QObject {
Q_OBJECT
public:
    JsCppInterface(QObject *parent = nullptr)
        : QObject (parent)
        , m_jsLoadFinished (false)
    {
    }
    ~JsCppInterface()
    {
    }

    void waitForLoadFinished();

    Q_SCRIPTABLE void startApp(const QStringList &appParameters) {
        qInfo() << "startApp: " << appParameters;
        // GdbInjector::instance()->launchInject(appParameters);
        GdbInjector::instance()->launchPreload(appParameters);
    }
    Q_SCRIPTABLE void closeApp() {
        qInfo() << "closeApp";
        // GdbInjector::instance()->close();
    }
    Q_SCRIPTABLE void jsLoadFinished() {
        // 当接收到 JS 发来的数据时，向外部发送消息 ...
        m_jsLoadFinished = true;
        qInfo() << "JSLoadFinished";
        Q_EMIT Qt2JsMessage({"ssssss", "xxxxxxxxx"});
    }
    Q_SCRIPTABLE void jsExecFinished(QVariant result) {
        // 可以用一个随机字符标识每次的执行
        // m_jsLoadFinished = true;
        // qInfo() << "JSLoadFinished";
        Q_EMIT execFinished(result);
    }
    Q_SCRIPTABLE void findListItemByText(QString strParameter, QString str) {
        // 当接收到 JS 发来的数据时，向外部发送消息 ...
        qInfo() << strParameter << " " << str;
    }
    Q_SCRIPTABLE bool selectListItem(QStringList value) {
        if (value.size() == 1) {
            return selectListItemByText(value.first());
        } else if (value.size() == 2) {
            return selectListItemByText(value.first(), value.last().toInt());
        }
        return false;
    }
    Q_SCRIPTABLE bool clickButtonByText(QStringList list) {
        if (list.size() == 1) {
            return clickButtonByButtonText(list.first());
        } else if (list.size() == 2) {
            return clickButtonByButtonText(list.first(), list.last().toInt());
        }
        return false;
    }
    Q_SCRIPTABLE bool setLineEditText(QStringList list) {
        if (list.size() == 1) {
            return setLineEditTextByIndex(list.first());
        } else if (list.size() == 2) {
            return setLineEditTextByIndex(list.first(), list.last().toInt());
        }
        return false;
    }

Q_SIGNALS:
    void startTest(const QString &str);
    void Qt2JsMessage(const QStringList &message);
    void Js2QtMessage(const QStringList &message);  //! TODO
    void execFinished(const QVariant &result);

private:
    bool loadFinished () {
        return m_jsLoadFinished;
    }
    bool m_jsLoadFinished;
};

#endif//JS_CPP_INTERFACE_H
