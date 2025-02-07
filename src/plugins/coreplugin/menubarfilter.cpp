// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "menubarfilter.h"

#include "actionmanager/actioncontainer.h"
#include "actionmanager/actionmanager.h"
#include "coreconstants.h"
#include "icore.h"
#include "locator/locatormanager.h"

#include <utils/algorithm.h>
#include <utils/qtcassert.h>
#include <utils/stringutils.h>

#include <QMenuBar>
#include <QPointer>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
size_t qHash(const QPointer<QAction> &p, size_t seed)
{
    return qHash(p.data(), seed);
}
QT_END_NAMESPACE

using namespace Core::Internal;
using namespace Core;

MenuBarFilter::MenuBarFilter()
{
    setId("Actions from the menu");
    setDisplayName(tr("Actions from the Menu"));
    setDescription(
        tr("Triggers an action from the menu. Matches any part of a menu hierarchy, separated by "
           "\">\". For example \"sess def\" matches \"File > Sessions > Default\"."));
    setDefaultShortcutString("t");
    connect(ICore::instance(), &ICore::contextAboutToChange, this, [this] {
        if (LocatorManager::locatorHasFocus())
            updateEnabledActionCache();
    });
}

static const QList<QAction *> menuBarActions()
{
    QMenuBar *menuBar = Core::ActionManager::actionContainer(Constants::MENU_BAR)->menuBar();
    QTC_ASSERT(menuBar, return {});
    return menuBar->actions();
}

QList<LocatorFilterEntry> MenuBarFilter::matchesFor(QFutureInterface<LocatorFilterEntry> &future,
                                                    const QString &entry)
{
    Q_UNUSED(future)
    Q_UNUSED(entry)
    return std::move(m_entries);
}

void MenuBarFilter::accept(const LocatorFilterEntry &selection, QString *newText,
                           int *selectionStart, int *selectionLength) const
{
    Q_UNUSED(newText)
    Q_UNUSED(selectionStart)
    Q_UNUSED(selectionLength)
    if (auto action = selection.internalData.value<QPointer<QAction>>()) {
        QMetaObject::invokeMethod(action, [action] {
            if (action->isEnabled())
                action->trigger();
        }, Qt::QueuedConnection);
    }
}

QList<LocatorFilterEntry> MenuBarFilter::matchesForAction(QAction *action,
                                                          const QStringList &entryPath,
                                                          const QStringList &path,
                                                          QVector<const QMenu *> &processedMenus)
{
    QList<LocatorFilterEntry> entries;
    if (!m_enabledActions.contains(action))
        return entries;
    const QString whatsThis = action->whatsThis();
    const QString text = Utils::stripAccelerator(action->text())
                         + (whatsThis.isEmpty() ? QString() : QString(" (" + whatsThis + ")"));
    if (QMenu *menu = action->menu()) {
        if (processedMenus.contains(menu))
            return entries;
        processedMenus.append(menu);
        if (menu->isEnabled()) {
            const QList<QAction *> &actions = menu->actions();
            QStringList menuPath(path);
            menuPath << text;
            for (QAction *menuAction : actions)
                entries << matchesForAction(menuAction, entryPath, menuPath, processedMenus);
        }
    } else if (!text.isEmpty()) {
        int entryIndex = 0;
        int entryLength = 0;
        int pathIndex = 0;
        LocatorFilterEntry::HighlightInfo::DataType highlightType =
                LocatorFilterEntry::HighlightInfo::DisplayName;
        const QString pathText = path.join(" > ");
        QStringList actionPath(path);
        if (!entryPath.isEmpty()) {
            actionPath << text;
            for (const QString &entry : entryPath) {
                const QRegularExpression re(".*" + entry + ".*",
                                            QRegularExpression::CaseInsensitiveOption);
                pathIndex = actionPath.indexOf(re, pathIndex);
                if (pathIndex < 0)
                    return entries;
            }
            const QString &lastEntry(entryPath.last());
            entryLength = lastEntry.length();
            entryIndex = text.indexOf(lastEntry, 0, Qt::CaseInsensitive);
            if (entryIndex >= 0) {
                highlightType = LocatorFilterEntry::HighlightInfo::DisplayName;
            } else {
                entryIndex = pathText.indexOf(lastEntry, 0, Qt::CaseInsensitive);
                QTC_ASSERT(entryIndex >= 0, return entries);
                highlightType = LocatorFilterEntry::HighlightInfo::ExtraInfo;
            }
        }
        LocatorFilterEntry filterEntry(this, text, QVariant(), action->icon());
        filterEntry.internalData.setValue(QPointer<QAction>(action));
        filterEntry.extraInfo = pathText;
        filterEntry.highlightInfo = {entryIndex, entryLength, highlightType};
        entries << filterEntry;
    }
    return entries;
}

static void requestMenuUpdate(const QAction* action)
{
    if (QMenu *menu = action->menu()) {
        emit menu->aboutToShow();
        const QList<QAction *> &actions = menu->actions();
        for (const QAction *menuActions : actions)
            requestMenuUpdate(menuActions);
    }
}

void MenuBarFilter::updateEnabledActionCache()
{
    m_enabledActions.clear();
    QList<QAction *> queue = menuBarActions();
    for (QAction *action : qAsConst(queue))
        requestMenuUpdate(action);
    while (!queue.isEmpty()) {
        QAction *action = queue.takeFirst();
        if (action->isEnabled()) {
            m_enabledActions.insert(action);
            if (QMenu *menu = action->menu()) {
                if (menu->isEnabled())
                    queue.append(menu->actions());
            }
        }
    }
}

void Core::Internal::MenuBarFilter::prepareSearch(const QString &entry)
{
    Q_UNUSED(entry)
    static const QString separators = ". >/";
    static const QRegularExpression seperatorRegExp(QString("[%1]").arg(separators));
    QString normalized = entry;
    normalized.replace(seperatorRegExp, separators.at(0));
    const QStringList entryPath = normalized.split(separators.at(0), Qt::SkipEmptyParts);
    m_entries.clear();
    QVector<const QMenu *> processedMenus;
    for (QAction* action : menuBarActions())
        m_entries << matchesForAction(action, entryPath, QStringList(), processedMenus);
}
