#ifndef OBJECTPATH_H
#define OBJECTPATH_H
#include <QObject>
#include <QString>
#include <QVector>

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
        if (other.path().size() != m_path.size()) {
            return false;
        }
        const QVector<ObjectPath::NodeInfo> &node = other.path();
        for (int i = 0 ; i < node.size(); ++i) {
            equal &= (other.path()[i] == node[i]);  // 也是被重载的
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

public:
    // private:
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
    //! readFromJson(jsonFile)
    //! writeToJson(jsonFile)
    //! next() { return ObjectPath }
    //! hasNext

private:
    QVector<ObjectPath> m_paths;
};

#endif//OBJECTPATH_H
