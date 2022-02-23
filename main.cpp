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

#include "objectpath.h"
#include "objectpathresolver.h"

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
                    // QList<QAbstractButton *> btns = qFindChildren<QAbstractButton *>(widget/*->parentWidget()*/);
                    // int index = btns.indexOf((QAbstractButton *)widget);
                    // qInfo() << "widget: " << index << widget->metaObject()->className();
                    QObjectList tmpList = widget->children();       // Probe::instance()->focusObject()->children();
                    tmpList.append(widget);
                    Probe::instance()->setFocusObjects(tmpList);
                }
            }
        }
        return QObject::eventFilter(object, event);
    }
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

    QObject::connect(button4, &QPushButton::pressed, []{
        QWidget *widget = new QWidget(nullptr);
        QWidget *sub_widget = new QWidget(widget);
        QPushButton *button1 = new QPushButton("btn1", widget);             // pressed()
        QPushButton *button2 = new QPushButton("btn2", sub_widget);             // pressed()
        button2->move(0, 100);
        widget->resize(400, 300);
        widget->show();
        QObject::connect(button2, &QPushButton::pressed, []{
            qInfo() << "button2 pressed ...............................";
            exit(0);

            // 录制完毕转 Json 保存
#if 0
sop : {
    path_1 : {
        top : {             // top level
            type : xxx,
            index : xxxx,
            className : xxxx,
            depth : xxx
        },
        parents_n : {
            type : xxxx,
            index : xxxx,
            className : xxxx,
            depth : xxxx
        },
        target : {
            type : xxxx,
            index : xxxx,
            className : xxxx,
            depth : xxxx
        }
    },
    path_2 : {
    },
    path_3 : {
    }
}

#endif
            auto constructJsonObject = [](const QVector<ObjectPath> &paths){
                QJsonObject sop;
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
                        if (j == 0) {
                            nodeName = "target";
                        } else if (j == paths.size() - 1) {
                            nodeName = "topLevel";
                        } else {
                            nodeName = QString("parents_%1").arg(j-1);
                        }
                        jsonPath.insert(nodeName, nodeData);
                    }
                    sop[QString("path_%1").arg(i+1)] = jsonPath;
                }
                return sop;
            };
            QVector<ObjectPath> paths = ObjectPathManager::instance()->paths();
            qInfo() << "paths size: " << paths.size();
            QJsonObject json = constructJsonObject(paths);
            QJsonDocument doc;
            doc.setObject(json);
            QByteArray byte = doc.toJson(QJsonDocument::Indented);
            auto fileReadWrite = [](const QString &fileName, QByteArray &data, bool isRead) {
                QFile file(fileName);
                if (isRead && !file.exists())         // 文件不存在
                    return false;
                QIODevice::OpenMode mode = isRead ? QIODevice::ReadOnly : QIODevice::WriteOnly;
                if (!file.open(mode | QIODevice::Text))
                    return false;
                if (isRead) {
                    data = file.readAll();
                } else {
                    file.write(data, data.length());
                }
                file.close();
                return true;
            };
            bool res = fileReadWrite("/home/alex/Desktop/gamademo/record.json", byte, false);
            qInfo() << res << doc;
        });
    });
    QTimer::singleShot(1000*1, []{
        qInfo() << "singleShot 3s ------------------------------------";
#if 1
        ObjectPath path1;
        ObjectPath::NodeInfo node;
        node.depth = 1;
        node.className = "QMainWindow";
        node.index = -1;
        path1.m_path.push_back(node);

        node.depth = 2;
        node.className = "QWidget";
        node.index = 4;
        path1.m_path.push_back(node);

        node.depth = 3;
        node.className = "QPushButton";
        node.index = 0;
        path1.m_path.push_back(node);

        ObjectPathResolver resolver;
        resolver.setDiscoverCallback([path1](QObject *obj) -> bool {


            ObjectPath::NodeInfo info1 = ObjectPath::parseObjectInfo(obj);
            if (/*info1.depth<0 || */info1.depth>path1.path().size()) return false;              // 对象路径的长度应该和输入的路径长度一致
            ObjectPath::NodeInfo info2 = path1.path().at(info1.depth-1);
            qInfo() << "------------ " << obj << " depth: " << info1.depth << (info1 == info2);
            return info1 == info2;
        });

        resolver.findExistingObjects();
        // 从符合层级关系的对象中找出符合路径关系的对象
        QObjectList objects = resolver.objects();
        qInfo() << "objects size........... " << objects.size() << objects[0];
        for (QObject *obj : objects) {
            ObjectPath path(ObjectPath::parseObjectPath(obj));
            if (path == path1) {
                int index2 = obj->metaObject()->indexOfSignal("pressed()");
                if (index2 < 0) continue;
                obj->metaObject()->method(index2).invoke(obj, Qt::ConnectionType::AutoConnection);    // 信号的调用和槽的调用几乎是一样的
            }
        }
#endif
    });

    QTimer::singleShot(1000*2, []{
        qInfo() << "singleShot 5s ------------------------------------";
#if 1
        ObjectPathResolver resolver2;    // 改名：ObjectPathResolver. 一个resolver对应一个path。

        ObjectPath path2;
        ObjectPath::NodeInfo node2;

        node2.depth = 1;
        node2.className = "QWidget";
        node2.index = -1;
        path2.m_path.push_back(node2);

        node2.depth = 2;
        node2.className = "QWidget";
        node2.index = 0;
        path2.m_path.push_back(node2);

        node2.depth = 3;
        node2.className = "QPushButton";
        node2.index = 0;
        path2.m_path.push_back(node2);
        resolver2.setDiscoverCallback([path2](QObject *obj) -> bool {
            ObjectPath::NodeInfo info1 = ObjectPath::parseObjectInfo(obj);
            if (/*info1.depth<0 ||*/ info1.depth>path2.path().size()) return false;              // 对象路径的长度应该和输入的路径长度一致
            ObjectPath::NodeInfo info2 = path2.path().at(info1.depth-1);
            qInfo() << "------------ " << obj << " depth: " << info1.depth << (info1 == info2);
            return info1 == info2;
        });

        resolver2.findExistingObjects();
        QObjectList objects = resolver2.objects();
        qInfo() << "objects size........... " << objects.size() << objects[0];
        for (QObject *obj : objects) {
            ObjectPath path(ObjectPath::parseObjectPath(obj));
            if (path == path2) {
                qInfo() << "path equal...........";
                int index2 = obj->metaObject()->indexOfSignal("pressed()");
                if (index2 < 0) continue;
                obj->metaObject()->method(index2).invoke(obj, Qt::ConnectionType::AutoConnection);    // 信号的调用和槽的调用几乎是一样的
            }
        }

#endif
    });

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

