#ifndef LOCAL_SERVER_H
#define LOCAL_SERVER_H

#include <QUrl>
#include <QObject>
#include <QLocalSocket>
#include <QLocalServer>
#include <QScopedPointer>

namespace DtkUiTest {

class LocalServer : public QObject {
    Q_OBJECT

    LocalServer(QObject *parent = nullptr)
        : QObject (parent)
        , m_localServer (new QLocalServer)
    {
        m_localServer->setSocketOptions(QLocalServer::WorldAccessOption);

        connect(m_localServer.data(), &QLocalServer::newConnection, this, [this](){
            QLocalSocket *socket = m_localServer->nextPendingConnection();
            connect(socket, &QLocalSocket::readyRead, [socket, this](){
                if (socket)
                {
                    QTextStream stream(socket);
                    QString result = stream.readAll();
                    sendMessage(socket, result + ". i know ");

                    if (m_controller.second == socket) {
                        // 当前客户端是命令行客户端的话
                        if (result.startsWith("isOnline?")) {
                            QStringList cmds = result.split("?");
                            QString appName = cmds.at(1);
                            for (auto apppid : m_appList) {
                                if (apppid.first == appName){
                                    sendMessage(socket, "true");
                                    return;
                                }
                            }
                            sendMessage(socket, "false");
                            return;
                        }
                        if (result == "dtk-ui-test-client") {
                            m_controller = qMakePair<QString, QLocalSocket*>("dtk-ui-test-client", socket);
                            return;
                        }
                        return;
                    }

                    if (result.startsWith("appinfo:")) {
                        QStringList info = result.split(":");
                        int pid = info.at(1).toInt();
                        QString name = info.at(2);
                        if (m_appList.contains(pid)) {
                            m_appList.erase(m_appList.find(pid));
                        }
                        m_appList.insert(pid, qMakePair<QString, QLocalSocket*>(name, socket));
                        return;
                    }
                }
            });
            connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
        });
    }

    ~LocalServer()
    {
        m_localServer->close();
    }

public:
    static LocalServer *instance() {
        static LocalServer *server = new LocalServer;
        return server;
    }
    bool listen(const QString &key)
    {
        QLocalServer::removeServer(key);
        return m_localServer->listen(key);
    }

    void sendMessage(QLocalSocket *socket, const QString &msg)
    {
        socket->write(msg.toStdString().c_str());
        socket->flush();
    }
private:
    QScopedPointer<QLocalServer> m_localServer;
    QMap <int, QPair<QString, QLocalSocket*>> m_appList;
    QPair <QString, QLocalSocket*> m_controller;
};

}

#endif//LOCAL_SERVER_H

