#ifndef DBUS_REGISTER_H
#define DBUS_REGISTER_H
#include <QObject>
#include <QDebug>

class DbusRegister: public QObject
{
    Q_OBJECT
    //定义Interface名称为com.Interface.CTestDbus
    Q_CLASSINFO("D-Bus Interface", "com.Interface.DtkTst")
    explicit DbusRegister(QObject* parent = nullptr);

public:
    static DbusRegister *instance();
    bool create();

public Q_SLOTS:
    bool execScriptFromPath(QString path)
    {
        qDebug() <<"execScriptFromPath: " << path;
        bool res = false;
        // TODO
        emit testResult(res);
        return res;
    }
    bool execCommand(QString cmd)
    {
        qDebug() << "execCommand: " << cmd;
        bool res = false;
        // TODO
        emit testResult(res);
        return res;
    }

Q_SIGNALS:
    void testResult(bool);
};

#endif//DBUS_REGISTER_H
