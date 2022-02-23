#include "objectpath.h"

ObjectPath::NodeInfo::NodeInfo(const ObjectPath::NodeInfo &other) {
    type = other.type;
    index = other.index;
    depth = other.depth;
    className = other.className;
}
bool ObjectPath::NodeInfo::operator==(const NodeInfo &other) {
    return className == other.className
            && index == other.index
            && depth == other.depth;
    // && type == other.type; // type 不太重要，因为不一定准确
}
int ObjectPath::getSiblingIndex(QObject *obj) {
    int index = -1;
    QObject *parent = obj->parent();
    if (parent) {
        QObjectList children =  parent->children();
        std::list<QObject *> list = children.toStdList();
        auto shouldErease = [obj, parent](QObject *child){
            bool isRemove = child != obj; // child 里面包含了 obj 自己, 不能被误删
            bool isSameType = child->metaObject()->className() == obj->metaObject()->className();
            bool isDirectChild = child->parent() == parent;
            return isRemove && isSameType && isDirectChild;
        };
        list.erase(std::remove_if(list.begin(), list.end(), shouldErease), list.end());
        index = QList<QObject *>::fromStdList(list).indexOf(obj);
    }
    return index;
}

ObjectPath::NodeInfo ObjectPath::parseObjectInfo(QObject *object, ObjectPath::NodeType targetType) {
    auto getDepth = [](QObject *root_obj)
    {
        if (!root_obj) {
            return 1;
        }
        int depth = 0;
        QObject *obj = root_obj;
        do {
            ++depth;
            obj = obj->parent();
        } while (obj);
        return depth;
    };

    NodeInfo node;
    node.depth = getDepth(object);
    node.index = getSiblingIndex(object);
    node.className = object->metaObject()->className();
    node.type = targetType;

    return node;
}

// 解析节点的 path， 包含 index、depth、type
QVector<ObjectPath::NodeInfo> ObjectPath::parseObjectPath(QObject *object, ObjectPath::NodeType targetType) {
    auto getRelationship = [](QObject *root_obj)
    {
        QObjectList path;
        if (!root_obj) {
            return QObjectList();
        }
        QObject *obj = root_obj;
        do {
            path << obj;
            obj = obj->parent();
        } while (obj);
        return path;
    };
    auto getSiblingIndex = [](QObject *obj)
    {
        int index = -1;
        QObject *parent = obj->parent();
        if (parent) {
            QObjectList children =  parent->children();
            std::list<QObject *> list = children.toStdList();
            auto shouldErease = [obj, parent](QObject *child){
                bool isRemove = child != obj; // child 里面包含了 obj 自己, 不能被误删
                bool isSameType = child->metaObject()->className() == obj->metaObject()->className();
                bool isDirectChild = child->parent() == parent;
                return isRemove && isSameType && isDirectChild;
            };
            list.erase(std::remove_if(list.begin(), list.end(), shouldErease), list.end());
            index = QList<QObject *>::fromStdList(list).indexOf(obj);
        }
        return index;
    };
    QVector<NodeInfo> result;
    QObjectList path = getRelationship(object);
    int depth = path.size();
    for (int i = 0; i < path.size(); ++i) {
        NodeInfo info;
        info.depth = path.size() - i;
        info.index = getSiblingIndex(path.at(i));
        info.className = path.at(i)->metaObject()->className();
        if (i == 0) {
            info.type = targetType;         // 最顶级的是准确的，最底层的默认的
        } else if (i < path.size() - 1) {
            info.type = ObjectPath::NodeType::Parent;
        } else if (i == path.size() - 1) {
            info.type = ObjectPath::NodeType::Top;
        }
        result.push_back(info);
    }
    return result;
}

void ObjectPath::setPath(const QVector<NodeInfo> &path) {
    m_path.clear();
    for (auto node : path) {
        m_path.push_back(node);
    }
}
QVector<ObjectPath::NodeInfo> ObjectPath::path() const {
    return m_path;
}
void ObjectPath::setMethod(const QString &method) {
    m_method = method;
}
QString ObjectPath::method() const {
    return m_method;
}

