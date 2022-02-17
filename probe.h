#ifndef PROBE_HPP
#define PROBE_HPP

#include <QSet>
#include <QTimer>
#include <QObject>
#include <QVector>
#include <QMutex>
#include <QPointer>

class Probe : public QObject {
    Q_OBJECT
public:
    static Probe * instance();
    static void createProbe(bool findExisting);
    explicit Probe(QObject *parent = nullptr);
    static QAtomicPointer<Probe> s_instance;
    ~Probe() {}
    static bool isInitialized();

    void findExistingObjects();
    void discoverObject(QObject *object);
    static void objectAdded(QObject *obj, bool fromCtor = false);
    static void objectRemoved(QObject *obj);
    void dumpObjects();

    // for processQueuedObjectChanges
    void objectFullyConstructed(QObject *obj);
    void queueCreatedObject(QObject *obj);
    void queueDestroyedObject(QObject *obj);
    bool isObjectCreationQueued(QObject *obj) const;
    void purgeChangesForObject(QObject *obj);
    void notifyQueuedObjectChanges();

    // for hooks
    static QMutex *objectLock();
    bool filterObject(QObject *obj) const;
    bool isValidObject(QObject *obj) const;
    static QSet<QObject *> *validObject();
    static void startupHookReceived();

    // for sigslot filter
    void addFocusObject(QObject *obj);
    void setFocusObjects(QObjectList objs);
    QObjectList focusObjects() const;

Q_SIGNALS:
    void objectCreated(QObject *obj);
    void objectDestroyed(QObject *obj);

private:
    QSet<QObject *> m_validObjects;
    struct ObjectChange {
        QObject *obj;
        enum Type {
            Create,
            Destroy
        } type;
    };
    QTimer *m_queueTimer;
    QVector<ObjectChange> m_queuedObjectChanges;    // TODO: 排查哪些地方用到了
    QObjectList m_objs;
};

#endif//PROBE_HPP
