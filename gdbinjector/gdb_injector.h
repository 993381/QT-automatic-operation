#ifndef GDB_INJECTOR_H
#define GDB_INJECTOR_H
#include <QObject>
#include <QProcess>
#include <QDebug>

#include <dlfcn.h>
#include <unistd.h>

class GdbInjector : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(GdbInjector)
    GdbInjector() {}
    ~GdbInjector() {
        if (m_process) {
            m_process->terminate();
            if (!m_process->waitForFinished(1000)) {
                m_process->kill(); // kill it softly
            }
        }
    }

public:
    static GdbInjector* instance() {
        static GdbInjector* injector = new GdbInjector;
        return injector;
    }
    //! TODO： 从pid启动用 ptrace 的方式实现，是实现跨进程录制的前提
    bool attachInject(const int &pid) {
    }

    bool launchPreload(const QStringList &programAndArgs, const QProcessEnvironment &env = QProcessEnvironment()) {
        if (m_process && m_process->state() == QProcess::ProcessState::Running) {
            qInfo() << "process is running";
            return false;
        }
        m_process.reset(new QProcess);

        qputenv("LD_PRELOAD", INJECTOR_DLL);
        qputenv("EXEC_JS_SCRIPT_PRE", "1");
        qputenv("SHOW_UIA_WINDOW_PRE", "1");

        QStringList args(programAndArgs);
//  这种方案为了方便调试
#define USE_EXEC_LAUNCH 0
#if !USE_EXEC_LAUNCH
        QString program = args.takeFirst();
        m_process->start(program, args);
        bool status = m_process->waitForStarted(-1);
        qInfo() << "pid xxxxxxxxxxxx: " << m_process->pid();
        if (!status) {
            qInfo() << "process start failed!";
            return false;
        }
        Q_EMIT gdbStarted();
        qInfo() << "process start success";

        return true;
#else
        // qputenv("SHOW_UIA_WINDOW_PRE", "0");

        QString program = args.takeFirst();
        // argv 不能为空
        args.insert(0, program);
        qInfo() << "args: " <<args;

        char *argv[args.size() + 1];
        // if (!args.size()) {
            argv[args.size()] = nullptr;
        // }
        for (int i = 0; i < args.size(); ++i) {
            QString arg = args.takeFirst();
            argv[i] = new char[arg.size()];
            memcpy(argv[i], arg.toStdString().c_str(), arg.toStdString().size());
        }
        extern char **environ;
        return execve(program.toStdString().c_str(), argv, environ) != -1;
#endif
    }

    //! 从GDB启动相当于子进程，还需要处理进程通信，比较麻烦
    bool launchInject(const QStringList &programAndArgs) {
        qputenv("EXEC_JS_SCRIPT", "1");
        QObject::connect(this, &GdbInjector::gdbStarted, this, [this]{
            // 1. 在main函数打断点等待注入程序
            execCmds({"set confirm off",
                      "break main",
                      "run",
                      "sha Qt5Core",
                      "break QCoreApplication::exec",
                      "continue"
                     });

            // 2. 在断点处注入程序并分离退出gdb让程序继续执行
            const QString &probeFunc("gammaray_probe_attach");
            execCmds({"sha dl",
                      QStringLiteral("call (void) dlopen(\"%1\", %2)").arg(INJECTOR_DLL).arg(RTLD_NOW).toUtf8(),
                      "sha " INJECTOR_DLL,
                      QStringLiteral("call (void) %1()").arg(probeFunc).toUtf8(),
                      "detach",             // 子进程分离
                      "continue"            // 原本是"quit"，不 quit，继续执行
                      // "quit"
                     });

            Q_EMIT injectFinished();
        });
        QStringList gdbArgs;
        gdbArgs.push_back(QStringLiteral("--args"));
        gdbArgs.append(programAndArgs);
        qInfo() << "gdb args: " << gdbArgs;

        // QProcessEnvironment env = QProcessEnvironment();
        // env.insert("EXEC_JS_SCRIPT", "1");
        return startDebugger(gdbArgs);
    }
    void close() { }

Q_SIGNALS:
    void gdbStarted();
    void injectFinished();

private:
    void execCmds(const QByteArrayList &cmds) {
        for (const QByteArray &cmd : cmds) {
            execCmd(cmd);
        }
    }
    void execCmd(const QByteArray &cmd, bool waitForWritten = true)
    {
        m_process->write(cmd + '\n');

        if (waitForWritten)
            m_process->waitForBytesWritten(-1);
        qInfo() << m_process->readAll();
    }
    bool startDebugger(const QStringList &args = {}, const QProcessEnvironment &env = QProcessEnvironment())
    {
        if (m_process && m_process->state() == QProcess::ProcessState::Running) {
            qInfo() << "gdb is running";
            return false;
        }
        m_process.reset(new QProcess);
        if (!env.isEmpty())
            m_process->setProcessEnvironment(env);
        QObject::connect(m_process.data(), &QProcess::readyReadStandardError, [this](){
            m_process->setReadChannel(QProcess::StandardError);
            while (m_process->canReadLine()) {
                const QString &error = QString::fromLocal8Bit(m_process->readLine());
                qInfo() << error;
            }
        });
        QObject::connect(m_process.data(), &QProcess::readyReadStandardOutput, [this]{
            m_process->setReadChannel(QProcess::StandardOutput);
            while (m_process->canReadLine()) {
                const QString &output = QString::fromLocal8Bit(m_process->readLine());
                qInfo() << output;
            }
        });
        QObject::connect(m_process.data(), &QProcess::started, []{});
        QObject::connect(m_process.data(), qOverload<int>(&QProcess::finished), [this](int exitCode){
#if 0
            if (!mManualError) {
                emit attached();
            } else {
                emit finished();
            }
#endif
        });
        m_process->setProcessChannelMode(QProcess::SeparateChannels);
        m_process->start("/usr/bin/gdb", args);
        bool status = m_process->waitForStarted(-1);

        if (!status) {
            qInfo() << "gdb start failed!";
        } else {
            Q_EMIT gdbStarted();
            qInfo() << "gdb start success";
        }
        return status;
    }

private:
    QScopedPointer<QProcess> m_process;
};

#endif//GDB_INJECTOR_H
