#ifndef WEBENGINE_H
#define WEBENGINE_H
#include <QObject>

#include <QtWebEngineWidgets/QWebEngineHistory>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QtWebEngineWidgets>

class JsCppInterface;

class ScriptEngine : public QObject {
    Q_DISABLE_COPY(ScriptEngine)
    ScriptEngine();
    ~ScriptEngine() {}
public:
    static ScriptEngine* instance();

    QWidget *widget() {
        return m_webViewEngine.get();
    }
    void runScript (const QString &string);
    QPair<bool, QVariant> syncRunJavaScript(const QString &javascript, int msec = 0);

private:
    QScopedPointer<QWebEngineView> m_webViewEngine;
    JsCppInterface *m_interface;
};

#endif//WEBENGINE_H
