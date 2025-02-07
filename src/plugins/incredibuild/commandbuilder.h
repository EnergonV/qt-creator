// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <projectexplorer/buildstep.h>

#include <QCoreApplication>

namespace IncrediBuild {
namespace Internal {

class CommandBuilder
{
    Q_DECLARE_TR_FUNCTIONS(IncrediBuild::Internal::CommandBuilder)

public:
    CommandBuilder(ProjectExplorer::BuildStep *buildStep) : m_buildStep(buildStep) {}
    virtual ~CommandBuilder() = default;

    virtual QList<Utils::Id> migratableSteps() const { return {}; }

    ProjectExplorer::BuildStep *buildStep() const { return m_buildStep; }

    virtual QString id() const { return "CustomCommandBuilder"; }
    virtual QString displayName() const { return tr("Custom Command"); }

    virtual void fromMap(const QVariantMap &map);
    virtual void toMap(QVariantMap *map) const;

    virtual Utils::FilePath defaultCommand() const { return {}; }
    virtual QString defaultArguments() const { return QString(); }
    virtual QString setMultiProcessArg(QString args) { return args; }

    Utils::FilePath command() const { return m_command; }
    void setCommand(const Utils::FilePath &command);
    Utils::FilePath effectiveCommand() const { return m_command.isEmpty() ? defaultCommand() : m_command; }

    QString arguments() { return m_args.isEmpty() ? defaultArguments() : m_args; }
    void setArguments(const QString &arguments);

private:
    ProjectExplorer::BuildStep *m_buildStep{};
    Utils::FilePath m_command;
    QString m_args;
};

} // namespace Internal
} // namespace IncrediBuild
