#ifndef UIACONTROLLER_H
#define UIACONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QApplication>
#include "objectpath.h"

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

    bool attachApp(int pid);
    bool startApp(const QStringList &programAndArgs);

    bool startEventMonitoring();
    void stopEventMonitoring();

    void startSigslotMonitoring();
    void stopSigslotMonitoring();

    bool startAllMonitoring();
    void stopAllMonitoring();

    bool createUiaWidget();
    bool initOperationSequence();

private:
    QScopedPointer<EventFilter> m_filter;

    bool nextStep();
    QVector<ObjectPath> m_paths;
    QVector<ObjectPath>::iterator m_itr;

    QScopedPointer<QProcess> m_process;
};

#endif//UIACONTROLLER_H
