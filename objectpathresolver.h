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
    void setDiscoverCallback(std::function<bool(QObject *)> callback);
    QObjectList objects() const;
private:
    std::function<bool(QObject *)> m_callback;
    int m_layer_count;                  //! TODO: 改成layer count
    QObjectList m_objs;
    QObjectList m_validObjects;
};


#endif//OBJECTPATHRESXOLVER_H
