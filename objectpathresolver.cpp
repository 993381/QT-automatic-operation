#include "objectpathresolver.h"
#include "scopeguard.h"

#include <QMutex>
#include <QWindow>
#include <QWidget>
#include <QApplication>
#include <QGuiApplication>

#include <QDebug>

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_lock, (QMutex::Recursive))

ObjectPathResolver::ObjectPathResolver() : m_layer_count (0) {}
ObjectPathResolver::~ObjectPathResolver() {}
void ObjectPathResolver::findExistingObjects()
{
    m_validObjects.clear();
    m_objs.clear();
    // discoverObject(QCoreApplication::instance());
    // if (auto guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
    if (qApp) {
        foreach (auto window, qApp->topLevelWidgets()) {        //! foreach (auto window, guiApp->allWindows()) {
            discoverObject(window);
        }
    }
}
// 最好的方法：通过栈的深度得到树的高度，检验对象类型，决定是否保存
void ObjectPathResolver::discoverObject(QObject *object)
{
    ++m_layer_count;
    auto guard = qScopeGuard([this]{
        --m_layer_count;
    });
    if (!object)
        return;

    QMutexLocker lock(s_lock());
    if (m_validObjects.contains(object))
        return;

    if (m_callback && m_callback(object)) {     // 只关心关系链之中的信息，depth、index、classname对的上，就把它存进去
        if (!m_objs.contains(object))
            m_objs.push_back(object);
        // qInfo() << "object: " << object << " m_layer_count: " << m_layer_count;
    }
    m_validObjects.push_back(object);
    // qInfo() << "object: " << object << " m_layer_count: " << m_layer_count;
    foreach (QObject *child, object->children()) {
        discoverObject(child);
    }
}

//! TODO 把这个作为 findExistingObjects 的参数传入
void ObjectPathResolver::setDiscoverCallback(std::function<bool(QObject *)> callback) {
    m_callback = callback;
}

// void setPathFilter(const ObjectPath &path) {
//     m_obj_path = path;
// }

QObjectList ObjectPathResolver::objects() const {
    return m_objs;
}
