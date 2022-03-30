//#include "probe.h"
//#include "hooks.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QMainWindow>

#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QDebug>
#include <QTimer>

#include "util.h"

//#include <iostream>

//#include <QObject>

//#include <QtCore/private/qmetaobject_p.h>
//#include <QVector>
//#include <QMutexLocker>
//#include "signalspycallbackset.h"
//#include <QWindow>
//#include <QtCore/private/qhooks_p.h>

// probe 由 ProbeCreator 在 gammaray_startup_hook 里面创建

// 关于QMetaMethod：https://blog.csdn.net/zhizhengguan/article/details/115584141
//#include <QEvent>
//#include <QMouseEvent>

//#include "scopeguard.h"
//#include <QWindow>
//#include <QFileDialog>

//#include "objectpath.h"
//#include "objectpathresolver.h"
#include <QAbstractButton>
#include <QLineEdit>
#include <QTextEdit>

#include <QJsonArray>
//#include "util.h"
#include "uiacontroller.h"

#include "scriptengine/scriptengine.h"
#include "gdbinjector/gdb_injector.h"

// 添加关注的对象、类型、方法：void MetaObjectRepository::initQObjectTypes()
// void MetaObject::addBaseClass(MetaObject *baseClass) 这里面有对于baseclass inherts的判断: #define MO_ADD_BASECLASS(Base)
// 搜这个可以发现，类型的注册在不同的功能模块是不一样的：MO_ADD_METAOBJECT0\MO_ADD_METAOBJECT1
// public MetaObjectRepository

// GammaRay对信号类型的判断： isNotifySignal
// PluginInfo::isStatic
// 森林的层序遍历及求高度：https://www.freesion.com/article/5690776446/

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // QApplication::notify();   用functionOverride重写一下就能获取到所有的了。
#if 1
    QMainWindow window;
    QPushButton *button = new QPushButton("Launch", &window);
    QObject::connect(button, &QPushButton::clicked, [] {
        QByteArray testCase;
        if (fileReadWrite(TESTCASE_JS, testCase, true)) {
            auto result = ScriptEngine::instance()->runScript(testCase);
            if (result.isError()) {
                qInfo() << "error when load TESTER_JS";
            }
            ScriptEngine::instance()->runScript("TestMethod.launch();");
        }
    });

    window.show();
    return app.exec();
#endif

#if 0
    UiaController::instance()->createUiaWidget();
    UiaController::instance()->initOperationSequence();

    return app.exec();
#endif

#if 0
    QWebEngineView *webview = new QWebEngineView();
    // webview->load(QUrl("http://qt-project.org/"));
    webview->load(QUrl("http://www.baidu.com"));
    webview->show();

    QWebEnginePage *page = new QWebEnginePage;
    page->runJavaScript("'Java' + 'Script'", [](const QVariant &result){ qDebug() << result; });
    // qInfo() << page->profile()->scripts()->toList().at(0).sourceCode();

    return app.exec();
#endif

    QMainWindow w;
    w.resize(420, 380);
    // const int constexpr *const ptr = nullptr;

    QPushButton *button1 = new QPushButton("btn1", &w);             // pressed()
    QPushButton *button2 = new QPushButton("btn2", &w);             // pressed()
    QPushButton *button3 = new QPushButton("btn3", &w);             // pressed()
    QWidget *widget = new QWidget(&w);
    QPushButton *button4 = new QPushButton("btn4", widget);         // pressed()
    QLineEdit *edit1 = new QLineEdit(&w);                            // editingFinished()
    QLineEdit *edit2 = new QLineEdit(&w);                            // editingFinished()
    button2->move(100, 0);
    button3->move(200, 0);
    edit1->move(100, 30);
    edit2->move(200, 30);
    widget->resize(200, 100);
    widget->move(100, 70);

    QObject::connect(edit1, &QLineEdit::editingFinished, [edit1] {
        qInfo() << "editingFinished " << edit1->metaObject()->superClass()->className();
        // if (edit->metaObject()->superClass()) qInfo() << "editingFinished " << edit->metaObject()->superClass();
    });

    // QTimer::singleShot(1000*2, [button1]{
    //     button1->click();
    // });

    QObject::connect(button2, &QPushButton::pressed, [] {
        qInfo() << "button2 pressed.............. widget1 level1";
    });


    QStandardItemModel model;
    QListView view(&w);
    view.setModel(&model);
    // QAbstractItemModel *model2 = qobject_cast<QAbstractItemModel *>(&model);
    auto row1 = new QStandardItem("xxxxxxxxxxx");
    auto row2 = new QStandardItem("yyyyyyyyyyy");
    model.appendRow(row1);
    model.appendRow(row2);
    view.resize(120, 50);
    view.move(200,200);
    QModelIndex qindex = model.index(1, 0);   //默认选中 index
    // view.setCurrentIndex(qindex);
    // QObject obj;
    // obj.metaObject()->

    edit1->setToolTip("11111111111111111");

//    auto cb = [](QObject *target, QVariant value){
//        if (auto listview = qobject_cast<QListView *>(target/*->parent()*/)) {
//            listview->setCurrentIndex(value.toModelIndex());
//        }
//    };

    selectListItemByText("yyyyyyyyyyy");
    // qInfo() << view.objectName() << view.currentIndex().row() << view.currentIndex().column();
    // qInfo() << "mode name: " << row->metaObject()->className();

    // after attach.
    UiaController::instance()->createUiaWidget();
    UiaController::instance()->initOperationSequence();

    // QJsonArray parameterTypesArr;
    // parameterTypesArr.insert(0, 1);
    // parameterTypesArr.insert(1, "2");
    // qInfo() << parameterTypesArr;

    QObject::connect(button4, &QPushButton::pressed, [] {
        QWidget *widget = new QWidget(nullptr);
        QWidget *sub_widget = new QWidget(widget);
        QPushButton *button1 = new QPushButton("btn1", widget);             // pressed()
        QPushButton *button2 = new QPushButton("btn2", sub_widget);             // pressed()
        button2->move(0, 100);
        widget->resize(400, 300);
        widget->show();
        // QObject::connect(button2, &QPushButton::pressed, [=] {
        //     QObjectList btns = findObjects(ByButtonText, "btn1");
        //     qInfo() << "button2 pressed ............................... widget2 level1";
        //     for (auto btn : btns) {
        //          ((QAbstractButton *)btn)->click();
        //     }
        //     Q_ASSERT(btns[0] == button1);
        //     Q_ASSERT(btns[1] == button2);
        // });
        QObject::connect(button1, &QPushButton::pressed, [] {
            qInfo() << "button1 pressed ............................... widget2 level1";
        });
        QObject::connect(button2, &QPushButton::pressed, [] {
            qInfo() << "button2 pressed ............................... widget2 level1";
        });
    });

#if 0
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
#endif

    w.show();
    return app.exec();
}


#if 0
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




