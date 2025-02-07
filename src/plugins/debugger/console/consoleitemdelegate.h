// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "consoleitemmodel.h"

#include <QStyledItemDelegate>

QT_FORWARD_DECLARE_CLASS(QTextLayout)

namespace Debugger::Internal {

class ConsoleItemDelegate : public QStyledItemDelegate
{
public:
    ConsoleItemDelegate(ConsoleItemModel *model, QObject *parent);

    void emitSizeHintChanged(const QModelIndex &index);
    QColor drawBackground(QPainter *painter, const QRect &rect, const QModelIndex &index,
                          const QStyleOptionViewItem &opt) const;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    qreal layoutText(QTextLayout &tl, int width, bool *success = nullptr) const;

private:
    ConsoleItemModel *m_model;
    const QIcon m_logIcon;
    const QIcon m_warningIcon;
    const QIcon m_errorIcon;
    const QIcon m_expandIcon;
    const QIcon m_collapseIcon;
    const QIcon m_prompt;
    mutable int m_cachedHeight = 0;
    mutable QFont m_cachedFont;
};

/*
  +----------------------------------------------------------------------------+
  | TYPEICONAREA  EXPANDABLEICONAREA  TEXTAREA               FILEAREA LINEAREA |
  +----------------------------------------------------------------------------+

 */
/*
  +----------------------------------------------------------------------------+
  | PROMPTAREA  EDITABLEAREA                                                   |
  +----------------------------------------------------------------------------+

 */
class ConsoleItemPositions
{
public:
    ConsoleItemPositions(ConsoleItemModel *model, const QRect &rect,
            const QFont &font, bool showTaskIconArea, bool showExpandableIconArea)
        : m_x(rect.x()),
          m_width(rect.width()),
          m_top(rect.top()),
          m_bottom(rect.bottom()),
          m_showTaskIconArea(showTaskIconArea),
          m_showExpandableIconArea(showExpandableIconArea)
    {
        m_fontHeight = QFontMetrics(font).height();
        m_maxFileLength = model->sizeOfFile(font);
        m_maxLineLength = model->sizeOfLineNumber(font);
    }

    int adjustedTop() const { return m_top + ITEM_PADDING; }
    int adjustedLeft() const { return m_x + ITEM_PADDING; }
    int adjustedRight() const { return m_width - ITEM_PADDING; }
    int adjustedBottom() const { return m_bottom; }
    int lineHeight() const { return m_fontHeight + 1; }
    int minimumHeight() const { return typeIconHeight() + 2 * ITEM_PADDING; }

    // PROMPTAREA is same as TYPEICONAREA
    int typeIconLeft() const { return adjustedLeft(); }
    int typeIconWidth() const { return TASK_ICON_SIZE; }
    int typeIconHeight() const { return TASK_ICON_SIZE; }
    int typeIconRight() const { return m_showTaskIconArea ? typeIconLeft() + typeIconWidth()
                                                          : adjustedLeft(); }
    QRect typeIcon() const { return QRect(typeIconLeft(), adjustedTop(), typeIconWidth(),
                                          typeIconHeight()); }

    int expandCollapseIconLeft() const { return typeIconRight() + ITEM_SPACING; }
    int expandCollapseIconWidth() const { return TASK_ICON_SIZE; }
    int expandCollapseIconHeight() const { return TASK_ICON_SIZE; }
    int expandCollapseIconRight() const { return m_showExpandableIconArea ?
                    expandCollapseIconLeft() + expandCollapseIconWidth() : typeIconRight(); }
    QRect expandCollapseIcon() const { return QRect(expandCollapseIconLeft(), adjustedTop(),
                                                    expandCollapseIconWidth(),
                                                    expandCollapseIconHeight()); }

    int textAreaLeft() const { return  expandCollapseIconRight() + ITEM_SPACING; }
    int textAreaWidth() const { return textAreaRight() - textAreaLeft(); }
    int textAreaRight() const { return fileAreaLeft() - ITEM_SPACING; }
    QRect textArea() const { return QRect(textAreaLeft(), adjustedTop(), textAreaWidth(),
                                          lineHeight()); }

    int fileAreaLeft() const { return fileAreaRight() - fileAreaWidth(); }
    int fileAreaWidth() const { return m_maxFileLength; }
    int fileAreaRight() const { return lineAreaLeft() - ITEM_SPACING; }
    QRect fileArea() const { return QRect(fileAreaLeft(), adjustedTop(), fileAreaWidth(),
                                          lineHeight()); }

    int lineAreaLeft() const { return lineAreaRight() - lineAreaWidth(); }
    int lineAreaWidth() const { return m_maxLineLength; }
    int lineAreaRight() const { return adjustedRight() - ITEM_SPACING; }
    QRect lineArea() const { return QRect(lineAreaLeft(), adjustedTop(), lineAreaWidth(),
                                          lineHeight()); }

private:
    int m_x;
    int m_width;
    int m_top;
    int m_bottom;
    int m_fontHeight;
    int m_maxFileLength;
    int m_maxLineLength;
    bool m_showTaskIconArea;
    bool m_showExpandableIconArea;

public:
    static const int TASK_ICON_SIZE = 16;
    static const int ITEM_PADDING = 8;
    static const int ITEM_SPACING = 4;
};

} // Debugger::Internal
