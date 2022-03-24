#include "uiacontroller.h"
#include "objectlistmanager.h"
#include "operationmanager.h"
#include "objectpathresolver.h"
#include "objectpath.h"
#include "scopeguard.h"
#include "gdbinjector/gdb_injector.h"
#include "scriptengine/scriptengine.h"
#include "util.h"
#include "probe.h"
#include "sigspy.h"

#include <QEvent>
#include <QWidget>
#include <QListView>
#include <QMouseEvent>
#include <QPushButton>
#include <QLineEdit>
#include <QMetaMethod>
#include <QJsonObject>
#include <QSizePolicy>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QtCore/private/qobject_p.h>  // qt_register_signal_spy_callbacks

class EventFilter : public QObject {
public:
    EventFilter() {}
    ~EventFilter() {}
protected:
    bool eventFilter(QObject *object, QEvent *event) override
    {
        // 在黑名单中的将不会加入到focus列表
        if (ObjectListManager::instance()->isInBlackList(object, true)) {
            return false; // QObject::eventFilter(object, event);
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEv = static_cast<QMouseEvent *>(event);
            if (mouseEv->button() == Qt::LeftButton) {
                QObject *target = nullptr;
                QWidget *widget = QApplication::widgetAt(mouseEv->globalPos());
                if (widget) {
                    qInfo() << "widget: " << widget
                            << " object name: " << widget->objectName()
                            << " class name: " << widget->metaObject()->className()
                            << " tooltipText: " << widget->toolTip()
                            << " accessibleName: " << widget->accessibleName();

                    target = widget;
                    if (widget->objectName() == "qt_scrollarea_viewport") {
                        if (auto listview = qobject_cast<QListView *>(widget->parent())) {
                            qInfo() << "is list view ........... " << listview->model()->data(listview->indexAt(mouseEv->pos())).toString();
                            target = listview;
                        }
                    }
                    if (auto button = qobject_cast<QAbstractButton *>(widget)) {
                        qInfo() << "eventFilter, is QAbstractButton ........... " << button->metaObject()->className() << " text: " << button->text();
                    }
                    if (auto listView = qobject_cast<QAbstractItemView *>(widget)) {
                        qInfo() << "eventFilter, is QListView ........... " << listView->metaObject()->className();
                    }
                    if (auto lineEdit = qobject_cast<QLineEdit *>(widget)) {
                        qInfo() << "eventFilter, is QLineEdit ........... " << lineEdit->metaObject()->className();
                    }
                    QObjectList tmpList = target->children();
                    tmpList.append(target);
                    Probe::instance()->setFocusObjects(tmpList);
                }
            }
        }
        return QObject::eventFilter(object, event);
    }
};

UiaController::UiaController()
    : m_filter (new EventFilter)
    , m_itr (m_paths.begin())
{
}

UiaController::~UiaController() {
}

bool UiaController::attachApp(int pid) {
}

bool UiaController::startApp(const QStringList &programAndArgs) {
    GdbInjector::instance()->launchPreload(programAndArgs);
    // GdbInjector::instance()->launchInject(programAndArgs);
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
            UiaController::instance()->m_paths.clear();
            const QVector<ObjectPath> &paths = ObjectPathManager::instance()->loadPaths();
            instance()->m_paths = paths;
            instance()->m_itr = UiaController::instance()->m_paths.begin();

#if 0
            qInfo() << "xxxxxxxxxxxxxxxxxxxxxxxxx " << paths.size();
            for (auto path : paths) {                           // ObjectPathManager::instance()->loadPaths()) {
                if (path.parameters().parameterCount) {
                    const int &uniq_index = path.parameters().uniqIndex;
                    const QString &method = path.parameters().playMethod;
                    const QString &text = path.parameters().parameterValues.first().toString();
                    qInfo() << "method selectListItemByText: "
                            << method << " uniq_index: " << uniq_index
                            << path.parameters().parameterCount
                            << path.parameters().parameterValues.first();
                    if (uniq_index == -1) {
                         qInfo() << "uniq_index error";
                         exit(0);
                    }
                    static int inteval = 0;
                    if (method == "selectListItemByText") {
                        QTimer::singleShot(1000 * inteval, [path, text, uniq_index] {
                            // qInfo() << "selectListItemByText: " << text;
                            selectListItemByText(text, uniq_index == -1? 0 : uniq_index);
                        });
                        continue;
                    }
                    inteval+=5;
                }


#if 0
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

                        int index2 = obj->metaObject()->indexOfSignal("pressed()");
                        if (index2 < 0) continue;
                        QTimer::singleShot(1000*inteval, [path, obj, index2] {
                            qInfo() << "target obj found: " << obj;
                            // obj->metaObject()->method(index2).invoke(obj, Qt::ConnectionType::DirectConnection);    // 信号的调用和槽的调用几乎是一样的
                        });
                        // inteval+=5;
                    }
                }
#endif
            }
#endif
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

bool UiaController::nextStep() {
    if (instance()->m_itr == instance()->m_paths.end()) {
        qInfo() << "UiaController::nextStep end";
        return false;
    }
    auto guard = qScopeGuard([]{
        instance()->m_itr++;
    });
    if (instance()->m_itr->parameters().parameterCount) {
        const int &uniq_index = instance()->m_itr->parameters().uniqIndex;
        const QString &playMethod = instance()->m_itr->parameters().playMethod;
        const QString &recordMethod = instance()->m_itr->parameters().recordMethod;
        const QString &discoverDesc = instance()->m_itr->parameters().discoverDesc;
        const QString &text = instance()->m_itr->parameters().parameterValues.first().toString();

        qInfo() << " playMethod: " << playMethod
                << " recordMethod: " << recordMethod
                << " discoverDesc: " << discoverDesc
                << " uniq_index: " << uniq_index
                << instance()->m_itr->parameters().parameterCount
                << instance()->m_itr->parameters().parameterValues.first();
        if (uniq_index == -1) {
             qInfo() << "uniq_index error: " << -1;
             // exit(0);
        }
        if (discoverDesc == "FindListItemByText") {
            const QString &text = instance()->m_itr->parameters().parameterValues.first().toString();
            return selectListItemByText(text, uniq_index == -1? 0 : uniq_index);
        }
        if (discoverDesc == "FindListItemByIndex") {
            const int &row = instance()->m_itr->parameters().parameterValues.at(1).toInt();
            const int &coloum = instance()->m_itr->parameters().parameterValues.at(2).toInt();
            return selectListItemByIndex(text, uniq_index == -1? 0 : uniq_index, row, coloum);
        }

        if (discoverDesc == "FindButtonByObjectName") {
            return clickButtonByObjectName(text, uniq_index == -1? 0 : uniq_index);
        }
        if (discoverDesc == "FindButtonByButtonText") {
            return clickButtonByButtonText(text, uniq_index == -1? 0 : uniq_index);
        }
        if (discoverDesc == "FindButtonByToolTip") {
            return clickButtonByToolTip(text, uniq_index == -1 ? 0 : uniq_index);
        }
        if (discoverDesc == "FindButtonByAccessibleName") {
            return clickButtonByAccessbleName(text, uniq_index == -1 ? 0 : uniq_index);
        }
        if (discoverDesc == "FindButtonByButtonIndex") {
            return clickButtonByButtonIndex(text, uniq_index == -1 ? 0 : uniq_index);
        }

        if (discoverDesc == "FindLineEditByItemIndex") {
            const int &layer = instance()->m_itr->parameters().parameterValues.at(1).toInt();
            const int &index = instance()->m_itr->parameters().parameterValues.at(2).toInt();
            const QString &content = instance()->m_itr->parameters().parameterValues.at(3).toString();
            return setLineEditTextByItemIndex(text, content, layer, index, instance()->m_itr);
        }
    }
    return false;
}

bool UiaController::createUiaWidget() {
    // add to black list.
    QWidget *widget = new QWidget;
    ObjectListManager::instance()->addToBlackList({widget});     // 只加toplevel，判断的时候用 recursive 就行

    widget->setWindowTitle("Uia Controller");

    QHBoxLayout *layout  = new QHBoxLayout(widget);
    QPushButton *button1 = new QPushButton("record");
    QPushButton *button2 = new QPushButton("play");
    QPushButton *button3 = new QPushButton("save");
    QPushButton *button4 = new QPushButton("exit");
    QPushButton *button5 = new QPushButton("edit");
    QPushButton *button6 = new QPushButton("load");      // load json file
    QPushButton *button7 = new QPushButton("stop");
    QPushButton *button8 = new QPushButton("next");
    QPushButton *button9 = new QPushButton("launch");
    QLabel *label = new QLabel(qAppName() + " " + QString(std::to_string(getpid()).c_str()));
    label->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);

    layout->addWidget(button1);
    layout->addWidget(button2);
    layout->addWidget(button3);
    layout->addWidget(button4);
    layout->addWidget(button5);
    layout->addWidget(button6);
    layout->addWidget(button7);
    layout->addWidget(button8);
    layout->addWidget(button9);
    layout->addWidget(label);

    // for (auto window : qApp->allWidgets()) {
    //     QLabel *windowLabel = new QLabel(window->window()->topLevelWidget()->windowTitle());
    //     layout->addWidget(windowLabel);
    // }

    QObject::connect(button9, &QPushButton::clicked, [](bool checked) {
        // UiaController::instance()->startApp({"/usr/bin/dde-control-center", "-s"});

        // QObject::connect(GdbInjector::instance(), &GdbInjector::injectFinished, []{
        //     ScriptEngine::instance()->runScript("Uia.startTest();");
        // });
    });
    QObject::connect(button8, &QPushButton::clicked, [](bool checked){
        instance()->nextStep();
    });
    QObject::connect(button7, &QPushButton::clicked, [](bool checked){
        OperationManager::instance()->stop();
    });
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
    QObject::connect(button4, &QPushButton::clicked, [](bool checked){
        qApp->exit();
    });

    widget->setFixedSize(widget->sizeHint());
    widget->show();
}

