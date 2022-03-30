#include "scriptengine.h"
#include "js_cpp_interface.h"
#include "../util.h"
#include <unistd.h>

ScriptEngine::ScriptEngine()
    : m_jsEngine (new QJSEngine)
    , m_interface (nullptr)
{
    m_jsEngine->installExtensions(QJSEngine::TranslationExtension | QJSEngine::ConsoleExtension);
    m_interface = new JsCppInterface(this);

    QJSValue scriptBtn = m_jsEngine->newQObject(m_interface);
    m_jsEngine->globalObject().setProperty("TestMethod", scriptBtn);

    // 几个都要按顺序加载。把他们拼接后一起加载比较方便
    QByteArray testBaseJs, testApiJs;
    bool res = fileReadWrite(TESTBASE_JS, testBaseJs, true) && fileReadWrite(TESTAPI_JS, testApiJs, true);
    if (!res) {
        qInfo() << "Error when read TESTBASE_JS && TESTAPI_JS";
        return;
    }
    QJSValue eval = m_jsEngine->evaluate(testBaseJs + testApiJs);
    if (eval.isError()) {
        qDebug() << "Error!"
                 << eval.property("name").toString() << ", "
                 << eval.property("message").toString()
                 << eval.property("lineNumber").toInt();
    }
}

ScriptEngine* ScriptEngine::instance()  {
    static ScriptEngine* manager = new ScriptEngine;
    return manager;
}

QJSValue ScriptEngine::runScript(const QString &string) {
    return m_jsEngine->evaluate(string);
}

void ScriptEngine::execTest(const QStringList &list) {
    if(list.size()>1) {
        clickButtonByAccessbleName(list.at(1));
        return;
    }
    m_interface->clickButtonByText(list);
}

// QPair<bool, QVariant> ScriptEngine::syncRunJavaScript(const QString &javascript, int msec) {
    // TODO, 本来是同步的,增加异步执行的逻辑
// }
