// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/settingsaccessor.h>

#include <QList>

namespace ProjectExplorer {

class ToolChain;

namespace Internal {

class ToolChainSettingsAccessor : public Utils::UpgradingSettingsAccessor
{
public:
    ToolChainSettingsAccessor();

    QList<ToolChain *> restoreToolChains(QWidget *parent) const;

    void saveToolChains(const QList<ToolChain *> &toolchains, QWidget *parent);

private:
    QList<ToolChain *> toolChains(const QVariantMap &data) const;
};

} // namespace Internal
} // namespace ProjectExplorer
