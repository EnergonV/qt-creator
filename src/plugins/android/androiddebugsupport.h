// Copyright (C) 2016 BogDan Vatra <bog_dan_ro@yahoo.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "androidrunner.h"
#include <debugger/debuggerruncontrol.h>

namespace Android {
namespace Internal {

class AndroidDebugSupport : public Debugger::DebuggerRunTool
{
    Q_OBJECT

public:
    AndroidDebugSupport(ProjectExplorer::RunControl *runControl,
                        const QString &intentName = QString());

    void start() override;
    void stop() override;

private:
    AndroidRunner *m_runner = nullptr;
};

} // namespace Internal
} // namespace Android
