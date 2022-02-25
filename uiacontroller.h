#ifndef UIACONTROLLER_H
#define UIACONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QApplication>

class EventFilter;
class UiaController {
    Q_DISABLE_COPY(UiaController)
    UiaController();
    ~UiaController();

public:
    static UiaController* instance()  {
        static UiaController* controller = new UiaController;
        return controller;
    }
    bool startEventMonitoring();
    void stopEventMonitoring();

    void startSigslotMonitoring();
    void stopSigslotMonitoring();

    bool startAllMonitoring();
    void stopAllMonitoring();

    bool initOperationSequence();
    bool createUiaWidget();

private:
    QScopedPointer<EventFilter> m_filter;
};

#endif//UIACONTROLLER_H
