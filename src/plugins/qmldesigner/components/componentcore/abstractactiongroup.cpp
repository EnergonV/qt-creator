// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "abstractactiongroup.h"

#include <QMenu>

namespace QmlDesigner {

AbstractActionGroup::AbstractActionGroup(const QString &displayName) :
    m_displayName(displayName),
    m_menu(new QMenu)
{
    m_menu->setTitle(displayName);
    m_action = m_menu->menuAction();
}

ActionInterface::Type AbstractActionGroup::type() const
{
    return ActionInterface::ContextMenu;
}

QAction *AbstractActionGroup::action() const
{
    return m_action;
}

QMenu *AbstractActionGroup::menu() const
{
    return m_menu.data();
}

SelectionContext AbstractActionGroup::selectionContext() const
{
    return m_selectionContext;
}

void AbstractActionGroup::currentContextChanged(const SelectionContext &selectionContext)
{
    m_selectionContext = selectionContext;
    updateContext();
}

void AbstractActionGroup::updateContext()
{
    if (m_selectionContext.isValid()) {
        m_action->setEnabled(isEnabled(m_selectionContext));
        m_action->setVisible(isVisible(m_selectionContext));
    }
}

} // namespace QmlDesigner
