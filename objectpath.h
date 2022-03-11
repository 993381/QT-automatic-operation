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
//    ObjectPath(const ObjectPath &other) {
//        m_arg = other.m_arg;
//        m_path = other.m_path;
//    }

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
    static int getLayerCount(QObject *obj);

    static NodeInfo parseObjectInfo(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget);

    // 解析节点的 path， 包含 index、depth、type
    static QVector<NodeInfo> parseObjectPath(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget);

    void setPath(const QVector<NodeInfo> &path);        //! setSignatuare
    QVector<NodeInfo> path() const;

    class Arguments {
    public:
        Arguments()
            : parameterCount (0)
        {}
        ~Arguments() {}
        int parameterCount;
        QStringList parameterTypes;
        QVariantList parameterValues;
        QString recordMethod;           // 录制的时候被调用的方法
        QString playMethod;             // 播放的时候要调用的方法
        QString discoverDesc;           // 找到改对象要调用的方法
        int uniqIndex;
    };
    void setRecordMethod(const QString &method);
    void setPlayMethod(const QString &method);

    void setParameters(int parameterCount, QStringList parameterTypes, QVariantList parameterValues, QString playMethod, QString discoverDesc, int uniqIndex = -1) {
        m_arg.parameterCount = parameterCount;
        m_arg.parameterTypes = parameterTypes;
        m_arg.parameterValues = parameterValues;
        m_arg.discoverDesc = discoverDesc;
        m_arg.playMethod = playMethod;
        m_arg.uniqIndex = uniqIndex;
    }

    void setParameters(const Arguments &args) {
        m_arg = args;
    }
    Arguments parameters() const {
        return m_arg;
    }

    void dump () {
        for (NodeInfo &node : m_path) {
            qInfo() << "dump "<< this << " type: " << node.type << " index: " << node.index << " className: " << node.className << " depth: " << node.depth;
        }
        if (m_arg.parameterCount) qInfo() << "___________" << m_arg.parameterCount << m_arg.parameterTypes << m_arg.parameterValues << m_arg.playMethod;
    }

//public:
private:
    Arguments m_arg;
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
