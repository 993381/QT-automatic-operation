#include "probe.h"
#include <QMutexLocker>
#include <QApplication>
#include <QAtomicPointer>
#include <QWindow>
#include <QObject>
#include <QThread>
#include <QMetaMethod>
#include "probeguard.h"

#include <QDebug>
#include <iostream>

#ifndef IF_DEBUG
#define IF_DEBUG(X) // X
#endif

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_lock, (QMutex::Recursive))
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_lock2, (QMutex::Recursive))

QAtomicPointer<Probe> Probe::s_instance = QAtomicPointer<Probe>(nullptr);

using namespace GammaRay;

struct Listener
{
    Listener()
        : trackDestroyed(true)
    {
    }

    bool trackDestroyed;
    QVector<QObject *> addedBeforeProbeInstance;
};
Q_GLOBAL_STATIC(Listener, s_listener)


Probe *Probe::instance()
{
    return s_instance.load();
}
void Probe::createProbe(bool findExisting)
{
    Q_ASSERT(qApp);
    Q_ASSERT(!isInitialized());
    Probe *probe = nullptr;
    {
        ProbeGuard guard;
        probe = new Probe;
    }
    // TODO: slot
    connect(qApp, SIGNAL(aboutToQuit()), probe, SLOT(shutdown()));
    connect(qApp, SIGNAL(destroyed()), probe, SLOT(shutdown()));
    {
        QMutexLocker lock(s_lock());
        // now we set the instance while holding the lock,
        // all future calls to object{Added,Removed} will
        // act directly on the data structures there instead
        // of using addedBeforeProbeInstance
        // this will only happen _after_ the object lock above is released though
        Q_ASSERT(!instance());

        s_instance = QAtomicPointer<Probe>(probe);

        // add objects to the probe that were tracked before its creation
        foreach (QObject *obj, s_listener()->addedBeforeProbeInstance) {
            objectAdded(obj);
        }
        s_listener()->addedBeforeProbeInstance.clear();

        // try to find existing objects by other means
        if (findExisting)
            probe->findExistingObjects();
    }
}

Probe::Probe(QObject *parent)
    : QObject(parent)
    , m_queueTimer(new QTimer(this))
    , m_objs()
{
}

bool Probe::isInitialized()
{
    return instance();
}

void Probe::findExistingObjects()
{
    discoverObject(QCoreApplication::instance());
    if (auto guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        foreach (auto window, guiApp->allWindows()) {
            discoverObject(window);
        }
    }
}
void Probe::discoverObject(QObject *object)
{
    if (!object)
        return;

    QMutexLocker lock(s_lock());
    if (m_validObjects.contains(object))
        return;

    objectAdded(object);
    foreach (QObject *child, object->children()) {
        discoverObject(child);
    }
}
void Probe::objectAdded(QObject *obj, bool fromCtor) {
    QMutexLocker lock(s_lock());
    // attempt to ignore objects created by GammaRay itself, especially short-lived ones
    if (fromCtor && ProbeGuard::insideProbe() && obj->thread() == QThread::currentThread()) {
        return;
    }
    if (s_listener.isDestroyed())
        return;
    if (!isInitialized()) {
        IF_DEBUG(std::clog
                 << "objectAdded Before: "
                 << hex << obj
                 << (fromCtor ? " (from ctor)" : "") << std::endl);
        s_listener()->addedBeforeProbeInstance << obj;
        return;
    } else {
        if (fromCtor) IF_DEBUG(std::clog << "new obj fromCtor: " << hex << obj << " xxx "<< (bool)s_listener()->addedBeforeProbeInstance.isEmpty() << std::endl);
    }

    //! TODO if (instance()->filterObject(obj)) // 用于递归检测

    if (instance()->m_validObjects.contains(obj)) {
        return;
    }
    // make sure we already know the parent
    if (obj->parent() && !instance()->m_validObjects.contains(obj->parent())) {
        objectAdded(obj->parent(), fromCtor);
    }
    instance()->m_validObjects << obj;
    // 对象追追踪使用 qHookData 代替信号槽的连接
    if (!fromCtor && obj->parent() && instance()->isObjectCreationQueued(obj->parent())) {
        // when a child event triggers a call to objectAdded while inside the ctor
        // the parent is already tracked but it's call to objectFullyConstructed
        // was delayed. hence we must do the same for the child for integrity
        fromCtor = true;
    }
    if (fromCtor) {
        instance()->queueCreatedObject(obj);
        // printf("queueCreatedObject: %x\n", obj);
    } else {
        instance()->objectFullyConstructed(obj);
    }
    Probe::instance()->addFocusObject(obj);
}
void Probe::objectRemoved(QObject *obj)
{
    QMutexLocker lock(s_lock());

    if (!isInitialized()) {
        IF_DEBUG(std::cout
                 << "objectRemoved Before: "
                 << hex << obj
                 << " have statics: " << s_listener() << std::endl);

        if (!s_listener())
            return;

        QVector<QObject *> &addedBefore = s_listener()->addedBeforeProbeInstance;
        for (auto it = addedBefore.begin(); it != addedBefore.end();) {
            if (*it == obj)
                it = addedBefore.erase(it);
            else
                ++it;
        }
        return;
    }

    IF_DEBUG(std::cout << "object removed:" << hex << obj << " " << obj->parent() << std::endl);

    bool success = instance()->m_validObjects.remove(obj);
    if (!success) {
        // object was not tracked by the probe, probably a gammaray object
        //! EXPENSIVE_ASSERT(!instance()->isObjectCreationQueued(obj));
        return;
    }

    instance()->purgeChangesForObject(obj);
    //! EXPENSIVE_ASSERT(!instance()->isObjectCreationQueued(obj));

    if (instance()->thread() == QThread::currentThread())
        emit instance()->objectDestroyed(obj);
    else
        instance()->queueDestroyedObject(obj);
}

void Probe::dumpObjects() {
    for (auto obj : m_validObjects) {
        qInfo() << obj->objectName() << obj->metaObject()->className();
    }
}

// pre-condition: lock is held already, our thread
void Probe::objectFullyConstructed(QObject *obj)
{
    Q_ASSERT(thread() == QThread::currentThread());

    if (!m_validObjects.contains(obj)) {
        // deleted already
        IF_DEBUG(std::cout << "stale fully constructed: " << hex << obj << std::endl);
        return;
    }

    //! TODO if (filterObject(obj))                 // 防止递归

    IF_DEBUG(std::cout << "fully constructed: " << hex << obj << std::endl);

    // ensure we know all our ancestors already
    for (QObject *parent = obj->parent(); parent; parent = parent->parent()) {
        if (!m_validObjects.contains(parent)) {
            objectAdded(parent); // will also handle any further ancestors
            break;
        }
    }
    Q_ASSERT(!obj->parent() || m_validObjects.contains(obj->parent()));

    //! TODO m_toolManager->objectAdded(obj);       // 添加元对象的入口
    emit objectCreated(obj);
}
// pre-condition: we have the lock, arbitrary thread
void Probe::queueCreatedObject(QObject *obj)
{
    //! EXPENSIVE_ASSERT(!isObjectCreationQueued(obj));

    ObjectChange c;
    c.obj = obj;
    c.type = ObjectChange::Create;
    m_queuedObjectChanges.push_back(c);
    notifyQueuedObjectChanges();
}
// pre-condition: we have the lock, arbitrary thread
void Probe::queueDestroyedObject(QObject *obj)
{
    ObjectChange c;
    c.obj = obj;
    c.type = ObjectChange::Destroy;
    m_queuedObjectChanges.push_back(c);
    notifyQueuedObjectChanges();
}
// pre-condition: we have the lock, arbitrary thread
bool Probe::isObjectCreationQueued(QObject *obj) const
{
    return std::find_if(m_queuedObjectChanges.begin(), m_queuedObjectChanges.end(),
                        [obj](const ObjectChange &c) {
        return c.obj == obj && c.type == Probe::ObjectChange::Create;
    }) != m_queuedObjectChanges.end();
}
// pre-condition: we have the lock, arbitrary thread
void Probe::purgeChangesForObject(QObject *obj)
{
    for (int i = 0; i < m_queuedObjectChanges.size(); ++i) {
        if (m_queuedObjectChanges.at(i).obj == obj
                && m_queuedObjectChanges.at(i).type == ObjectChange::Create) {
            m_queuedObjectChanges.remove(i);
            return;
        }
    }
}
// pre-condition: we have the lock, arbitrary thread
void Probe::notifyQueuedObjectChanges()
{
    if (m_queueTimer->isActive())
        return;

    if (thread() == QThread::currentThread()) {
        m_queueTimer->start();
    } else {
        static QMetaMethod m;
        if (m.methodIndex() < 0) {
            const auto idx = QTimer::staticMetaObject.indexOfMethod("start()");
            Q_ASSERT(idx >= 0);
            m = QTimer::staticMetaObject.method(idx);
            Q_ASSERT(m.methodIndex() >= 0);
        }
        m.invoke(m_queueTimer, Qt::QueuedConnection);
    }
}
QMutex *Probe::objectLock()
{
    return s_lock();
}
bool Probe::filterObject(QObject *obj) const
{
    if (obj->thread() != thread()) {
        // shortcut, never filter objects from a different thread
        return false;
    }

    QSet<QObject *> visitedObjects;
    int iteration = 0;
    QObject *o = obj;
    do {
        if (iteration > 100) {
            // Probably we have a loop in the tree. Do loop detection.
            if (visitedObjects.contains(o)) {
                std::cerr << "We detected a loop in the object tree for object " << o;
                if (!o->objectName().isEmpty())
                    std::cerr << " \"" << qPrintable(o->objectName()) << "\"";
                std::cerr << " (" << o->metaObject()->className() << ")." << std::endl;
                return true;
            }
            visitedObjects << o;
        }
        ++iteration;

        if (o == this /*|| o == window()*/)
            return true;
        o = o->parent();
    } while (o);
    return false;
}
bool Probe::isValidObject(QObject *obj) const
{
    return m_validObjects.contains(obj);
}
QSet<QObject *> *Probe::validObject() {
    return &Probe::instance()->m_validObjects;
}
void Probe::startupHookReceived()
{
    s_listener()->trackDestroyed = false;
}
void Probe::addFocusObject(QObject *obj) {
    // if (!obj->inherits("QWidget")) return;
    // if (!obj->inherits("QAbstractButton")) return;
    // qInfo() << "add object: " << obj;
    QString super = "null";
    if (obj->metaObject()->superClass())
        super = obj->metaObject()->superClass()->className();
    // printf("add object: %x %s %s\n", obj, obj->metaObject()->className(), super.toLocal8Bit().data());
    if (!(super == "QAbstractButton" || super == "QWindow" || super == "QWidget")) return;
    if (m_objs.contains(obj)) return;
    QMutexLocker lock(s_lock2());
    m_objs.append(obj);
}
void Probe::setFocusObjects(QObjectList objs) {
    m_objs = objs;
}
QObjectList Probe::focusObjects() const {
    return m_objs;
}
