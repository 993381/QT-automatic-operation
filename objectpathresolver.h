#ifndef OBJECTPATHRESXOLVER_H
#define OBJECTPATHRESXOLVER_H

#include <QObject>
#include <functional>

class ObjectPathResolver {
public:
    ObjectPathResolver();
    ~ObjectPathResolver();
    void findExistingObjects();
    // 最好的方法：通过栈的深度得到树的高度，检验对象类型，决定是否保存
    void discoverObject(QObject *object);
    // 对搜索过程进行过滤，过早使用会导致子对象被过滤掉，和路径相关的才用
    void setDiscoverCallback(std::function<bool(QObject *)> callback);
    // 对结果进行过滤
    void setValidFilter(std::function<bool(QObject *)> filter);
    QObjectList objects() const;
    QObjectList validObjects() const;
private:
    std::function<bool(QObject *)> m_callback;
    std::function<bool(QObject *)> m_filter;
    int m_layer_count;                  //! TODO: 改成layer count
    QObjectList m_objs;
    QObjectList m_validObjects;
};


#endif//OBJECTPATHRESXOLVER_H
