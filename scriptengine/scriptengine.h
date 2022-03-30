#ifndef WEBENGINE_H
#define WEBENGINE_H
#include <QObject>

#include <QtWebEngineWidgets/QWebEngineHistory>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QtWebEngineWidgets/QtWebEngineWidgets>

#include <QJSValue>
#include <QJSEngine>

class JsCppInterface;

class ScriptEngine : public QObject {
    Q_DISABLE_COPY(ScriptEngine)
    ScriptEngine();
    ~ScriptEngine() {}
public:
    static ScriptEngine* instance();
    JsCppInterface *interface() {
        return m_interface;
    }
    QJSValue runScript (const QString &string);
    // QPair<bool, QVariant> syncRunJavaScript(const QString &javascript, int msec = 0);
    void execTest(const QStringList &string);

private:
    QScopedPointer<QJSEngine> m_jsEngine;
    JsCppInterface *m_interface;
};

#endif//WEBENGINE_H
