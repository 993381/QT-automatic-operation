#include "probe.h"
#include "hooks.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>

#include "sigspy.h"
#include <QDebug>
#include <iostream>

#include <QObject>
#include <QtCore/private/qobject_p.h>  // qt_register_signal_spy_callbacks

#include <QJsonObject>
#include <QJsonDocument>

//#include <QtCore/private/qmetaobject_p.h>
//#include <QVector>
//#include <QMutexLocker>
//#include "signalspycallbackset.h"
//#include <QWindow>
//#include <QtCore/private/qhooks_p.h>

// probe 由 ProbeCreator 在 gammaray_startup_hook 里面创建

// 关于QMetaMethod：https://blog.csdn.net/zhizhengguan/article/details/115584141
#include <QEvent>
#include <QMouseEvent>
#include <QMetaMethod>
#include "scopeguard.h"
#include <QWindow>

// 添加关注的对象、类型、方法：void MetaObjectRepository::initQObjectTypes()
// void MetaObject::addBaseClass(MetaObject *baseClass) 这里面有对于baseclass inherts的判断: #define MO_ADD_BASECLASS(Base)
// 搜这个可以发现，类型的注册在不同的功能模块是不一样的：MO_ADD_METAOBJECT0\MO_ADD_METAOBJECT1
// public MetaObjectRepository

// GammaRay对信号类型的判断： isNotifySignal
// PluginInfo::isStatic


// 森林的层序遍历及求高度：https://www.freesion.com/article/5690776446/


class EventFilter : public QObject {
public:
    EventFilter() {}
    ~EventFilter() {}
    QWidget *last_widget = nullptr;
protected:
    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            // qInfo() << "MouseButtonPress";
            QMouseEvent *mouseEv = static_cast<QMouseEvent *>(event);
            if (mouseEv->button() == Qt::LeftButton) {
                QWidget *widget = QApplication::widgetAt(mouseEv->globalPos());
                if (widget) {
                    QList<QAbstractButton *> btns = qFindChildren<QAbstractButton *>(widget->parentWidget());
                    int index = btns.indexOf((QAbstractButton *)widget);
                    qInfo() << "widget: " << index << widget->metaObject()->className();
                    QObjectList tmpList = widget->children();       // Probe::instance()->focusObject()->children();
                    tmpList.append(widget);
                    Probe::instance()->setFocusObjects(tmpList);
                }
#if 0
                // qInfo() << "xxxxxxxxxx " << widget->metaObject()->className();
                QWidget *top_window = widget->topLevelWidget();
                if (last_widget == top_window) return QObject::eventFilter(object, event);
                last_widget = top_window;
                if (top_window) {
                    QObjectList tmpList = top_window->children(); // Probe::instance()->focusObject()->children();
                    tmpList.append(top_window);
                    Probe::instance()->setFocusObjects(tmpList);
                }
#endif
                // QPushButton *button = qobject_cast<QPushButton *>(object);
                // if (button) {
                // }
                // if (widget) {
                //     QObjectList tmpList = object->children(); // Probe::instance()->focusObject()->children();
                //     tmpList.append(object);
                //     Probe::instance()->setFocusObjects(tmpList);
                // }
            }
        }
        return QObject::eventFilter(object, event);
    }
};

// 获取所有的顶级窗口
void discoverObjects()
{
    if (qApp) {
        foreach (QWidget *widget, qApp->topLevelWidgets())
            qInfo() << "widget: " << widget;
        // m_probe->discoverObject(widget);
    }
}

// 获取对象的父子关系链   https://doc.qt.io/archives/qq/qq03-big-brother.html
void dumpObject(QObject *obj)
{
    if (!obj) {
        std::clog << "QObject(0x0)" << std::endl;
        return;
    }

    const std::ios::fmtflags oldFlags(std::clog.flags());
    do {
        std::clog << obj->metaObject()->className() << "(" << hex << obj << ")";
        obj = obj->parent();
        if (obj)
            std::clog << " <- ";
    } while (obj);
    std::clog << std::endl;
    std::clog.flags(oldFlags);
}

// GET_VALUE_FROM_OBJ(json, "ancestors", String, ancestors);
// qInfo() << "ancestors: " << ancestors;


class ObjectPath {
public:
    ObjectPath() {}
    ObjectPath(const ObjectPath &other) {
        m_path = other.m_path;
    }
    ~ObjectPath() {}
    enum NodeType { Unknow, Top, Parent, Taget };
    class NodeInfo {
    public:
        NodeInfo() {}
        ~NodeInfo() {}
        NodeInfo(const NodeInfo &other) {
            type = other.type;
            index = other.index;
            depth = other.depth;
            className = other.className;
        }
        NodeType type;
        QString className;
        // QString parent_type;     // 放个metaMethod在这里替换一些信息
        int index;                  // index_in_childrens
        int depth;                  // depth_in_ancestors

        bool operator==(const NodeInfo &other) {
            return className == other.className
                    && index == other.index
                    && depth == other.depth;
            // && type == other.type; // type倒不是那么重要
        }
    };
    static int getSiblingIndex(QObject *obj){
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

    static NodeInfo parseObjectInfo(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget) {
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
    static QVector<NodeInfo> parseObjectPath(QObject *object, ObjectPath::NodeType targetType = ObjectPath::Taget) {
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
        auto getSiblingIndex = [](QObject *obj){
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

    void setPath(const QVector<NodeInfo> &path) {
        m_path.clear();
        for (auto node : path) {
            m_path.push_back(node);
        }
    }
    QVector<NodeInfo> path() const {
        return m_path;
    }
    void setMethod(const QString &method) {
        m_method = method;
    }
    QString method() const {
        return m_method;
    }

// private:
public:
    QString m_method;                 // signal or slot，最后要调用的方法
    QVector<NodeInfo> m_path;
};

Q_GLOBAL_STATIC_WITH_ARGS(QMutex, s_lock, (QMutex::Recursive))
class ObjectResolver {
public:
    ObjectResolver() : m_depth_count (0) {}
    ~ObjectResolver() {}
    void findExistingObjects()
    {
        // discoverObject(QCoreApplication::instance());
        if (auto guiApp = qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
            //foreach (auto window, guiApp->allWindows()) {
            foreach (auto window, qApp->topLevelWidgets()) {
                // if (window->metaObject()->className() == "QMainWindow")
                discoverObject(window);
            }
        }
    }
    // 最好的方法：通过栈的深度得到树的高度，检验对象类型，决定是否保存
    void discoverObject(QObject *object)
    {
        ++m_depth_count;
        auto guard = qScopeGuard([this]{
            --m_depth_count;
        });
        if (!object)
            return;

        QMutexLocker lock(s_lock());
        if (m_validObjects.contains(object))
            return;

        if (m_callback && m_callback(object)) {     // 只关心关系链之中的信息，depth、index、classname对的上，就把它存进去
            m_obj_path.push_back(object);
            qInfo() << "object: " << object << " m_depth_count: " << m_depth_count;
        }
        m_validObjects.push_back(object);
        // qInfo() << "object: " << object << " m_depth_count: " << m_depth_count;
        foreach (QObject *child, object->children()) {
            discoverObject(child);
        }
    }
    void setDiscoverCallback(std::function<bool(QObject *)> callback) {
        m_callback = callback;
    }
    // void setPathFilter(const ObjectPath &path) {
    //     m_obj_path = path;
    // }
    QObjectList objectPath() const {
        return m_obj_path;
    }
private:
    std::function<bool(QObject *)> m_callback;
    int m_depth_count;
    QObjectList m_obj_path;
    QObjectList m_validObjects;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    EventFilter filter;
    qApp->installEventFilter(&filter);

    QMainWindow w;
    w.resize(420, 380);
    QPushButton *button1 = new QPushButton("btn1", &w);             // pressed()
    QPushButton *button2 = new QPushButton("btn2", &w);             // pressed()
    QPushButton *button3 = new QPushButton("btn3", &w);             // pressed()
    QWidget *widget = new QWidget(&w);
    QPushButton *button4 = new QPushButton("btn4", widget);         // pressed()
    QLineEdit *edit = new QLineEdit(&w);                            // editingFinished()
    button2->move(100, 0);
    button3->move(200, 0);
    edit->move(100, 30);
    widget->resize(200, 100);
    widget->move(100, 70);
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    qt_register_signal_spy_callbacks(cbs);


    using SopNode = QList<QPair<QString, int>>;

    SopNode playback;
    playback.push_back({"QMainWindow",-1});
    playback.push_back({"QWidget",4});
    playback.push_back({"QPushButton",0});

#if 1
    // 传入符合类型的 parent object，返回对应索引的 child object
    auto getObjectFromIndex = [](QObject *obj, QString childClassName, int childIndex) {       // 应该改成：parent, type, index
        QObject *result = nullptr;
        if (obj) {
            QObjectList children =  obj->children();
            std::list<QObject *> list = children.toStdList();
            auto shouldErease = [obj, childClassName](QObject *child){
                bool isSameType = child->metaObject()->className() == childClassName;
                bool isDirectChild = child->parent() == obj;
                return isSameType && isDirectChild;
            };
            list.erase(std::remove_if(list.begin(), list.end(), shouldErease), list.end());
            result = QList<QObject *>::fromStdList(list).at(childIndex);
        }
        return result;
    };
    auto resolvingObjectsFromRelationship = [](const SopNode &relation){                // 应该改成：parent, type, index
        QObject *current = nullptr;
        for (int i = 0; i < relation.size(); ++i) {
            auto &node = relation[i];
            if (i == 0) {
                if (node.second != -1) {            // 说明应该是toplevel
                    qInfo() <<"Error";
                }
                for (auto w : qApp->topLevelWidgets()) {
                    if (w->metaObject()->className() != node.first) {
                        continue;
                    }
                    current = w;
                }
            } else if (i < relation.size()-1) {     // 此时是parent中的某一个
                if (current->parent()) {
                    QObjectList children = current->parent()->children();
                    int index = ObjectPath::getSiblingIndex(current);
                }
            } else if (i == relation.size()-1) {    // 此时是target
                if (current->parent()) {
                    QObjectList children = current->parent()->children();
                }
            }
        }
    };
#endif

    // QObject::connect(button4, &QPushButton::pressed, [button1,button4, &w] {
    QTimer::singleShot(1000*3, []{
        qInfo() << "------------------------------------";
        ObjectResolver resolver;    // 改名：ObjectPathResolver

        ObjectPath path;
        ObjectPath::NodeInfo node;
        node.depth = 1;
        node.className = "QMainWindow";
        node.index = -1;
        path.m_path.push_back(node);

        node.depth = 2;
        node.className = "QWidget";
        node.index = 4;
        path.m_path.push_back(node);

        node.depth = 3;
        node.className = "QPushButton";
        node.index = 0;
        path.m_path.push_back(node);

        resolver.setDiscoverCallback([&path](QObject *obj) -> bool {
            ObjectPath::NodeInfo info1 = ObjectPath::parseObjectInfo(obj);
            if (info1.depth<0 || info1.depth>path.path().size()) return false;
            ObjectPath::NodeInfo info2 = path.path().at(info1.depth-1);
            return info1 == info2;
        });

        resolver.findExistingObjects();
        QObjectList objPath = resolver.objectPath();
        QObject *obj = objPath.last();
        int index = obj->metaObject()->indexOfSignal("pressed()");
        obj->metaObject()->method(index).invoke(obj, Qt::ConnectionType::AutoConnection); // 信号的调用和槽的调用几乎是一样的

        // (QPair("QPushButton",0), QPair("QWidget",4), QPair("QMainWindow",-1))
        // qInfo() << getRelationship(button4) << button1->metaObject()->className();
        // qInfo() << indexies;
        // indexies.clear();

#if 0
        QList<QAbstractButton *> btns = w.findChildren<QAbstractButton *>("", Qt::FindChildrenRecursively);
        QObjectList btnLists = w.children();
        qInfo() << btnLists.size();
        for (auto bt : btns) {
            qInfo() << bt->text();
        }
        // const QObjectList *list = QObject::objectTrees();

        qInfo() << "------------------------------------";
#endif

        QWidget *widget = new QWidget(nullptr);
        widget->show();
        // discoverObjects();

    });
#if 0
    一个控件的path及它在它父窗口的index决定了它的绝对位置
            找path更高效的方法：也配合每个路径上每个节点的index
            findChildByParents(type[], index[])
            node1:
    {
toplevel : {
type : 'QMainWindow',
index : 0,
        },
ancestors : {
ancestor_count : 1,
parents1 : {
type : 'QWidget',       // parents 的类型也应该加上
index : 0
            }
        },
target : {
type : "QAbstractButton",
index : 4,
signal : "pressed()",       // method
args : null
        }
    }
    node1、node2、...、node3 : sop
                        #endif
                            auto ConstructJsonObject = [](){
        QJsonObject json, nodeData, ancestors, target;
        nodeData["type"] = "QMainWindow";
        nodeData["index"] = 0;
        json.insert("toplevel", nodeData);

        nodeData["type"] = "QWidget";
        nodeData["index"] = 0;
        ancestors["parents1"] = nodeData;

        json.insert("ancestors", ancestors);

        target["type"] = "QAbstractButton";
        target["index"] = 4;
        target["signal"] = "pressed()";
        target["args"] = "null";

        json.insert("target", target);
        return json;
    };
    QJsonObject json = ConstructJsonObject();
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byte = doc.toJson(QJsonDocument::Indented);
    qInfo() << doc;

    auto PaeserJsonObject = [](QJsonObject json){
        // QString ancestors;
        // QJsonObject object;
        // // if (json["ancestors"].isObject()) {
        // QJsonValue value = json["ancestors"];
        // if (value.isObject()) {
        //     qInfo() << "isobject: " << value.toString();
        // }

        for (QString key : json.keys()) {
            if (json[key].isObject()) {
                // PaeserJsonObject(json[key]);
                qInfo() << json[key].toObject().keys();
            }
        }
    };
    PaeserJsonObject(json);
    //    QTimer::singleShot(1000*3, [button]{
    //        button->pressed();
    //    });

    w.show();

    return app.exec();
}

int main1(int argc, char *argv[]) {
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    // cbs.signal_end_callback = signal_end_callback;
    // cbs.slot_begin_callback = slot_begin_callback;
    // cbs.slot_end_callback = slot_end_callback;
    QApplication app(argc, argv);
    qt_register_signal_spy_callbacks(cbs);

    // EventFilter filter;
    // qApp->installEventFilter(&filter);

    QMainWindow w;
    w.resize(420, 380);
    QPushButton *button = new QPushButton(&w);
    qInfo() << "button: " << button;    // ->metaObject()->superClass()->className();
    QObject::connect(button, &QPushButton::pressed, [button] {
        qInfo() << "button  pressed";
        // for (int var = 0; var < button->metaObject()->methodCount(); ++var) {
        //     qInfo() << button->metaObject()->method(var).methodSignature();
        // }
    });

    // QTimer::singleShot(1000*3, [button]{
    //     button->setEnabled(false);
    // });

    w.show();

    return app.exec();
}


#if 0
int main2(int argc, char *argv[]) {
    using namespace GammaRay;
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    //    cbs.signal_end_callback = signal_end_callback;
    //    cbs.slot_begin_callback = slot_begin_callback;
    //    cbs.slot_end_callback = slot_end_callback;
    qt_register_signal_spy_callbacks(cbs);

    GammaRay::Hooks::installHooks();    // for object add/rm

    QApplication app(argc, argv);
    QMainWindow w;
    QPushButton *button = new QPushButton(&w);
    QObject::connect(button, &QPushButton::pressed, [&w] {
        qInfo() << "new widget";
        // QWidget *widget = new QWidget(nullptr);
        // qInfo() << "new widget: " << widget;
        // widget->deleteLater();
        // auto objs = Probe::instance()->validObject();
        //        for(auto obj : *objs) {
        //            qInfo() << "objectName: " << obj->objectName() << " className: " << obj->metaObject()->className();
        //        }
    });

    w.resize(420, 380);
    w.show();
    return app.exec();
}
#endif

