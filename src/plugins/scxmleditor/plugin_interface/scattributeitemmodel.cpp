// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "scattributeitemmodel.h"
#include "mytypes.h"

#include <QBrush>

using namespace ScxmlEditor::PluginInterface;

SCAttributeItemModel::SCAttributeItemModel(QObject *parent)
    : AttributeItemModel(parent)
{
}

QVariant SCAttributeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return section == 0 ? tr("Name") : tr("Value");

    return QVariant();
}

bool SCAttributeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || !m_tag)
        return false;

    bool bEditable = m_tag->tagType() <= MetadataItem;

    if (index.row() >= 0 && m_document) {
        if (!bEditable) {
            if (index.row() < m_tag->info()->n_attributes)
                m_document->setValue(m_tag, index.row(), value.toString());
        } else {
            if (index.column() == 0) {
                m_tag->setAttributeName(index.row(), value.toString());
                m_document->setValue(m_tag, value.toString(), m_tag->attribute(value.toString()));
            } else
                m_document->setValue(m_tag, m_tag->attributeName(index.row()), value.toString());
        }
        emit dataChanged(index, index);
        emit layoutChanged();
        return true;
    }

    return false;
}

QVariant SCAttributeItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_tag)
        return QVariant();

    if (index.row() < 0)
        return QVariant();

    bool bEditable = m_tag->tagType() <= MetadataItem;

    if (!bEditable && index.row() >= m_tag->info()->n_attributes)
        return QVariant();

    if (bEditable && index.row() > m_tag->attributeCount())
        return QVariant();

    bool bExtraRow = bEditable && m_tag->attributeCount() == index.row();

    switch (role) {
    case Qt::DisplayRole:
        if (bExtraRow)
            return index.column() == 0 ? tr("- name -") : tr(" - value -");
        Q_FALLTHROUGH();
    case Qt::EditRole: {
        if (index.column() == 0) {
            if (bEditable) {
                return m_tag->attributeName(index.row());
            } else {
                scxmltag_attribute_t attr = m_tag->info()->attributes[index.row()];
                if (attr.required)
                    return QString::fromLatin1("*%1").arg(QLatin1String(attr.name));
                else
                    return QString::fromLatin1(attr.name);
            }
        } else {
            if (bEditable) {
                if (m_tag->tagType() > MetadataItem && m_tag->info()->attributes[index.row()].datatype == QVariant::StringList)
                    return QString::fromLatin1(m_tag->info()->attributes[index.row()].value).split(";");
                else
                    return m_tag->attribute(index.row());
            } else {
                return m_tag->attribute(QLatin1String(m_tag->info()->attributes[index.row()].name));
            }
        }
    }
    case Qt::TextAlignmentRole:
        if (bExtraRow)
            return Qt::AlignHCenter;
        else
            break;
    case DataTypeRole: {
        if (m_tag->tagType() == Metadata || m_tag->tagType() == MetadataItem)
            return (int)QVariant::String;
        else if (index.column() == 1 && m_tag->info()->n_attributes > 0)
            return m_tag->info()->attributes[index.row()].datatype;
        else
            return QVariant::Invalid;
    }
    case DataRole: {
        if (m_tag->info()->n_attributes > 0)
            return QString::fromLatin1(m_tag->info()->attributes[index.row()].value);
        else
            return QVariant();
    }
    default:
        break;
    }

    return QVariant();
}

Qt::ItemFlags SCAttributeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !m_tag)
        return Qt::NoItemFlags;

    if (m_tag->tagType() <= MetadataItem || (index.column() == 1 && m_tag->info()->n_attributes > 0 && m_tag->info()->attributes[index.row()].editable))
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

    return index.column() == 0 ? Qt::ItemIsEnabled : Qt::NoItemFlags;
}

int SCAttributeItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

int SCAttributeItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_tag) {
        if (m_tag->tagType() <= MetadataItem)
            return m_tag->attributeCount() + 1;
        else
            return m_tag->info()->n_attributes;
    }

    return 0;
}
