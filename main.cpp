#include "probe.h"
#include "hooks.h"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

#include "sigspy.h"
#include <QDebug>

#include <QtCore/private/qobject_p.h>  // qt_register_signal_spy_callbacks

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

// 添加关注的对象、类型、方法：void MetaObjectRepository::initQObjectTypes()
// void MetaObject::addBaseClass(MetaObject *baseClass) 这里面有对于baseclass inherts的判断: #define MO_ADD_BASECLASS(Base)
// 搜这个可以发现，类型的注册在不同的功能模块是不一样的：MO_ADD_METAOBJECT0\MO_ADD_METAOBJECT1
// public MetaObjectRepository

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
                // qInfo() << "xxxxxxxxxx " << widget->metaObject()->className();
                QWidget *top_window = widget->topLevelWidget();
                if (top_window == top_window) return QObject::eventFilter(object, event);
                last_widget = top_window;
                if (top_window) {
                    QObjectList tmpList = object->children(); // Probe::instance()->focusObject()->children();
                    tmpList.append(object);
                    Probe::instance()->setFocusObjects(tmpList);
                }
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

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(420, 380);
    QPushButton *button = new QPushButton(&w);
    QObject *btn = button;
    qInfo() << "------------------------------------" << button->metaObject()->methodCount();
    for (int i = 0; i < button->metaObject()->methodCount(); ++i) {
        // qInfo() << "method: " << button->metaObject()->method(i).methodSignature();
    }
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    qt_register_signal_spy_callbacks(cbs);

    QObject::connect(button, &QPushButton::pressed, [button, &w] {
        // QWidget *widget = new QWidget(nullptr);
        // widget->show();
        // qInfo() << "button  pressed";
    });

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

