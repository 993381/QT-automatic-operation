#include "uiacontroller.h"
#include "objectlistmanager.h"

#include "probe.h"
#include "sigspy.h"

#include <QtCore/private/qobject_p.h>  // qt_register_signal_spy_callbacks
#include <QEvent>
#include <QMouseEvent>

#include <QWidget>

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
