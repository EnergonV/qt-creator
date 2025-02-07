// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <remotelinux/linuxdevicetester.h>
#include <utils/qtcprocess.h>

#include <QStringList>

namespace Qnx {
namespace Internal {

class QnxDeviceTester : public ProjectExplorer::DeviceTester
{
    Q_OBJECT

public:
    explicit QnxDeviceTester(QObject *parent = nullptr);

    void testDevice(const ProjectExplorer::IDevice::Ptr &deviceConfiguration) override;
    void stopTest() override;

private:
    enum State {
        Inactive,
        GenericTest,
        VarRunTest,
        CommandsTest
    };

    void handleGenericTestFinished(ProjectExplorer::DeviceTester::TestResult result);
    void handleProcessDone();
    void handleVarRunDone();
    void handleCommandDone();

    void testNextCommand();
    void setFinished(ProjectExplorer::DeviceTester::TestResult result);

    QStringList versionSpecificCommandsToTest(int versionNumber) const;

    RemoteLinux::GenericLinuxDeviceTester *m_genericTester;
    ProjectExplorer::IDevice::ConstPtr m_deviceConfiguration;
    ProjectExplorer::DeviceTester::TestResult m_result = TestSuccess;
    State m_state = Inactive;

    int m_currentCommandIndex = 0;
    QStringList m_commandsToTest;
    Utils::QtcProcess m_process;
};

} // namespace Internal
} // namespace Qnx
