// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QAbstractListModel>
#include <import.h>

#include <QSet>

namespace QmlDesigner {

class ItemLibraryEntry;

class ItemLibraryAddImportModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ItemLibraryAddImportModel(QObject *parent = nullptr);
    ~ItemLibraryAddImportModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void update(const QList<Import> &possibleImports);
    void setSearchText(const QString &searchText);
    Import getImportAt(int index) const;

    void setPriorityImports(const QSet<QString> &priorityImports);
    Import getImport(const QString &importUrl) const;

private:
    QString m_searchText;
    QList<Import> m_importList;
    QSet<QString> m_importFilterList;
    QHash<int, QByteArray> m_roleNames;
    QSet<QString> m_priorityImports;
};

} // namespace QmlDesigner
