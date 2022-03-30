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

    filters.append("Text File (*.json)");
    filters.append("All Files (*)");
    filters.append("Config Files (*.json)");

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
inline bool selectListItemByText(const QString &text, int index = 0) {
    const QObjectList &list = findObjects(ByItemText, text);
    qInfo() << "selectListItemByText size: " << list.size();

    // 从整个页面中发现有相同的itemText的ListView就都存起来，再看ListView里面还有没有相同的
    QVector<QPair<QListView*, QModelIndex>> itemIndexList;
    for (QObject *obj : list) {
        if (auto listview = qobject_cast<QListView *>(obj->parent())) {
            for (auto idx : getItemIndexesByText(obj, text)) {
                itemIndexList.push_back({listview, idx});
            }
        }
    }
    if (index >= itemIndexList.size()) {
        qInfo() << "selectListItemByText index too big! " << index;
        return false;
    }

    auto listview = itemIndexList.at(index).first;
    auto modelIndex = itemIndexList.at(index).second;
    qInfo() << "selectListItemByText will select: " << modelIndex.data().toString() << " row: " << modelIndex.row() << " column: " << modelIndex.column();
    listview->pressed(modelIndex);
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
        button->click();
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
        qInfo() << "clickButtonByButtonText start...........";
        // Q_EMIT button->clicked();
        QPoint pos;
        pos.setX(0);
        pos.setY(0);
        QMouseEvent *mEvnPress;
        QMouseEvent *mEvnRelease;
        mEvnPress = new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(button, mEvnPress);
        mEvnRelease = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::postEvent(button, mEvnRelease);
        qInfo() << "clickButtonByButtonText end  ...........";
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
        button->click();
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
        qInfo() << "clickNoTextButtonByInfo3: " << text;
        button->click();
        // Q_EMIT button->clicked(true);
        // Q_EMIT button->clicked(false);
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
        button->click();
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
        button->click();
        return true;
    }
    return false;
}
inline bool clickButtonByButtonIndex(const QString text, int index) {
    const QObjectList &list = findObjects(ByClassName, text);
    if (index >= list.size()) {
        return false;
    }
    if (auto button = qobject_cast<QAbstractButton *>(list.at(index))){
        button->click();
        return true;
    }
    return false;
}

inline bool setLineEditTextByIndex(const QString &lineEditContent,
                                   int index = 0) {
    ObjectPathResolver resolver;
    resolver.setValidFilter([](QObject *obj) -> bool {
        if (auto edit = qobject_cast<QLineEdit *>(obj)) {
            return edit->isVisible();
        }
        return false;
    });
    resolver.findExistingObjects();

    QObjectList list = resolver.validObjects();
    if (index >= list.size()) {
        return false;
    }
    auto lineEdit = qobject_cast<QLineEdit *>(list.at(index));
    lineEdit->setText(lineEditContent);
    Q_EMIT lineEdit->editingFinished();
    return true;
}

inline bool setLineEditTextByItemIndex(const QString &lineEditClassName,
                                       const QString &lineEditContent,
                                       const int &layer,
                                       const int &index,
                                       const QVector<ObjectPath>::iterator &itr) {
    QObjectList list = findObjects(ByClassName, lineEditClassName);
    // qInfo() << "setLineEditTextByItemIndex: " << list.size() << " layer " << layer << " index " << index;
    if (index >= list.size()) {
        return false;
    }
    for (auto edit : list) {
        auto path = ObjectPath::parseObjectPath(edit);
        ObjectPath tempPath(path);
        if (*itr == tempPath) {
            auto lineEdit = qobject_cast<QLineEdit *>(edit);
            lineEdit->setText(lineEditContent);
            Q_EMIT lineEdit->editingFinished();
            return true;
        }
    }
//    if (auto lineEdit = qobject_cast<QLineEdit *>(list[index < 0 ? 0 : index])){
//        // if (ObjectPath::getLayerCount(lineEdit) != layer || ObjectPath::getSiblingIndex(lineEdit) != index) {
//        //     return false;
//        // }
//        lineEdit->setText(lineEditContent);
//        Q_EMIT lineEdit->editingFinished();
//        return true;
//    }
    return false;
}

#endif//UTIL_H
