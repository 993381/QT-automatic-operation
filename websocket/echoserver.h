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
#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QByteArray>
#include <QProcess>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class EchoServer : public QObject
{
    Q_OBJECT
public:
    explicit EchoServer(quint16 port, bool debug = false, QObject *parent = nullptr);
    ~EchoServer();

    QWebSocket *getAppSocket(const QString &app, int pid = 0);
    bool isOnline(const QString &app);

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection1();
    void onNewConnection2();
    void processTextMessage1(QString message);
    void processTextMessage2(QString message);
    // void processBinaryMessage(QByteArray message);
    void socketDisconnected1();
    void socketDisconnected2();

private:
    QWebSocketServer *m_pWebSocketServer1;
    QWebSocketServer *m_pWebSocketServer2;

    bool m_debug;

    // app client manager
    QMap <int, QPair<QString, QWebSocket*>> m_appList;
    QList<QWebSocket *> m_clients;      // 所有的控制端。目前为单实例，只有一个。应该和instanceID绑定
    // QMap<QWebSocket *, QList<QPair<QString, QProcess *>>> m_processList;  // app name, process
    // QMap<QWebSocket *, QList<QPair<int, QProcess *>>> m_processList;  // app id, process
    QMap<int, QProcess *> m_processMap;

    QPair<QString, int> m_currentSelect;  // name pid
};

#endif //ECHOSERVER_H
