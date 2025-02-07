// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "resourceview.h"

#include <QString>
#include <QUndoCommand>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace ResourceEditor::Internal {

/*!
    \class ViewCommand

    Provides a base for \l ResourceView-related commands.
*/
class ViewCommand : public QUndoCommand
{
protected:
    ResourceView *m_view;

    ViewCommand(ResourceView *view);
    ~ViewCommand() override;
};

/*!
    \class ModelIndexViewCommand

    Provides a mean to store/restore a \l QModelIndex as it cannot
    be stored safely in most cases. This is an abstract class.
*/
class ModelIndexViewCommand : public ViewCommand
{
    int m_prefixArrayIndex;
    int m_fileArrayIndex;

protected:
    ModelIndexViewCommand(ResourceView *view);
    ~ModelIndexViewCommand() override;
    void storeIndex(const QModelIndex &index);
    QModelIndex makeIndex() const;
};

/*!
    \class ModifyPropertyCommand

    Modifies the name/prefix/language property of a prefix/file node.
*/
class ModifyPropertyCommand : public ModelIndexViewCommand
{
    ResourceView::NodeProperty m_property;
    QString m_before;
    QString m_after;
    int m_mergeId;

public:
    ModifyPropertyCommand(ResourceView *view, const QModelIndex &nodeIndex,
            ResourceView::NodeProperty property, const int mergeId, const QString &before,
            const QString &after = QString());

private:
    int id() const override { return m_mergeId; }
    bool mergeWith(const QUndoCommand * command) override;
    void undo() override;
    void redo() override;
};

/*!
    \class RemoveEntryCommand

    Removes a \l QModelIndex including all children from a \l ResourceView.
*/
class RemoveEntryCommand : public ModelIndexViewCommand
{
    EntryBackup *m_entry;
    bool m_isExpanded;

public:
    RemoveEntryCommand(ResourceView *view, const QModelIndex &index);
    ~RemoveEntryCommand() override;

private:
    void redo() override;
    void undo() override;
    void freeEntry();
};

/*!
    \class RemoveMultipleEntryCommand

    Removes multiple \l QModelIndex including all children from a \l ResourceView.
*/
class RemoveMultipleEntryCommand : public QUndoCommand
{
    std::vector<QUndoCommand *> m_subCommands;
public:
    // list must be in view order
    RemoveMultipleEntryCommand(ResourceView *view, const QList<QModelIndex> &list);
    ~RemoveMultipleEntryCommand() override;
private:
    void redo() override;
    void undo() override;
};


/*!
    \class AddFilesCommand

    Adds a list of files to a given prefix node.
*/
class AddFilesCommand : public ViewCommand
{
    int m_prefixIndex;
    int m_cursorFileIndex;
    int m_firstFile;
    int m_lastFile;
    const QStringList m_fileNames;

public:
    AddFilesCommand(ResourceView *view, int prefixIndex, int cursorFileIndex,
            const QStringList &fileNames);

private:
    void redo() override;
    void undo() override;
};

/*!
    \class AddEmptyPrefixCommand

    Adds a new, empty prefix node.
*/
class AddEmptyPrefixCommand : public ViewCommand
{
    int m_prefixArrayIndex;

public:
    AddEmptyPrefixCommand(ResourceView *view);

private:
    void redo() override;
    void undo() override;
};

} // ResourceEditor::Internal
