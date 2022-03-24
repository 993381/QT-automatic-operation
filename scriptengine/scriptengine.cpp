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

    // 几个都要按顺序加载。把他们拼接后一起加载比较方便
    QByteArray channelJs, testBaseJs, testApiJs;
    bool res = fileReadWrite(CHANNEL_JS, channelJs, true)
            && fileReadWrite(TESTBASE_JS, testBaseJs, true)
            && fileReadWrite(TESTAPI_JS, testApiJs, true);

    if (!res) {
        qInfo() << "Error when read CHANNEL_JS && TESTBASE_JS && TESTAPI_JS";
        return;
    }
    QPair<bool, QVariant> result = syncRunJavaScript(channelJs + testBaseJs + testApiJs);
    if (!result.first) {
        qInfo() << "Error when load CHANNEL_JS && TESTBASE_JS && TESTAPI_JS";
        return;
    }
    m_interface->waitForLoadFinished();
    qInfo() << "Read js finished";
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
