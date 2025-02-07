// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "vcsbase_global.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace VcsBase {

namespace Internal { class CleanDialogPrivate; }

class VCSBASE_EXPORT CleanDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CleanDialog(QWidget *parent = nullptr);
    ~CleanDialog() override;

    void setFileList(const QString &workingDirectory, const QStringList &files,
                     const QStringList &ignoredFiles);

public slots:
    void accept() override;

private:
    void slotDoubleClicked(const QModelIndex &);
    void selectAllItems(bool checked);
    void updateSelectAllCheckBox();

    QStringList checkedFiles() const;
    bool promptToDelete();
    void addFile(const QString &workingDirectory, QString fileName, bool checked);

    Internal::CleanDialogPrivate *const d;
};

} // namespace VcsBase
