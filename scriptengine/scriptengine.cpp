#include "scriptengine.h"
#include "js_cpp_interface.h"
#include "../util.h"

#include <unistd.h>

#include <QUrl>
#include <QWebChannel>

class WebPage : public QWebEnginePage {
public:
    WebPage() {}
    ~WebPage() {}
    // 重载以获取 console.log
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override {
        // qInfo() << "level: " << level << " message: " << message << " linNumber: "  << sourceID;
        qInfo() << "message: " << message;
    }
};

ScriptEngine::ScriptEngine()
    : m_webViewEngine (new QWebEngineView)
    , m_interface (nullptr)
{
    WebPage *page = new WebPage;
    m_webViewEngine->setPage(page);

    QWebChannel *webChannel = new QWebChannel;
    m_interface = new JsCppInterface(this);
    webChannel->registerObject(QStringLiteral("tester"), m_interface);
    page->setWebChannel(webChannel);

    QPair<bool, QVariant> result;
    QByteArray channelJs;
    if (fileReadWrite(CHANNEL_JS, channelJs, true)) {
        result = syncRunJavaScript(channelJs);
        if (!result.first) {
            qInfo() << "error when load CHANNEL_JS";
        }
    }
    QByteArray testerJs;
    if (fileReadWrite(TESTER_JS, testerJs, true)) {
        result = syncRunJavaScript(testerJs);
        if (!result.first) {
            qInfo() << "error when load TESTER_JS";
        }
    }

    m_interface->waitForLoadFinished();
    qInfo() << "read js finished";
}

ScriptEngine* ScriptEngine::instance()  {
    static ScriptEngine* manager = new ScriptEngine;
    return manager;
}

void ScriptEngine::runScript(const QString &string) {
     m_webViewEngine->page()->runJavaScript(string);
}

QPair<bool, QVariant> ScriptEngine::syncRunJavaScript(const QString &javascript, int msec) {
     QPair<bool, QVariant> result = qMakePair(false, 0);
     QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
     if (msec) QTimer::singleShot(msec, loop.data(), &QEventLoop::quit);
     m_webViewEngine->page()->runJavaScript(javascript, [loop, &result](const QVariant &val) {
         if (loop->isRunning()) {
             result.first = true;
             result.second = val;
             loop->quit();
         }
     });
     loop->exec();
     return result;
 }
