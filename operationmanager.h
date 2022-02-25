#ifndef OPERATIONMANAGER_H
#define OPERATIONMANAGER_H
#include <QObject>

//! TODO: 使用状态机管理状态？
class OperationManager : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(OperationManager)
    OperationManager() : m_state (Undefine) {}
    ~OperationManager() {}
public:
    static OperationManager* instance()  {
        static OperationManager* manager = new OperationManager;
        return manager;
    }
    enum State { Undefine, Playing, Recording, Stop };
    void startPlaying() {
        m_state = Playing;
        Q_EMIT Start(m_state);
    }
    void startRecording() {
        m_state = Recording;
        Q_EMIT Start(m_state);
    }
    void stop() {
        m_state = Stop;
        Q_EMIT Stopped(m_state);
    }
    State state() {
        return m_state;
    }
    bool isOperating() {
        return m_state == Playing || m_state == Recording;
    }

Q_SIGNALS:
    void Start(State);
    void Stopped(State);

private:
    State m_state;
};

#endif//OPERATIONMANAGER_H
