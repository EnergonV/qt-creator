// Copyright (C) 2016 Lorenz Haas
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "../beautifierabstracttool.h"

#include "artisticstyleoptionspage.h"
#include "artisticstylesettings.h"

namespace Beautifier {
namespace Internal {

class ArtisticStyle : public BeautifierAbstractTool
{
    Q_OBJECT

public:
    ArtisticStyle();

    QString id() const override;
    void updateActions(Core::IEditor *editor) override;
    TextEditor::Command command() const override;
    bool isApplicable(const Core::IDocument *document) const override;

private:
    void formatFile();
    QString configurationFile() const;
    TextEditor::Command command(const QString &cfgFile) const;

    QAction *m_formatFile = nullptr;
    ArtisticStyleSettings m_settings;
    ArtisticStyleOptionsPage m_page{&m_settings};
};

} // namespace Internal
} // namespace Beautifier
