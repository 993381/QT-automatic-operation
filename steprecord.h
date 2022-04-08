#ifndef STEP_RECORD_H
#define STEP_RECORD_H
#include <QObject>
#include <QString>
#include <QStringList>

class StepRecord : public QObject {
    Q_OBJECT
public:
    StepRecord() {
        connect(this, &StepRecord::stepUpdate, this, [this](QStringList steps){
            m_steps = steps;
        });
    }
    ~StepRecord() {}

    static StepRecord *instance() {
        static StepRecord *record = new StepRecord;
        return record;
    }
    QStringList steps() {
        return m_steps;
    }
    bool append(const QString &step) {
        if (step.isEmpty()) {
            return false;
        }
        m_steps << step;
        Q_EMIT stepAppend(step);
    }

Q_SIGNALS:
    void stepUpdate(QStringList steps); // 从外部更新内部维护的
    void stepAppend(QString step);      // 有新的步骤就通知外部

private:
    QStringList m_steps;
};



#endif//STEP_RECORD_H
