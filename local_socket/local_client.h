#ifndef LOCAL_CLIENT_H
#define LOCAL_CLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QLocalServer>
#include <QScopedPointer>

// dup
#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <QCoreApplication>

class LocalClient : public QObject
{
    Q_OBJECT

    LocalClient() : m_socket (new QLocalSocket)
    {
        connect(m_socket.data(), &QLocalSocket::connected, [this]{
            qDebug() << "connected.";
            if (!m_appInfo.isEmpty()) {
                // 应用注册自己的 pid 和 appName
                sendMessage(QString("appinfo:%1:%2").arg(m_appInfo.at(0)).arg(m_appInfo.at(1)));
            } else {
                // 命令行客户端注册自己的信息
                sendMessage("dtk-ui-test-client");
            }
        });
        connect(m_socket.data(), &QLocalSocket::disconnected, []{
            qDebug() << "disconnected.";
        });
        connect(m_socket.data(), static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
                [](QLocalSocket::LocalSocketError error){
            qWarning() << error;
        });
    }

    ~LocalClient()
    {
        m_socket->disconnectFromServer();
    }
public:
    static LocalClient *instance() {
        static LocalClient *client = new LocalClient;
        return client;
    }

public:
    void ConnectToServer(const QString &strServerName)
    {
        //! TODO error handle
        m_socket->connectToServer(strServerName);
        if (m_socket->waitForConnected())
        {
            connect(m_socket.data(), &QLocalSocket::readyRead, [this](){
                QTextStream stream(m_socket.data());
                if (m_readyRead) {
                    m_readyRead(stream.readAll());
                }
            });
        }
    }

    void setReadCallback(std::function<void(QString)>readyRead) {
        m_readyRead = readyRead;
    }

    // default timeout msecs = 30000
    QString syncRead() {
        if (!m_socket->bytesAvailable())
            m_socket->waitForReadyRead();

        QTextStream stream(m_socket.data());
        QString respond = stream.readAll();
        qDebug() << "Read Data From Server: " << respond;
        return respond;
    }

    void sendMessage(const QString &msg)
    {
        m_socket->write(msg.toStdString().c_str());
        m_socket->flush();
    }

//    static void outputRedirection(QtMsgType type, const QMessageLogContext &context, const QString &msg)
//    {
//        LocalClient::instance()->m_socket->write(msg.toStdString().c_str());
//        LocalClient::instance()->m_socket->flush();
//    }

    void setAppInfo(QStringList info) {
        m_appInfo = info;
    }

private:
    QScopedPointer<QLocalSocket> m_socket;
    QStringList m_appInfo;
    std::function<void(QString)> m_readyRead;
};

#if 0
        // 把客户端的消息重定向到服务端
        int ret = dup3(m_socket->socketDescriptor(), STDOUT_FILENO, O_CLOEXEC);
        if (ret < 0) {
            perror("dup3");
        }
        qInfo() << m_socket->socketDescriptor();
        ret = dup3(m_socket->socketDescriptor(), STDERR_FILENO, O_CLOEXEC);
        if (ret < 0) {
            perror("dup3");
        }

        // qInstallMessageHandler(outputRedirection);
#endif

#endif//LOCAL_CLIENT_H
