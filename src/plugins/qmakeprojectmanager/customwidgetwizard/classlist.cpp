// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "classlist.h"

#include <utils/qtcassert.h>

#include <QKeyEvent>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QDebug>
#include <QRegularExpression>

namespace QmakeProjectManager {
namespace Internal {

// ClassModel: Validates the class name in setData() and
// refuses placeholders and invalid characters.
class ClassModel : public QStandardItemModel {
public:
    explicit ClassModel(QObject *parent = nullptr);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void appendPlaceHolder() { appendClass(m_newClassPlaceHolder); }

    QModelIndex placeHolderIndex() const;
    QString newClassPlaceHolder() const { return m_newClassPlaceHolder; }

private:
    void appendClass(const QString &);

    QRegularExpression m_validator;
    const QString m_newClassPlaceHolder;
};

ClassModel::ClassModel(QObject *parent) :
    QStandardItemModel(0, 1, parent),
    m_validator(QLatin1String("^[a-zA-Z][a-zA-Z0-9_]*$")),
    m_newClassPlaceHolder(ClassList::tr("<New class>"))
{
    QTC_ASSERT(m_validator.isValid(), return);
    appendPlaceHolder();
}

void ClassModel::appendClass(const QString &c)
{
    auto *item = new QStandardItem(c);
    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable);
    appendRow(item);
}

bool ClassModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && !m_validator.match(value.toString()).hasMatch())
        return false;
    return QStandardItemModel::setData(index, value, role);
}

QModelIndex ClassModel::placeHolderIndex() const
{
    return index(rowCount() - 1, 0);
}

// --------------- ClassList
ClassList::ClassList(QWidget *parent) :
    QListView(parent),
    m_model(new ClassModel)
{
    setModel(m_model);
    connect(itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &ClassList::classEdited);
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ClassList::slotCurrentRowChanged);
}

void ClassList::startEditingNewClassItem()
{
    // Start editing the 'new class' item.
    setFocus();

    const QModelIndex index = m_model->placeHolderIndex();
    setCurrentIndex(index);
    edit(index);
}

QString ClassList::className(int row) const
{
    return m_model->item(row, 0)->text();
}

void ClassList::classEdited()
{
    const QModelIndex index = currentIndex();
    QTC_ASSERT(index.isValid(), return);

    const QString name = className(index.row());
    if (index == m_model->placeHolderIndex()) {
        // Real name class entered.
        if (name != m_model->newClassPlaceHolder()) {
            emit classAdded(name);
            m_model->appendPlaceHolder();
        }
    } else {
        emit classRenamed(index.row(), name);
    }
}

void ClassList::removeCurrentClass()
{
    const QModelIndex index = currentIndex();
    if (!index.isValid() || index == m_model->placeHolderIndex())
        return;
    if (QMessageBox::question(this,
                              tr("Confirm Delete"),
                              tr("Delete class %1 from list?").arg(className(index.row())),
                              QMessageBox::Ok|QMessageBox::Cancel) != QMessageBox::Ok)
        return;
    // Delete row and set current on same item.
    m_model->removeRows(index.row(), 1);
    emit classDeleted(index.row());
    setCurrentIndex(m_model->indexFromItem(m_model->item(index.row(), 0)));
}

void ClassList::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        removeCurrentClass();
        break;
    case Qt::Key_Insert:
        startEditingNewClassItem();
        break;
    default:
        QListView::keyPressEvent(event);
        break;
    }
}

void ClassList::slotCurrentRowChanged(const QModelIndex &current, const QModelIndex &)
{
    emit currentRowChanged(current.row());
}

} // namespace Internal
} // namespace QmakeProjectManager
