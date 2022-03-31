/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "echoserver.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>
#include <QCoreApplication>
#include <QProcess>
#include <QTimer>
#include <dlfcn.h>

QT_USE_NAMESPACE

EchoServer::EchoServer(quint16 port, bool debug, QObject *parent)
    : QObject(parent)
    , m_pWebSocketServer1(new QWebSocketServer(QStringLiteral("Client Server"),
                                            QWebSocketServer::NonSecureMode, this))
    , m_pWebSocketServer2(new QWebSocketServer(QStringLiteral("Control Server"),
                                            QWebSocketServer::NonSecureMode, this))
    , m_debug(debug)
{
    // 用于接受客户端连接
    if (m_pWebSocketServer1->listen(QHostAddress::Any, port)) {
        if (m_debug)
            qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer1, &QWebSocketServer::newConnection,
                this, &EchoServer::onNewConnection1);
        // connect(m_pWebSocketServer1, &QWebSocketServer::closed, qApp, &QCoreApplication::quit);
    }
    // 用于接受控制端连接
    if (m_pWebSocketServer2->listen(QHostAddress::Any, port + 1)) {
        if (m_debug)
            qDebug() << "Echoserver listening on port" << port + 1;
        connect(m_pWebSocketServer2, &QWebSocketServer::newConnection,
                this, &EchoServer::onNewConnection2);
        connect(m_pWebSocketServer2, &QWebSocketServer::closed, qApp, &QCoreApplication::quit);
    }
}

EchoServer::~EchoServer()
{
    m_pWebSocketServer1->close();
    m_pWebSocketServer2->close();
    //!TODO qDeleteAll(m_clients.begin(), m_clients.end());
}

void EchoServer::onNewConnection1()
{
    QWebSocket *pSocket = m_pWebSocketServer1->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processTextMessage1);
    // connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EchoServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &EchoServer::socketDisconnected1);
}

void EchoServer::onNewConnection2()
{
    QWebSocket *pSocket = m_pWebSocketServer2->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processTextMessage2);
    // connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EchoServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &EchoServer::socketDisconnected2);
}

void EchoServer::processTextMessage1(QString message)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    socket->sendTextMessage("Server recved: " + message);

    int appPid = 0;
    if (message.startsWith("appinfo:")) {
        QStringList info = message.split(":");
        appPid = info.at(1).toInt();
        QString name = info.at(2);
        if (m_appList.contains(appPid)) {
            m_appList.erase(m_appList.find(appPid));
        }
        m_appList.insert(appPid, qMakePair<QString, QWebSocket*>(name, socket));

        m_currentSelect.first = name;
        m_currentSelect.second = appPid;

        // 收到登陆消息后，如果发现控制端正在等待注入结果，就发送注入成功的消息
        if (injectNotice.second && injectNotice.first == appPid) {
            // 清空，5秒后会再判断，如果还没收到就算超时，发送失败消息
            auto sock = injectNotice.second;
            injectNotice.first = 0;
            injectNotice.second = nullptr;
            sock->sendTextMessage(QString("Online-status-2:1"));
        }

        return;
    }

    if (message.startsWith("Exec-")) {
        for (auto c : m_clients) {
            c->sendTextMessage(message);
        }
    }
}

void EchoServer::processTextMessage2(QString message) {
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    socket->sendTextMessage("Server recved: " + message);

    // 程序注册
    if (message == "dtk-ui-test-client") {
        // (qMakePair<QString, QWebSocket*>("dtk-ui-test-client", socket));
        m_clients.push_back(socket);
        socket->sendTextMessage("loginOn");
        return;
    }
    // 处理在线查询的请求
    if (message.startsWith("isOnline?")) {
        QStringList cmds = message.split("?");
        QString appName = cmds.at(1);
        for (auto apppid : m_appList) {
            if (apppid.first == appName){
                socket->sendTextMessage("true");
                return;
            }
        }
        socket->sendTextMessage("false");
        return;
    }
    // 程序启动
    if (message.startsWith("launch:")) {
        QStringList msg = message.split(":");
        if (isOnline(msg.at(1))) {
            socket->sendTextMessage("Already-Online");
        } else {
            if (!qgetenv("INJECTOR_PATH").isEmpty()) {
                qputenv("LD_PRELOAD", qgetenv("INJECTOR_PATH"));
            } else {
                qputenv("LD_PRELOAD", INJECTOR_DLL);
            }

            // qputenv("EXEC_JS_SCRIPT_PRE", "1");
            // qputenv("SHOW_UIA_WINDOW_PRE", "1");
            QProcess *process = new QProcess;
            m_processMap.insert(process->pid(), process);

            QStringList args;
            if (msg.size() > 2) {
                for (int i = 2; i < msg.size(); ++i) {
                    args << msg.at(i);
                }
            }
            process->start(msg.at(1), args);
            bool status = process->waitForStarted(-1);
            socket->sendTextMessage(status ? "launch success" : "launch failed" + process->readAll());
        }
    }
    if (message.startsWith("isOnline-pid-1:")) {
        int pid = message.split(":").at(1).toInt();
        socket->sendTextMessage(QString("Online-status-1:%1").arg(isOnline(pid)));
        if (isOnline(pid)) {
            auto sock = m_appList.find(pid);
            m_currentSelect.first =  sock.value().first;
            m_currentSelect.second = pid;
        }
    }
    if (message.startsWith("isOnline-pid-2:")) {
        int pid = message.split(":").at(1).toInt();
        // socket->sendTextMessage(QString("Online-status-1:%1").arg(isOnline(pid)));
        injectNotice.first = pid;
        injectNotice.second = socket;
        // 5秒后如果还没收到就算超时，发送失败消息
        QTimer::singleShot(1000*5, [this]{
            if (injectNotice.second) {
                injectNotice.second->sendTextMessage("Online-status-2:0");
            }
        });
    }
    if (message.startsWith("execute-script:")) {
        QStringList msg = message.split(":");
        if (!isOnline(m_currentSelect.first)) {
            socket->sendTextMessage("App-not-online");
        } else {
            if (auto appSock = getAppSocket(m_currentSelect.first)) {
                appSock->sendTextMessage(QString("Exec-script:%1").arg(msg.at(1)));   // TODO: Exec-Reply
            }
        }
    }
    if (message.startsWith("execute-function:")) {
        QStringList msg = message.split(":");
        if (!isOnline(m_currentSelect.first)) {
            socket->sendTextMessage("App-not-online: " + m_currentSelect.first);
        } else {
            if (auto appSock = getAppSocket(m_currentSelect.first)) {
                appSock->sendTextMessage(QString("Exec-function:%1").arg(msg.at(1)));   // TODO: Exec-Reply
            }
        }
    }
}

//void EchoServer::processBinaryMessage(QByteArray message)
//{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    if (m_debug)
//        qDebug() << "Binary Message received:" << message;
//    if (pClient) {
//        pClient->sendBinaryMessage(message);
//    }
//}

QWebSocket *EchoServer::getAppSocket(const QString &appName, int pid) {
    if (pid) {
        auto app = m_appList.find(pid);
        if (app.key() == pid && app.key() == app->first) {
            return app->second;
        }
    }
    QMap <int, QPair<QString, QWebSocket*>>::iterator itr = m_appList.begin();
    for (; itr != m_appList.end(); ++itr) {
        if (itr->first == appName) {
            return itr->second;
        }
    }
    return nullptr;
}

bool EchoServer::isOnline(const QString &app) {
    QMap <int, QPair<QString, QWebSocket*>>::iterator itr = m_appList.begin();
    for (; itr != m_appList.end(); ++itr) {
        if (itr->first == app) {
            return true;
        }
    }
    return false;
}
bool EchoServer::isOnline(const int &appPid) {
    QMap <int, QPair<QString, QWebSocket*>>::iterator itr = m_appList.begin();
    for (; itr != m_appList.end(); ++itr) {
        if (itr.key() == appPid) {
            return true;
        }
    }
    return false;
}

void EchoServer::socketDisconnected1()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;

    int appPid = 0;
    // 清理 m_appList 和 m_appList
    QMap <int, QPair<QString, QWebSocket*>>::iterator itr = m_appList.begin();
    for (; itr != m_appList.end(); ++itr) {
        if (itr->second == pClient) {
            appPid = itr.key();
            itr->second->deleteLater();
            m_appList.erase(itr);
            break;
        }
    }
    // app 退出以后，清理它对应的 process
    if (appPid) {
        auto res = m_processMap.find(appPid);
        if (res != m_processMap.end()) {
            res.value()->close();
            res.value()->deleteLater();
            m_processMap.erase(res);
        }
    }
}

void EchoServer::socketDisconnected2()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug)
        qDebug() << "socketDisconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
