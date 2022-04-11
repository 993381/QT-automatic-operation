#ifndef UIACONTROLLER_H
#define UIACONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QApplication>

class EventFilter;
class QTextEdit;
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

    QTextEdit *m_edit = nullptr;
    bool nextStep();
};

#endif//UIACONTROLLER_H
