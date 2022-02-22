#ifndef SOP_H
#define SOP_H
#include <QObject>
#include <QMetaObject>
#include <QAbstractButton>
#include <QWidget>
#include <QStack>

class Sop {
public:
    Sop()
        : m_index_in_sibling (-1)
        , m_current_obj (nullptr)
        , m_parents ()
    {
    }

    ~Sop() {}

    bool addObject(QObject *obj) {
        if (m_current_obj) {
            return false;
        }
        m_current_obj = obj;
        // 用 register meta type 代替，参考 void WidgetInspectorServer::registerWidgetMetaTypes()
        if (obj->metaObject()->className() == QByteArrayLiteral("QAbstractButton")) {
            QWidget *widget = qobject_cast<QWidget *>(obj);
            if (widget) {
                // QList<QAbstractButton *> sub_objs = qFindChildren<QAbstractButton *>(widget->topLevelWidget());
                QList<QAbstractButton *> sub_objs = qFindChildren<QAbstractButton *>(widget->parentWidget());
                // 1. 获取子节点索引
                m_index_in_sibling = sub_objs.indexOf((QAbstractButton *)obj);
                setRelationship();
                return true;
            }
        }
        return false;
    }
    QObject *object() {
        return m_current_obj;
    }
    QStringList relationship() const {
        return m_parents;
    }
    int indexInSibling() {
        return m_index_in_sibling;
    }

private:
    // 2. 设置继承关系
    bool setRelationship()
    {
        QStringList result;
        if (!m_current_obj) {
            return false;
        }
        QObject *obj = m_current_obj;
        do {
            result << obj->metaObject()->className();
            obj = obj->parent();
        } while (obj);
        m_parents = result;
        return true;
    }

private:
    QObject *m_current_obj;
    QStringList m_parents;          // 以上对象的继承关系. 对象的父亲是谁以及对象父亲有那些孩子，基本可以唯一确定一个对象。
    int m_index_in_sibling;         // 在 toplevel widget 里面的索引
    // add parent/toplevel
};


class SopManager {
    SopManager() {}
    ~SopManager() {}
    static SopManager *instance() {
        static SopManager *manager = new SopManager;
        return manager;
    }

    // TODO: 在这里 addObject 是不是更好
    void addSop(const Sop &sop) {
        // if (SopManager::s_sops.contains(sop)) {     // TODO, 处理重复
        //     return;
        // }
        SopManager::s_sops.append(sop);
    }

    static bool saveToJsonConfig(const QString &path) {
        // TODO
    }
    static bool openFromJsonConfig(const QString &path) {
        // TODO
    }

    static QStack<Sop> s_sops;
};

#endif//SOP_H
