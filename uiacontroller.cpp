#include "uiacontroller.h"
#include "objectlistmanager.h"
#include "operationmanager.h"
#include "objectpathresolver.h"

#if QT_VERSION > QT_VERSION_CHECK(5, 11, 3)
# include <qscopeguard.h>
#else
# include "scopeguard.h"
#endif

#include "gdbinjector/gdb_injector.h"
#include "scriptengine/scriptengine.h"
#include "util.h"
#include "probe.h"
#include "sigspy.h"
#include "steprecord.h"

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
#include <QVBoxLayout>
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


#if QT_VERSION > QT_VERSION_CHECK(5, 11, 3)
    qt_register_signal_spy_callbacks(&cbs);
#else
    qt_register_signal_spy_callbacks(cbs);
#endif
}

void UiaController::stopSigslotMonitoring() {
    // unregister sigslot callback
    QSignalSpyCallbackSet cbs = { nullptr, nullptr, nullptr, nullptr };
#if QT_VERSION > QT_VERSION_CHECK(5, 11, 3)
    qt_register_signal_spy_callbacks(&cbs);
#else
    qt_register_signal_spy_callbacks(cbs);
#endif

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
    QObject::connect(OperationManager::instance(), &OperationManager::Start, [this](OperationManager::State state){
        if (state == OperationManager::State::Playing) {        // start playing
            UiaController::instance()->stopAllMonitoring();
            QByteArray jsCode = m_edit->toPlainText().toUtf8();
            QString execInit = ";resetConfiguration();useTimer = %1;";
            // QString execFinished = ";execFinished();";
            auto result = ScriptEngine::instance()->syncRunJavaScript(execInit.arg("true") + jsCode /* + execFinished*/);
            // if (!result.first) {
            //     client->sendTextMessage("Exec-c-failed: " + result.second.toString().toLocal8Bit());
            // } else {
            //     client->sendTextMessage("Exec-c-success " + result.second.toString().toLocal8Bit());
            // }
            qApp->processEvents();
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
    auto guard = qScopeGuard([]{
    });
    return false;
}

bool UiaController::createUiaWidget() {
    // add to black list.
    QWidget *widget = new QWidget;
    ObjectListManager::instance()->addToBlackList({widget});     // 只加toplevel，判断的时候用 recursive 就行

    widget->setWindowTitle("Uia Controller");

    QVBoxLayout *vboxLayout  = new QVBoxLayout(widget);
    QHBoxLayout *layout  = new QHBoxLayout;
    vboxLayout->addLayout(layout);
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


    // 显示执行步骤
    m_edit = new QTextEdit;
    vboxLayout->addWidget(m_edit);
    QObject::connect(StepRecord::instance(), &StepRecord::stepAppend, [this](QString step){
        m_edit->append(step);
    });

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
    QObject::connect(button6, &QPushButton::clicked, [this](bool checked){
        QString openFileName = showFileDialog(QFileDialog::AcceptOpen);
        if (openFileName.isEmpty()) return;

        QByteArray allData;
        if (!fileReadWrite(openFileName, allData, true)) {
            qInfo() << "read error: " << openFileName;
            return;
        }
        m_edit->setText(allData);
    });
    QObject::connect(button1, &QPushButton::clicked, [](bool checked){
        OperationManager::instance()->startRecording();
    });
    QObject::connect(button2, &QPushButton::clicked, [](bool checked){
        OperationManager::instance()->startPlaying();
    });
    QObject::connect(button3, &QPushButton::clicked, [this](bool checked){
        // if (OperationManager::instance()->state() != OperationManager::Recording) {
        //     qInfo() << "不在录制状态!";
        //     return;
        // }

        QString saveFileName = showFileDialog(QFileDialog::AcceptSave);
        if (saveFileName.isEmpty()) return;
        qInfo() << "Save json file to: " << saveFileName;
        QByteArray byte = m_edit->toPlainText().toUtf8();
        bool res = fileReadWrite(saveFileName, byte, false);
        if (res) {
            qInfo() << "save success";
        }
    });
    QObject::connect(button4, &QPushButton::clicked, [](bool checked){
        qApp->exit();
    });

    widget->setFixedSize(QSize(widget->sizeHint().width(), widget->sizeHint().height() + 200));
    widget->show();
}

