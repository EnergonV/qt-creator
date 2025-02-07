// Copyright (C) 2018 Sergey Morozov
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/filepath.h>
#include <utils/qtcprocess.h>

#include <QHash>
#include <QTimer>

namespace Cppcheck {
namespace Internal {

class CppcheckTool;

class CppcheckRunner final : public QObject
{
    Q_OBJECT

public:
    explicit CppcheckRunner(CppcheckTool &tool);
    ~CppcheckRunner() override;

    void reconfigure(const Utils::FilePath &binary, const QString &arguments);
    void addToQueue(const Utils::FilePaths &files,
                    const QString &additionalArguments = {});
    void removeFromQueue(const Utils::FilePaths &files);
    void stop(const Utils::FilePaths &files = {});

    const Utils::FilePaths &currentFiles() const;
    QString currentCommand() const;

private:
    void checkQueued();
    void readOutput();
    void readError();
    void handleDone();

    CppcheckTool &m_tool;
    Utils::QtcProcess m_process;
    Utils::FilePath m_binary;
    QString m_arguments;
    QHash<QString, Utils::FilePaths> m_queue;
    Utils::FilePaths m_currentFiles;
    QTimer m_queueTimer;
    int m_maxArgumentsLength = 32767;
};

} // namespace Internal
} // namespace Cppcheck
