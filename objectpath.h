#ifndef OBJECTPATH_H
#define OBJECTPATH_H
#include <QObject>
#include <QString>
#include <QVector>

#include <QDebug>

class ObjectPath {
public:
    ObjectPath() {}
    ~ObjectPath() {}
    enum NodeType { Unknow, Top, Parent, Taget };
    class NodeInfo {
    public:
        NodeInfo() {}
        ~NodeInfo() {}
        NodeInfo(const NodeInfo &other);
        NodeType type;
        QString className;
        // QString parent_type;     // 放个metaMethod在这里替换一些信息
        int index;                  // index_in_childrens
        int depth;                  // depth_in_ancestors

        bool operator==(const NodeInfo &other);
    };

    explicit ObjectPath(const QVector<NodeInfo>&path) { //! default method   // 这里的explicit就很重要
        m_path = path;
    }

    // 只对比了 path，method 需要单独处理
    bool operator==(const ObjectPath &other) {
        bool equal = true;
        QVector<ObjectPath::NodeInfo> other_path = other.path();
        if (other_path.size() != m_path.size()) {
            return false;
        }
        for (int i = 0 ; i < other_path.size(); ++i) {
            equal &= (other_path[i] == m_path[i]);  // 也是被重载的
        }
        return equal;
    }

    static int getSiblingIndex(QObject *obj);

    static NodeInfo parseObjectInfo(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget);

    // 解析节点的 path， 包含 index、depth、type
    static QVector<NodeInfo> parseObjectPath(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget);

    void setPath(const QVector<NodeInfo> &path);        //! setSignatuare
    QVector<NodeInfo> path() const;
    void setMethod(const QString &method);
    QString method() const;

    void dump () {
        for (NodeInfo &node : m_path) {
            qInfo() << "dump "<< this << " type: " << node.type << " index: " << node.index << " className: " << node.className << " depth: " << node.depth;
        }
    }

//public:
private:
    QString m_method;                 // signal or slot，最后要调用的方法
    QVector<NodeInfo> m_path;
};

// 可以用 Json 的填充，单纯只保留 ObjectPath 的信息在里面
class ObjectPathManager {
    Q_DISABLE_COPY(ObjectPathManager)
    ObjectPathManager() {}
    ~ObjectPathManager() {}

public:
    static ObjectPathManager* instance()  {
        static ObjectPathManager* manager = new ObjectPathManager;
        return manager;
    }
    void append(const ObjectPath &path) {
        instance()->m_paths.append(path);
    }
    QVector<ObjectPath> paths() const {
        return m_paths;
    }
    //! at(int index)  // layer
    //! initFromObjects(QObjectList) // method?  -> override append(object, method)
    static QJsonObject convertToJson(const QVector<ObjectPath> &paths);
    bool readFromJson(const QJsonObject &rootObj);
    QVector<ObjectPath> loadPaths() const {
        return m_loadPaths;
    }
    //! writeToJson(jsonFile)
    //! next() { return ObjectPath }
    //! hasNext

private:
    QVector<ObjectPath> m_paths;
    QVector<ObjectPath> m_loadPaths;
};

#endif//OBJECTPATH_H
