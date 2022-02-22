#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H
#include <QObject>
#include <utility>

template <typename F>
class [[nodiscard]] QScopeGuard
{
public:
    explicit QScopeGuard(F &&f) noexcept
        : m_func(std::move(f))
    {
    }

    explicit QScopeGuard(const F &f) noexcept
        : m_func(f)
    {
    }

    QScopeGuard(QScopeGuard &&other) noexcept
        : m_func(std::move(other.m_func))
        , m_invoke(std::exchange(other.m_invoke, false))
    {
    }

    ~QScopeGuard() noexcept
    {
        if (m_invoke)
            m_func();
    }

    void dismiss() noexcept
    {
        m_invoke = false;
    }

private:
    Q_DISABLE_COPY(QScopeGuard)

    F m_func;
    bool m_invoke = true;
};

#ifdef __cpp_deduction_guides
template <typename F> QScopeGuard(F(&)()) -> QScopeGuard<F(*)()>;
#endif

//! [qScopeGuard]
template <typename F>
[[nodiscard]] QScopeGuard<typename std::decay<F>::type> qScopeGuard(F &&f)
{
    return QScopeGuard<typename std::decay<F>::type>(std::forward<F>(f));
}

#endif//SCOPEGUARD_H
