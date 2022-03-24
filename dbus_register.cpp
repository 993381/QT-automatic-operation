#include "dbus_register.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QCoreApplication>

#define AUTO_TEST_SERVICE QStringLiteral("/com/AutoTest/%1").arg(qAppName())

DbusRegister::DbusRegister(QObject* parent)
    :QObject(parent)
{
}

DbusRegister *DbusRegister::instance() {
    static DbusRegister *obj = new DbusRegister;
    return obj;
}

bool DbusRegister::create() {
    // 建立到 session bus 的连接
    QDBusConnection connection = QDBusConnection::sessionBus();
    // 在 session bus 上注册名为 com.Server.Server1 的服务
    if (!connection.registerService("com.auto.tst.server"))
    {
        qDebug() << "error:" << connection.lastError().message();
        return false;
    }
    // 注册名为 /com/ObjectPath/CTestDbus 的对象，把类 Object 所有槽函数和信号导出为 object 的 method
    if (!connection.registerObject(AUTO_TEST_SERVICE, instance(),
                                   QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))
    {
        qDebug() << "error:" << connection.lastError().message();
        return false;
    }
    return true;
}
