#ifndef UTIL_H
#define UTIL_H

#include <QMap>
#include <QFile>
#include <QDebug>
#include <QEvent>
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QString>
#include <QListView>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardItem>
#include <QAbstractButton>
#include <QStandardItemModel>
#include <qitemselectionmodel.h>
#include <QThread>

#include "objectpath.h"
#include "objectlistmanager.h"
#include "objectpathresolver.h"

inline bool fileReadWrite(const QString &fileName, QByteArray &data, bool isRead) {
    QFile file(fileName);
    if (isRead && !file.exists()) {         // 文件不存在
        qInfo() << "file not exist!";
        return false;
    }
    QIODevice::OpenMode mode = isRead ? QIODevice::ReadOnly : QIODevice::WriteOnly;
    if (!file.open(mode | QIODevice::Text))
        return false;
    if (isRead) {
        data = file.readAll();
    } else {
        file.write(data, data.length());
    }
    file.close();
    return true;
};

inline QString showFileDialog(const QFileDialog::AcceptMode &mode) {
    QFileDialog dialog;
    ObjectListManager::instance()->addToBlackList({&dialog});

    QStringList filters;

    filters.append("Text File (*.js)");
    filters.append("All Files (*)");
    filters.append("Config Files (*.js)");

    dialog.setWindowTitle(mode == QFileDialog::AcceptOpen ? "录制" : "保存");
    dialog.setAcceptMode(mode);                 // 打开模式
    dialog.setNameFilters(filters);             // 筛选器
    QString fileName;
    if (mode == QFileDialog::AcceptOpen)
    {
        dialog.setFileMode(QFileDialog::ExistingFile);
    }
    if (mode == QFileDialog::AcceptSave)
    {
        dialog.setFileMode(QFileDialog::AnyFile);
    }
    if (dialog.exec() == QFileDialog::Accepted)
    {
        fileName = dialog.selectedFiles()[0];
    }
    return fileName;
}

// 从全局或给定的obj下面找符合条件的对象
enum FindType { Unknow, All, ByObjectName, ByAccessibleName,
                ByClassName, ByItemText, ByButtonText,
                ByButtonInfo,
                ByNoTextButtonIndex, ByToolTip/*, ByItemIndex*/ };
inline QObjectList findObjects(FindType type, QVariant value, QObject *rootObj = nullptr) {
    ObjectPathResolver resolver;
    if (type == ByObjectName) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            return obj->objectName() == value.toString();
        });
    }
    if (type == ByAccessibleName) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            if (QWidget *widget = qobject_cast<QWidget *>(obj)) {
                if (auto button = qobject_cast<QAbstractButton *>(widget)) {
                    qInfo() << "findObjects ByAccessibleName xxxxxxxxxx: " << button;
                }
                return widget->accessibleName() == value.toString() && widget->isVisible();
            }
            return false;
        });
    }
    if (type == ByClassName) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            return obj->metaObject()->className() == value.toString();
        });
    }
    if (type == ByItemText) {
        // select item by text/index?
        resolver.setValidFilter([value](QObject *obj) -> bool {
            if (obj->objectName() != "qt_scrollarea_viewport") {
                return false;
            }
            if (auto listview = qobject_cast<QListView *>(obj->parent())) {
                if (!listview->isVisible()) {
                    return false;
                }
                for (int r = 0; r < listview->model()->rowCount(); ++r) {
                    for (int c = 0; c < listview->model()->columnCount(); ++c) {
                        if (listview->model()->index(r, c).data().toString() == value.toString()) {
                            qInfo() << "found... " << value.toString() << listview->objectName() << listview->metaObject()->className();
                            return true;
                        }
                    }
                }
            }
            return false;
        });
    }
    if (type == ByButtonText) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            if (auto button = qobject_cast<QAbstractButton *>(obj)) {
                if (button->isVisible() && button->text() == value.toString()) {
                    return true;
                }
            }
            return false;
        });
    }
    if (type == ByNoTextButtonIndex) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            if (auto button = qobject_cast<QAbstractButton *>(obj)) {
                if (button->isVisible() && button->text().isEmpty()) {
                    return true;
                }
            }
            return false;
        });
    }
    if (type == ByButtonInfo) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            QString findBtnType = value.toStringList().at(0);
            QString text = value.toStringList().at(1);
            if (auto button = qobject_cast<QAbstractButton *>(obj)) {
                if (!button->isVisible()) {
                    return false;
                }
                if (findBtnType == QStringLiteral("byAccName") && button->accessibleName() == text) {
                    qInfo() << "ByNoTextButtonInfo setValidFilter " << button << button->isVisible() << button->isEnabled();
                    return true;
                }
                if (findBtnType == QStringLiteral("byObjName") && button->objectName() == text) {
                    return true;
                }
                if (findBtnType == QStringLiteral("byClassName") && button->metaObject()->className() == text) {
                    return true;
                }
                if (findBtnType == QStringLiteral("byToolTip") && button->toolTip() == text) {
                    return true;
                }
                if (button->objectName() == "ActionButton") {
                    qInfo() << "ByNoTextButtonInfo setValidFilter " << button << button->isVisible() << button->isEnabled();
                    return true;
                }
                // qInfo() << "object not find........................" << button->accessibleName() << button->metaObject()->className() << " " << text;
            }
            return false;
        });
    }
    if (type == ByToolTip) {
        resolver.setValidFilter([value](QObject *obj) -> bool {
            if (auto button = qobject_cast<QAbstractButton *>(obj)) {
                if (button->toolTip() == value.toString()) {
                    return true;
                }
            }
            return false;
        });
    }

    if (rootObj) {
        resolver.discoverObject(rootObj);
    } else {
        resolver.findExistingObjects();
    }
    return resolver.validObjects();
}

inline bool sendEvent() {
    // QKeyEvent tabKey(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    // QCoreApplication::sendEvent(this, &tabKey);
}


inline QModelIndexList getItemIndexesByText(QObject *obj, const QString &text) {
    QModelIndexList tempList;
    if (auto listview = qobject_cast<QListView *>(obj->parent())) {
        for (int r = 0; r < listview->model()->rowCount(); ++r) {
            for (int c = 0; c < listview->model()->columnCount(); ++c) {
                if (listview->model()->index(r, c).data().toString() == text) {
                    tempList.push_back(listview->model()->index(r, c));
                }
            }
        }
    }
    return tempList;
}

// 如果text唯一，直接选中，如果不唯一，做更多处理，按照指定的 row、coloum 选中
inline bool selectListItemByText(bool isDoubleClick, const QString &text, int index = 0) {
    const QObjectList &list = findObjects(ByItemText, text);
    qInfo() << "selectListItemByText size: " << list.size();

    // 查找每个 viewport 里面是否有重复的，将 listview 与 modexIndex 对应起来
    using ItemViewMap2Index = QVector<QPair<QAbstractItemView*, QModelIndex>>;
    auto getAllItemIndexesByText = [](QObject *obj, const QString &text) {
        ItemViewMap2Index itemIndexList;
        if (auto itemview = qobject_cast<QAbstractItemView *>(obj->parent())) {
            for (int r = 0; r < itemview->model()->rowCount(); ++r) {
                for (int c = 0; c < itemview->model()->columnCount(); ++c) {
                    if (itemview->model()->index(r, c).data().toString() == text) {
                        itemIndexList.push_back({ itemview, itemview->model()->index(r, c) });
                    }
                }
            }
        }
        return itemIndexList;
    };
    // 2. 再获取所有的 modex index, 找到符合条件的
    ItemViewMap2Index unfoldMap;
    for (QObject *obj : list) {
        unfoldMap << getAllItemIndexesByText(obj, text);
    }
    if (index >= unfoldMap.size()) {
        qInfo() << "selectListItemByText index too big! " << index;
        return false;
    }

    auto listview = unfoldMap.at(index).first;
    auto modelIndex = unfoldMap.at(index).second;
    qInfo() << "selectListItemByText will select: " << modelIndex.data().toString() << " row: " << modelIndex.row() << " column: " << modelIndex.column();

    if (isDoubleClick) {
        Q_EMIT listview->doubleClicked(modelIndex);
    } else {
        Q_EMIT listview->clicked(modelIndex);
    }
    Q_EMIT listview->activated(modelIndex);
    listview->selectionModel()->select(modelIndex, QItemSelectionModel::SelectCurrent);
    return true;
}

inline bool selectListItemByIndex(const QString className, int uniq_index, int row, int column) {
    const QObjectList &result = findObjects(ByObjectName, className);
    if (result.size() <= uniq_index)
        return false;
    if (auto listview = qobject_cast<QListView *>(result.at(uniq_index))) {
        if (row > listview->model()->rowCount() || column > listview->model()->columnCount()) {
            return false;
        }
        auto index = listview->model()->index(row, column);
        if (index.isValid()) {
            listview->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            Q_EMIT listview->activated(index);
            return true;
        }
    }
    qInfo() << "selectListItemByIndex cannot find anyone!!!!!!!!";
    return false;
}

inline bool clickButtonByObjectName(const QString text, int index = 0) {
    const QObjectList &list = findObjects(ByObjectName, text);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))) {
        // button->click();
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        qInfo() << "clickButtonByButtonText end  ...........";
        return true;
    }
    return false;
}
inline bool clickButtonByButtonText(const QString text, int index = 0) {
    const QObjectList &list = findObjects(ByButtonText, text);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        if (!button->isEnabled()) {
            return false;
        }
        QPoint pos(5, 5);
        QMouseEvent *pressEv = new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        // qInfo() << "clickButtonByButtonText end  ...........";
        // Q_EMIT button->clicked();
        return true;
    }
    return false;
}
inline bool clickNoTextButtonByIndex(int index = 0) {
    const QObjectList &list = findObjects(ByNoTextButtonIndex, index);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        if (!button->isEnabled()) {
            return false;
        }
        // button->click();
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        qInfo() << "clickButtonByButtonText end  ...........";
        return true;
    }
    return false;
}
inline bool clickButtonByInfo(const QStringList &info) {
    QString type = info.at(0);
    QString text = info.at(1);
    int index = info.size() > 2 ? info.at(2).toInt() : 0;
    const QObjectList &list = findObjects(ByButtonInfo, QStringList{ type, text});
    qInfo() << "clickNoTextButtonByInfo2: " << text << " size: " << list.size();
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        if (!button->isEnabled()) {
            return false;
        }
        qInfo() << "clickButtonByButtonText start...........";
        // Q_EMIT button->clicked();
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        qInfo() << "clickButtonByButtonText end  ...........";
        return true;
    }
    return false;
}
inline bool clickButtonByToolTip(const QString text, int index = 0) {
    const QObjectList &list = findObjects(ByToolTip, text);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        // button->click();
        if (!button->isEnabled()) {
            return false;
        }
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        return true;
    }
    return false;
}
inline bool clickButtonByAccessbleName(const QString text, int index = 0) {
    const QObjectList &list = findObjects(ByAccessibleName, text);
    qInfo() << "clickButtonByAccessbleName: " << list.size();
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        // button->click();
        if (!button->isEnabled()) {
            return false;
        }
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        return true;
    }
    return false;
}
#if 0
inline bool clickButtonByButtonIndex(const QString text, int index) {
    const QObjectList &list = findObjects(ByClassName, text);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        // button->click();
        if (!button->isEnabled()) {
            return false;
        }
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        return true;
    }
    return false;
}
#endif

// 按以下几种查找方式，直到确定该对象是唯一的
// byAccessableName、byClassName、byIndex
// value 用于传递 item text 等信息

enum LocationType {
    unknow,

    // 控件的三种基本信息,如果唯一,则只根据这三种信息查找
    byAccName,      // 优先级1

    byObjName,      // 优先级2
    byClassName,    // 优先级3

    byItemText,     // 优先级2 item view 有文字则优先使用文字查找
    byItemViewInfo, // 根据以上四种信息查找

    byButtonText,   // 优先级2
    // byButtonIndex  根据索引找就是同类的索引，最后带上类名和索引
};
inline static const QMap<LocationType, QString> type2Str {
    { byAccName, "byAccName" },
    { byObjName, "byObjName" },
    { byClassName, "byClassName" },
    { byItemText, "byItemText" },
    { byItemViewInfo, "byItemViewInfo" },
    { byButtonText, "byButtonText" },
    // { byButtonIndex, "byButtonIndex" },
};

struct ObjInfo {
    bool isValid() {
        return index!= -1 && type!= unknow;
    }
    int index = -1;
    LocationType type = unknow;
    QVariant value;
};

// 如果是 QAbstractItemView 等, value 要传 ModexIndex 进来
// 选中项(文本)  选中列表(列表信息) 选中项(行,列)
// 查询列表(列表信息) 返回全部/指定的列表
// 查询列表项(列表信息) 每一项都有文本则返回每项的文本, 没有文本则返回range
// 选择项视图(className, index)
// 查找文本项(text, item-view-info-list) 按文字从列表中找. 返回找到的结果列表
// 选择项(row, col) 按照行列数直接选中列表中的项
//!TODO: 应该首先从活动窗口里面找, 活动窗口中找到的结果要优先一些
inline ObjInfo findUniqInfo(QObject *object, QVariant value = {}) {
    ObjectPathResolver resolver;
    resolver.findExistingObjects();

    QVector<QPair<LocationType, QVariant>> findTypes;
    QString accessableName;

    //! 根据各自具体信息查找
    // byItemText        abstract view item 只有一两种找的方式有意义, text 和 index
    // if (object->objectName() == "qt_scrollarea_viewport" && object->parent()) {
    if (auto itemview = qobject_cast<QAbstractItemView *>(object)) {
        //!TODO: 首先还是要提示用户对列表进行标记
        //! 如果每项都有文字就根据文字查找, 没有文字根据标记和index找并提示用户进行标记
        QString itemText = value.toModelIndex().data().toString();
        if (!itemText.isEmpty()) {
            // qInfo() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
            findTypes << QPair<LocationType, QString>{ byItemText, itemText };
            goto FIND_OBJECTS;
        } else {
            //! findTypes << QPair<LocationType, QVariant>{ byItemText,  value };
        }
    }
    // byButtonText      button可以有多种找的方式. 根据索引查找就要在相同 className 中查找
    if (auto button = qobject_cast<QAbstractButton *>(object)) {
        if (!button->text().isEmpty()) {
            findTypes << QPair<LocationType, QString>{ byButtonText, button->text() };
        } else {
            // 最终记录 button 的 index + 类名
            // noTextButton又叫图形按钮，生成代码的时候找没有文字的
            // findTypes << QPair<LocationType, QString>{ byClassName, button->metaObject()->className() };
        }
    }

    //! 根据三种基本信息查找
    // byAccName
    if (QWidget *widget = qobject_cast<QWidget *>(object)) {
        // 只有 widget 或继承自 widget 的有 accessableName
        accessableName = widget->accessibleName();
        if (!widget->accessibleName().isEmpty()) {
            findTypes << QPair<LocationType, QString>{ byAccName, accessableName };
        }
    }
    // byObjName
    if (!object->objectName().isEmpty()) {
        findTypes << QPair<LocationType, QString>{ byObjName, object->objectName() };
    }
    // byClassName   把这种方式放到最后,因为不是最佳的方式
    findTypes << QPair<LocationType, QString>{ byClassName, object->metaObject()->className() };

    FIND_OBJECTS:

    // 如果没有发现任何唯一信息,就按照优先级从 byAccName byObjName byClassName 三个中选一个记录 index 信息
    // 最终如果都没有发现唯一的, 就返回一个 index 最小的
    QVector<ObjInfo> result;
    for (auto findType : findTypes) {
        if (findType.first == byAccName) {
            resolver.setValidFilter([&](QObject *obj) -> bool {
                QWidget *widget = qobject_cast<QWidget *>(obj);
                return widget && widget->accessibleName() == accessableName;
            });
            QObjectList list = resolver.validObjects();
            ObjInfo info = { list.indexOf(object), findType.first, findType.second.toString() };
            if (list.size() == 1) {
                return info;
            }
            if (list.size()) {
                result << info;
            }
            continue;
        }
        if (findType.first == byObjName) {
            resolver.setValidFilter([&](QObject *obj) -> bool {
                return obj->objectName() == findType.second.toString();
            });
            QObjectList list = resolver.validObjects();
            ObjInfo info = { list.indexOf(object), findType.first, findType.second.toString() };
            if (list.size() == 1) {
                return info;
            }
            if (list.size()) {
                result << info;
            }
            continue;
        }
        if (findType.first == byClassName) {
            // 只有 widget 或继承自 widget 的有 accessableName
            resolver.setValidFilter([&](QObject *obj) -> bool {
                return obj->metaObject()->className() == findType.second.toString();
            });
            QObjectList list = resolver.validObjects();
            ObjInfo info = { list.indexOf(object), findType.first, findType.second.toString() };
            if (list.size() == 1) {
                return info;
            }
            if (list.size()) {
                result << info;
            }
            continue;
        }
        if (findType.first == byItemText) {
            resolver.setValidFilter([&](QObject *obj) -> bool {
                 if (obj->objectName() != "qt_scrollarea_viewport") {
                     return false;
                 }
                // 找到所有包含该文本的 listview
                if (auto itemview = qobject_cast<QAbstractItemView *>(obj->parent())) {
                    if (!itemview->isVisible()) {
                        return false;
                    }
                    qInfo() << "found item view";
                    for (int r = 0; r < itemview->model()->rowCount(); ++r) {
                        for (int c = 0; c < itemview->model()->columnCount(); ++c) {
                            if (itemview->model()->index(r, c).data().toString() == findType.second.toString()) {
                                qInfo() << "found... " << findType.second.toString() << itemview->objectName() << itemview->metaObject()->className();
                                return true;
                            }
                        }
                    }
                }
                return false;
            });
            //! TODO 增加查找选中某类对象的功能, 再从该对象的孩子里面找符合条件的 selectObjByClass(className, index)
            // 1. 获取到了所有包含该文本的 listview 的子对象 qt_scrollarea_viewport
            QObjectList list = resolver.validObjects();

            // 查找每个 viewport 里面是否有重复的，将 listview 与 modexIndex 对应起来
            using ItemViewMap2Index = QVector<QPair<QAbstractItemView*, QModelIndex>>;
            auto getAllItemIndexesByText = [](QObject *obj, const QString &text) {
                ItemViewMap2Index itemIndexList;
                if (auto itemview = qobject_cast<QAbstractItemView *>(obj->parent())) {
                    for (int r = 0; r < itemview->model()->rowCount(); ++r) {
                        for (int c = 0; c < itemview->model()->columnCount(); ++c) {
                            if (itemview->model()->index(r, c).data().toString() == text) {
                                itemIndexList.push_back({ itemview, itemview->model()->index(r, c) });
                            }
                        }
                    }
                }
                return itemIndexList;
            };
            // 2. 再获取所有的 modex index, 找到符合条件的
            ItemViewMap2Index unfoldMap;
            for (QObject *obj : list) {
                unfoldMap << getAllItemIndexesByText(obj, findType.second.toString());
            }
            // 4. 从 itemview 与 index 的集合中找到符合row\count的索引
            if (auto itemview = qobject_cast<QAbstractItemView *>(object)) {
                for (int index = 0; index < unfoldMap.size(); ++index) {
                    auto itemIndex = unfoldMap.at(index);
                    if (itemIndex.first == itemview && itemIndex.second == value.toModelIndex()) {
                        // 把找到的索引和文本以及解析出来的行\列返回, 最重要的是索引
                        return { index, findType.first, value.toModelIndex().data().toString() };
                    }
                }
            }
            // 走到这里就错了,至少包含它自身这一项
            qInfo() << "list item by text not uniq in current window";
            return {};
        }
        if (findType.first == byButtonText) {
            resolver.setValidFilter([&](QObject *obj) -> bool {
                auto button = qobject_cast<QAbstractButton *>(obj);
                return button &&  button->text() == findType.second.toString();
            });
            QObjectList list = resolver.validObjects();
            ObjInfo info = { list.indexOf(object), findType.first, findType.second.toString() };
            if (list.size() == 1) {
                return info;
            }
            if (list.size()) {
                result << info;
            }
            continue;
        }
    }

    // 如果没有找到唯一标记信息,返回 index 最小的信息
    int minimium = -2;
    int miniIdx = -1;
    for (int i = 0; i < result.size(); ++i) {
        auto info = result.at(i);
        if (info.index == -1) {
            // 通常不可能是-1,如果是的话就错了
            continue;
        }
        if (minimium == -2) {
            minimium = info.index;
            miniIdx = i;
        }
        if (minimium > info.index) {
            minimium = info.index;
            miniIdx = i;
        }
    }
    if (miniIdx != -1) {
        return result.at(miniIdx);
    }
    //!TODO: 在图形界面中提示。最后根据 index 找的控件都要提示一下。index 只是一种 workaround
    qInfo() << "Error, xxxxxxxxxxxx 控件不唯一, 请标记... " << object->metaObject();
    return {};
}


inline bool setLineEditTextByInfo(const QStringList &lineEditInfo) {
    ObjectPathResolver resolver;
    QStringList editInfo = lineEditInfo;
    QString text, method, methodType;
    text = editInfo.takeFirst();
    method = editInfo.takeFirst();
    methodType = editInfo.takeFirst();
    int index = 0;
    if (editInfo.size()) {
        index = editInfo.takeFirst().toInt();
    }

    resolver.setValidFilter([method, methodType](QObject *obj) -> bool {
        if (auto edit = qobject_cast<QLineEdit *>(obj)) {
            if (method == "byAcc") {
                return edit->accessibleName() == methodType;
            }
            if (method == "byObj") {
                return edit->objectName() == methodType;
            }
            if (method == "byClass") {
                return edit->metaObject()->className() == methodType;
            }
            qInfo() << "edit type error!!! " << method;
            return false;
        }
        return false;
    });
    resolver.findExistingObjects();

    QObjectList list = resolver.validObjects();
    if (index >= list.size()) {
        return false;
    }
    auto lineEdit = qobject_cast<QLineEdit *>(list.at(index));
    lineEdit->setText(text);
    Q_EMIT lineEdit->editingFinished();
    return true;
}

inline bool clickButtonByButtonInfo(const QStringList &buttonInfo) {
    ObjectPathResolver resolver;
    QStringList btnInfo = buttonInfo;
    QString text;
    QString method, methodType;
    bool isFindByText = btnInfo.size() == 1;
    int index = 0;
    if (isFindByText) {
        text = btnInfo.takeFirst();
    } else {
        method = btnInfo.takeFirst();
        methodType = btnInfo.takeFirst();
        if (btnInfo.size()) {
            index = btnInfo.takeFirst().toInt();
        }
    }
    resolver.setValidFilter([=](QObject *obj) -> bool {
        if (auto button = qobject_cast<QAbstractButton *>(obj)){
            if (isFindByText) {
                return button->text() == text;
            } else {
                if (method == "byAcc") {
                    return button->accessibleName() == methodType;
                }
                if (method == "byObj") {
                    return button->objectName() == methodType;
                }
                if (method == "byClass") {
                    return button->metaObject()->className() == methodType;
                }
                qInfo() << "button type error!!! " << method;
                return false;
            }
        }
        return false;
    });
    resolver.findExistingObjects();

    QObjectList list = resolver.validObjects();
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        // button->click();
        if (!button->isEnabled()) {
            return false;
        }
        QPoint pos(0, 0);
        QMouseEvent pressEv(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent *releaseEv = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, &pressEv);
        QApplication::postEvent(button, releaseEv); // 防止模态窗口阻塞在这里, 配合 qApp->processEvents 进行处理
        return true;
    }
    return false;
}


#endif//UTIL_H
