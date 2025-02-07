// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/optional.h>

#include <QColor>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>

namespace Core {
namespace Internal {

struct FilterRuleSpec
{
    QString category;
    Utils::optional<QtMsgType> level;
    bool enabled;
};

class LoggingCategoryEntry
{
public:
    QtMsgType level = QtDebugMsg;
    bool enabled = false;
    QColor color;
};

class LoggingViewManager : public QObject
{
    Q_OBJECT
public:
    static inline QString messageTypeToString(QtMsgType type)
    {
        switch (type) {
        case QtDebugMsg: return {"Debug"};
        case QtInfoMsg: return {"Info"};
        case QtCriticalMsg: return {"Critical"};
        case QtWarningMsg: return {"Warning"};
        case QtFatalMsg: return {"Fatal"};
        default: return {"Unknown"};
        }
    }

    static inline QtMsgType messageTypeFromString(const QString &type)
    {
        if (type.isEmpty())
            return QtDebugMsg;

        // shortcut - only handle expected
        switch (type.at(0).toLatin1()) {
        case 'I':
            return QtInfoMsg;
        case 'C':
            return QtCriticalMsg;
        case 'W':
            return QtWarningMsg;
        case 'D':
        default:
            return QtDebugMsg;
        }
    }

    explicit LoggingViewManager(QObject *parent = nullptr);
    ~LoggingViewManager();

    static LoggingViewManager *instance();

    static inline bool enabled(QtMsgType current, QtMsgType stored)
    {
        if (stored == QtMsgType::QtInfoMsg)
            return true;
        if (current == stored)
            return true;
        if (stored == QtMsgType::QtDebugMsg)
            return current != QtMsgType::QtInfoMsg;
        if (stored == QtMsgType::QtWarningMsg)
            return current == QtMsgType::QtCriticalMsg || current == QtMsgType::QtFatalMsg;
        if (stored == QtMsgType::QtCriticalMsg)
            return current == QtMsgType::QtFatalMsg;
        return false;
    }

    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                  const QString &mssg);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    bool isCategoryEnabled(const QString &category);
    void setCategoryEnabled(const QString &category, bool enabled);
    void setLogLevel(const QString &category, QtMsgType type);
    void setListQtInternal(bool listQtInternal);
    QList<FilterRuleSpec> originalRules() const { return m_originalRules; }

    QMap<QString, LoggingCategoryEntry> categories() const { return m_categories; }
    void appendOrUpdate(const QString &category, const LoggingCategoryEntry &entry);

signals:
    void receivedLog(const QString &timestamp, const QString &type, const QString &category,
                     const QString &msg);
    void foundNewCategory(const QString &category, const LoggingCategoryEntry &entry);
    void updatedCategory(const QString &category, const LoggingCategoryEntry &entry);

private:
    void prefillCategories();
    void resetFilterRules();
    bool enabledInOriginalRules(const QMessageLogContext &context, QtMsgType type);

    QMap<QString, LoggingCategoryEntry> m_categories;
    const QString m_originalLoggingRules;
    QList<FilterRuleSpec> m_originalRules;
    bool m_enabled = false;
    bool m_listQtInternal = false;
};

} // namespace Internal
} // namespace Core

Q_DECLARE_METATYPE(Core::Internal::LoggingCategoryEntry)
