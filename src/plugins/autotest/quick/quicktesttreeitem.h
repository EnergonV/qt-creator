// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "../testtreeitem.h"

namespace Autotest {
namespace Internal {

class QuickTestTreeItem : public TestTreeItem
{
public:
    explicit QuickTestTreeItem(ITestFramework *testFramework,
                               const QString &name = QString(),
                               const Utils::FilePath &filePath = Utils::FilePath(),
                               Type type = Root)
        : TestTreeItem(testFramework, name, filePath, type)
    {}

    TestTreeItem *copyWithoutChildren() override;
    QVariant data(int column, int role) const override;
    Qt::ItemFlags flags(int column) const override;
    bool canProvideTestConfiguration() const override;
    bool canProvideDebugConfiguration() const override;
    ITestConfiguration *testConfiguration() const override;
    ITestConfiguration *debugConfiguration() const override;
    QList<ITestConfiguration *> getAllTestConfigurations() const override;
    QList<ITestConfiguration *> getSelectedTestConfigurations() const override;
    QList<ITestConfiguration *> getFailedTestConfigurations() const override;
    QList<ITestConfiguration *> getTestConfigurationsForFile(const Utils::FilePath &fileName) const override;
    TestTreeItem *find(const TestParseResult *result) override;
    TestTreeItem *findChild(const TestTreeItem *other) override;
    bool modify(const TestParseResult *result) override;
    bool lessThan(const ITestTreeItem *other, SortMode mode) const override;
    bool isGroupNodeFor(const TestTreeItem *other) const override;
    bool removeOnSweepIfEmpty() const override;
    TestTreeItem *createParentGroupNode() const override;
    bool isGroupable() const override;
    void markForRemovalRecursively(const Utils::FilePath &filePath) override;
private:
    TestTreeItem *findChildByFileNameAndType(const Utils::FilePath &filePath, const QString &name,
                                             Type tType);
    TestTreeItem *findChildByNameFileAndLine(const QString &name, const Utils::FilePath &filePath,
                                             int line);
    TestTreeItem *unnamedQuickTests() const;
};

} // namespace Internal
} // namespace Autotest
