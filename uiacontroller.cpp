#include "uiacontroller.h"
#include "objectlistmanager.h"
#include "operationmanager.h"
#include "objectpathresolver.h"
#include "objectpath.h"

#include "util.h"
#include "probe.h"
#include "sigspy.h"

#include <QEvent>
#include <QWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QMetaMethod>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore/private/qobject_p.h>  // qt_register_signal_spy_callbacks

class EventFilter : public QObject {
public:
    EventFilter() {}
    ~EventFilter() {}
protected:
    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEv = static_cast<QMouseEvent *>(event);
            if (mouseEv->button() == Qt::LeftButton) {
                QWidget *widget = QApplication::widgetAt(mouseEv->globalPos());
                if (widget && !ObjectListManager::instance()->isInBlackList(widget, true)) {
                    QObjectList tmpList = widget->children();
                    tmpList.append(widget);
                    Probe::instance()->setFocusObjects(tmpList);
                }
            }
        }
        return QObject::eventFilter(object, event);
    }
};

UiaController::UiaController()
    : m_filter (new EventFilter)
{
}

UiaController::~UiaController() {
}

bool UiaController::startEventMonitoring() {
    if (qApp) {
        qApp->installEventFilter(m_filter.get());
        return true;
    }
    return false;
}
void UiaController::stopEventMonitoring() {
    if (qApp) {
        qApp->removeEventFilter(m_filter.get());
    }
}

void UiaController::startSigslotMonitoring() {
    // register sigslot callback
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    cbs.signal_begin_callback = signal_begin_callback;
    qt_register_signal_spy_callbacks(cbs);
}

void UiaController::stopSigslotMonitoring() {
    // unregister sigslot callback
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
    qt_register_signal_spy_callbacks(cbs);
}

bool UiaController::startAllMonitoring() {
    bool res = startEventMonitoring();
    startSigslotMonitoring();
    return res;
}
void UiaController::stopAllMonitoring() {
    stopEventMonitoring();
    stopSigslotMonitoring();
}

bool UiaController::initOperationSequence() {
    QObject::connect(OperationManager::instance(), &OperationManager::Start, [](OperationManager::State state){
        if (state == OperationManager::State::Playing) {        // start playing
            UiaController::instance()->stopAllMonitoring();
            const QVector<ObjectPath> &paths = ObjectPathManager::instance()->loadPaths();
            qInfo() << "xxxxxxxxxxxxxxxxxxxxxxxxx " << paths.size();
            for (auto path : paths){ //ObjectPathManager::instance()->loadPaths()) {
                ObjectPathResolver resolver;
                resolver.setDiscoverCallback([path](QObject *obj) -> bool {
                    ObjectPath::NodeInfo info1 = ObjectPath::parseObjectInfo(obj);
                    if (/*info1.depth<0 || */info1.depth>path.path().size()) return false;              // 对象路径的长度应该和输入的路径长度一致
                    ObjectPath::NodeInfo info2 = path.path().at(info1.depth-1);
                    qInfo() << "------------ " << obj << " depth: " << info1.depth << (info1 == info2);
                    return info1 == info2;
                });

                resolver.findExistingObjects();
                // 从符合层级关系的对象中找出符合路径关系的对象
                QObjectList objects = resolver.objects();
                qInfo() << "objects size........... " << objects.size() << objects[0];
                // static int inteval = 0;
                for (QObject *obj : objects) {
                    ObjectPath obj_path(ObjectPath::parseObjectPath(obj));

                    if (obj_path == path) {
                        obj_path.dump();
                        path.dump();
                        // inteval++;
                        int index2 = obj->metaObject()->indexOfSignal("pressed()");
                        if (index2 < 0) continue;
                        // QTimer::singleShot(1000*inteval, [obj, index2] {
                        qInfo() << "target obj found: " << obj;
                            obj->metaObject()->method(index2).invoke(obj, Qt::ConnectionType::DirectConnection);    // 信号的调用和槽的调用几乎是一样的
                        // });
                    }
                }
            }
        }
        if (state == OperationManager::State::Recording) {      // start recording
            UiaController::instance()->startAllMonitoring();
        }
    });
    QObject::connect(OperationManager::instance(), &OperationManager::Stopped, [](OperationManager::State state){
        UiaController::instance()->stopAllMonitoring();
    });
    return true;
}

bool UiaController::createUiaWidget() {
    // add to black list.
    // UiaWidget obj;
    QWidget *widget = new QWidget(/*&obj*/);
    // qInfo() << ((QObject *)widget)->metaObject()->classInfo(0).value();
    // auto paly / next
    QPushButton *button1 = new QPushButton("record", widget);
    QPushButton *button2 = new QPushButton("play", widget);
    QPushButton *button3 = new QPushButton("save", widget);
    QPushButton *button4 = new QPushButton("exit", widget);
    QPushButton *button5 = new QPushButton("edit", widget);
    QPushButton *button6 = new QPushButton("load", widget);      // load json file
    QPushButton *button7 = new QPushButton("stop", widget);

    ObjectListManager::instance()->addToBlackList({widget});     // 只加toplevel，判断的时候用 recursive 就行

    QObject::connect(button6, &QPushButton::clicked, [](bool checked){
        QString openFileName = showFileDialog(QFileDialog::AcceptOpen);
        if (openFileName.isEmpty()) return;

        QByteArray allData;
        if (!fileReadWrite(openFileName, allData, true)) {
            qInfo() << "read error: " << openFileName;
            return;
        }

        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
        if(json_error.error != QJsonParseError::NoError) {
            qInfo() << "json parser error! " << json_error.errorString();
            return;
        }
        QJsonObject rootObj = jsonDoc.object();
        ObjectPathManager::instance()->readFromJson(rootObj);
    });
    QObject::connect(button1, &QPushButton::clicked, [](bool checked){
        OperationManager::instance()->startRecording();
    });
    QObject::connect(button2, &QPushButton::clicked, [](bool checked){
        OperationManager::instance()->startPlaying();
    });
    QObject::connect(button3, &QPushButton::clicked, [](bool checked){
        if (OperationManager::instance()->state() != OperationManager::Recording) {
            qInfo() << "不在录制状态!";
            return;
        }

        QString saveFileName = showFileDialog(QFileDialog::AcceptSave);
        if (saveFileName.isEmpty()) return;
        qInfo() << "Save json file to: " << saveFileName;

        // 录制完毕转 Json 保存
        QVector<ObjectPath> paths = ObjectPathManager::instance()->paths();
        qInfo() << "paths size: " << paths.size();
        QJsonObject json = ObjectPathManager::convertToJson(paths);
        QJsonDocument doc;
        doc.setObject(json);
        QByteArray byte = doc.toJson(QJsonDocument::Indented);
        bool res = fileReadWrite(saveFileName, byte, false);
        qInfo() << res << doc;
    });
    button2->move(100, 0);
    button3->move(200, 0);
    button4->move(300, 0);
    button5->move(400, 0);
    button6->move(500, 0);
    button7->move(600, 0);

    widget->resize(750, 100);
    widget->show();
}
