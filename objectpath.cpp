#include "objectpath.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

ObjectPath::NodeInfo::NodeInfo(const ObjectPath::NodeInfo &other) {
    type = other.type;
    index = other.index;
    depth = other.depth;
    className = other.className;
}
bool ObjectPath::NodeInfo::operator==(const NodeInfo &other) {
    return className == other.className
            && index == other.index
            && depth == other.depth
            && type == other.type; // type 不太重要，因为不一定准确
}
int ObjectPath::getSiblingIndex(QObject *obj) {
    int index = -1;
    QObject *parent = obj->parent();
    if (parent) {
        QObjectList children =  parent->children();
        std::list<QObject *> list = children.toStdList();
        auto shouldErease = [obj, parent](QObject *child){
            if (child == obj) {
                return false;               // child 里面包含了 obj 自己, 不能被误删
            }
            bool isSameType = child->metaObject()->className() == obj->metaObject()->className();
            bool isDirectChild = child->parent() == parent;
            return !(isSameType && isDirectChild);
        };
        list.erase(std::remove_if(list.begin(), list.end(), shouldErease), list.end());
        index = QList<QObject *>::fromStdList(list).indexOf(obj);
    }
    return index;
}

int ObjectPath::getLayerCount(QObject *root_obj) {
    if (!root_obj) {
        return 0;
    }
    int count = 0;
    QObject *obj = root_obj;
    do {
        ++count;
        obj = obj->parent();
    } while (obj);
    return count;
}

ObjectPath::NodeInfo ObjectPath::parseObjectInfo(QObject *object, ObjectPath::NodeType targetType) {
    NodeInfo node;
    node.depth = getLayerCount(object);
    node.index = getSiblingIndex(object);
    node.className = object->metaObject()->className();
    node.type = targetType;

    return node;
}

// 解析节点的 path， 包含 index、depth、type
// return: topLevel parent_n ... parent_2 parent_1 target
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

    // getRelationship 0~n: target parent_1 parent_2 ... parent_n topLevel
    QObjectList path = getRelationship(object);
    QVector<NodeInfo> result;
    result.reserve(path.size());
    for (int i = 0; i < path.size(); ++i) {
        NodeInfo info;
        info.depth = getLayerCount(path.at(i));
        info.index = getSiblingIndex(path.at(i));
        info.className = path.at(i)->metaObject()->className();
        if (i == 0) {
            info.type = targetType;
        } else if (i < path.size() - 1) {
            info.type = ObjectPath::NodeType::Parent;
        } else if (i == path.size() - 1) {
            info.type = ObjectPath::NodeType::Top;         // 最顶级的是准确的，最底层的默认的
        }
        result.prepend(info);                               // 原来是反的，逆置一下
    }
    ObjectPath paths(result);
    qInfo() <<"ObjectPath::parseObjectPath ---------------------- 1";
    paths.dump();
    qInfo() <<"ObjectPath::parseObjectPath ---------------------- 2";
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
void ObjectPath::setRecordMethod(const QString &method) {
    m_arg.recordMethod = method;
}
void ObjectPath::setPlayMethod(const QString &method) {
    m_arg.playMethod = method;
}

QJsonObject ObjectPathManager::convertToJson(const QVector<ObjectPath> &paths) {
    QJsonObject sop;
    qInfo() << "path size ************: " << paths.size();
    for (int i = 0; i < paths.size(); ++i) {
        QJsonObject jsonPath;
        QVector<ObjectPath::NodeInfo> nodePath = paths[i].path();
        for (int j = 0; j < nodePath.size(); ++j) {
            QJsonObject nodeData;
            nodeData["type"] = (int)nodePath[j].type;
            nodeData["index"] = nodePath[j].index;
            nodeData["className"] = nodePath[j].className;
            nodeData["depth"] = nodePath[j].depth;
            QString nodeName;
            QJsonObject targetOption;
            if (j == 0) {
                nodeName = "topLevel";
            } else if (j == (nodePath.size() - 1)) {
                nodeName = "target";

                // QJsonArray 当参数类型不同的时候，还能这么存吗？
                ObjectPath::Arguments args = paths[i].parameters();
                if (args.parameterCount) {
                    QJsonArray parameterTypesArr, parameterValuesArr;
                    for (int i = 0; i < args.parameterCount; ++i) {
                        parameterTypesArr.insert(i, args.parameterTypes.at(i));
                        parameterValuesArr.insert(i, args.parameterValues.at(i).toJsonValue());
                    }
                    targetOption["uniqIndex"] = args.uniqIndex;
                    targetOption["playMethod"] = args.playMethod;
                    targetOption["recordMethod"] = args.recordMethod;
                    targetOption["discoverDesc"] = args.discoverDesc;
                    targetOption["parameterCount"] = args.parameterCount;
                    targetOption["parameterTypes"] =  parameterTypesArr;
                    targetOption["parameterValues"] =  parameterValuesArr;
                }
            } else {
                nodeName = QString("parents_%1").arg(j);
            }
            jsonPath[nodeName] = nodeData;
            if (!targetOption.isEmpty())
                jsonPath["target_option"] = targetOption;
        }
        sop[QString("path_%1").arg(i+1)] = jsonPath;
    }
    return sop;
};
bool ObjectPathManager::readFromJson(const QJsonObject &rootObj) {
    m_loadPaths.clear();
    for (int i = 0; i < rootObj.size(); ++i) {
        // 解析 root 中的 path
        QString pathName = QString("path_%1").arg(i+1);
        if (!rootObj.keys().contains(pathName)) {
            continue;
        }
        QJsonValue value = rootObj[pathName];
        if (!value.isObject()) {
            qInfo() <<"error, unexpected json value: " << value;
            continue;
        }

        // 解析 path 中的 node
        QJsonObject nodes = value.toObject();
        QStringList node_keys = nodes.keys();
        if (!node_keys.contains("topLevel")) {
            qInfo() <<"fatal error, cannot find [topLevel]: " << pathName;
            continue;
        }
        if (!node_keys.contains("target")) {
            qInfo() <<"fatal error, cannot find [target]: " << pathName;
            continue;
        }

        // 只剩下确定的节点名称
        std::list<QString> node_list = node_keys.toStdList();
        node_list.erase(std::remove_if(node_list.begin(), node_list.end(), [](const QString &key){
                            return !(key == "target" || key == "topLevel" || key.startsWith("parents_"));
                        }),  node_list.end());

        QVector<ObjectPath::NodeInfo> nodePath(node_list.size());
        bool filled = false;
        for (QString &s : node_list) {
            // qInfo() << "-------: " << s << "  " << node_list.size(); // continue;

            int idx = -1;
            if (s == "topLevel") {
                idx = 0;
            } else if (s == "target") {
                idx = node_list.size() - 1;
            } else if (s.startsWith("parents_")) {
                static const QByteArray parents(QByteArrayLiteral("parents_"));
                idx = s.mid(parents.length()).toInt();
            } else {
                continue;
            }
            // qInfo() << "-------idx: " << idx;

            if (!(idx > -1 && idx < node_list.size())) {
                qInfo() <<"fatal error, idx invalid: " << pathName << " " << s << idx;
                continue;
            }
            QJsonObject obj_node = nodes.take(s).toObject();
            nodePath[idx].type = (ObjectPath::NodeType)obj_node["type"].toInt();
            nodePath[idx].index = obj_node["index"].toInt();
            nodePath[idx].className = obj_node["className"].toString();
            nodePath[idx].depth = obj_node["depth"].toInt();
            // qInfo() << "from json!!! " << s << " type: " << nodePath[idx].type << " index: " << nodePath[idx].index << " className: " << nodePath[idx].className << " depth: " << nodePath[idx].depth;
            filled = true;
        }
        if (!filled) {
            return false;
        }
        ObjectPath paths;
        paths.setPath(nodePath);
        if (node_keys.contains("target_option")) {
            ObjectPath::Arguments args;
            QJsonValue target_option = nodes.take("target_option");
            qInfo() << "obj_node xxxxxxxxxxx: " << target_option;
            args.uniqIndex = target_option["uniqIndex"].toInt();
            args.playMethod = target_option["playMethod"].toString();
            args.recordMethod = target_option["recordMethod"].toString();
            args.discoverDesc = target_option["discoverDesc"].toString();
            args.parameterCount = target_option["parameterCount"].toInt();
            for (int i = 0; i < args.parameterCount; ++i) {
                args.parameterTypes.push_back(target_option["parameterTypes"].toArray().takeAt(i).toString());
                args.parameterValues.push_back(target_option["parameterValues"].toArray().takeAt(i).toVariant());
            }
            // qInfo() << ".............+ " << args.parameterCount << args.playMethod << args.parameterTypes << args.parameterValues;
            paths.setParameters(args);
        }
        for (int idx = 0; idx < nodePath.size(); ++idx) {
            qInfo() << "from json!!! " << " type: " << nodePath[idx].type << " index: " << nodePath[idx].index << " className: " << nodePath[idx].className << " depth: " << nodePath[idx].depth;
        }
        m_loadPaths.push_back(paths); // path_1~path_n
    }
    return true;
}

