// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "qmt/model_widgets_ui/propertiesviewmview.h"

namespace qmt { class ProjectController; }

namespace Utils {
class FilePath;
class PathChooser;
} // Utils

namespace ModelEditor {
namespace Internal {

class ExtPropertiesMView : public qmt::PropertiesView::MView
{
    Q_OBJECT

public:
    ExtPropertiesMView(qmt::PropertiesView *view);
    ~ExtPropertiesMView();

    void setProjectController(qmt::ProjectController *projectController);

    void visitMPackage(const qmt::MPackage *package) override;

private:
    void onConfigPathChanged(const Utils::FilePath &path);

private:
    qmt::ProjectController *m_projectController = nullptr;
    Utils::PathChooser *m_configPath = nullptr;
    QLabel *m_configPathInfo = nullptr;
};

} // namespace Interal
} // namespace ModelEditor
