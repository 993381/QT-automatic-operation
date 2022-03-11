#ifndef OBJECTLISTMANAGEMENT_H
#define OBJECTLISTMANAGEMENT_H
#include <QObject>
#include <initializer_list>
#include <QMap>

// add mutex
class ObjectListManager {
    Q_DISABLE_COPY(ObjectListManager)
    ObjectListManager() {}
    ~ObjectListManager() {}

    QMap <QString, QObjectList> m_obj_list_map;

    static constexpr char Black[] = "Black";
    static constexpr char White[] = "White";
//    static constexpr char Focus[] = "Focus";
    static constexpr char User[] = "User";

public:
    // load from automic
    static ObjectListManager* instance()  {
        static ObjectListManager* manager = new ObjectListManager;
        return manager;
    }

    // recursive true: 判断是否在名单中或者是名单中任何一个对象的子对象
    // recursive false: 判断是否在名单中
    inline void addToWhiteList(const QObjectList &objs) {
        addToList(objs, White);
    }
    inline bool isInWhiteList(QObject *obj, bool recursive = false) const {
        return isInList(obj, White, recursive);
    }
    inline void addToBlackList(const QObjectList &objs) {
        addToList(objs, Black);
    }
    inline bool isInBlackList(QObject *obj, bool recursive = false) const {
        return isInList(obj, Black, recursive);
    }
//    inline void addToFocusList(const QObjectList &objs) {
//        addToList(objs, Focus);
//    }
//    inline bool isInFocusList(QObject *obj, bool recursive = false) const {
//        return isInList(obj, Focus, recursive);
//    }
    inline void addToUserList(const QString &listName, const QObjectList &objs) {
        addToList(objs, listName);
    }
    inline bool isInUserList(QObject *obj, const QString &listName, bool recursive = false) const {
        return isInList(obj, listName, recursive);
    }

private:
    inline void addToList(const QObjectList &objs, const QString &type) {
        QObjectList &list = m_obj_list_map[type];
        for (QObject *object : objs) {
            if (list.contains(object)) {
                return;
            }
            QObject::connect(object, &QObject::destroyed, [object, &list]{
                int index = list.indexOf(object);
                if (index > 0) {
                    list.removeAt(index);
                }
            });
            list.push_back(object);
        }
    }
    inline bool isInList(QObject *obj, const QString &type, bool recursive = false) const {
        if (!obj) return false;
        const QObjectList &list = m_obj_list_map.value(type);
        if (list.contains(obj))
            return true;
        if (recursive) {
            for (QObject *o : list) {
                if (o->children().contains(obj)) {
                    return true;
                }
            }
        }
        return false;
    }
};

#endif//OBJECTLISTMANAGEMENT_H
