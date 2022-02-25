#ifndef UTIL_H
#define UTIL_H

#include <QFile>
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include "objectlistmanager.h"

bool fileReadWrite(const QString &fileName, QByteArray &data, bool isRead) {
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

QString showFileDialog(const QFileDialog::AcceptMode &mode) {
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

#endif//UTIL_H
