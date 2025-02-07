// Copyright (C) 2016 Lorenz Haas
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "generalsettings.h"

#include "beautifierconstants.h"

#include <coreplugin/icore.h>
#include <utils/algorithm.h>
#include <utils/genericconstants.h>
#include <utils/mimeutils.h>

namespace Beautifier {
namespace Internal {

namespace {
const char AUTO_FORMAT_TOOL[]                 = "autoFormatTool";
const char AUTO_FORMAT_MIME[]                 = "autoFormatMime";
const char AUTO_FORMAT_ONLY_CURRENT_PROJECT[] = "autoFormatOnlyCurrentProject";
}

static GeneralSettings *m_instance;

GeneralSettings::GeneralSettings()
{
    m_instance = this;
    read();
}

GeneralSettings *GeneralSettings::instance()
{
    return m_instance;
}

void GeneralSettings::read()
{
    QSettings *s = Core::ICore::settings();
    s->beginGroup(Utils::Constants::BEAUTIFIER_SETTINGS_GROUP);
    s->beginGroup(Utils::Constants::BEAUTIFIER_GENERAL_GROUP);
    m_autoFormatOnSave = s->value(Utils::Constants::BEAUTIFIER_AUTO_FORMAT_ON_SAVE, false).toBool();
    m_autoFormatTool = s->value(AUTO_FORMAT_TOOL, QString()).toString();
    setAutoFormatMime(s->value(AUTO_FORMAT_MIME, "text/x-c++src;text/x-c++hdr").toString());
    m_autoFormatOnlyCurrentProject = s->value(AUTO_FORMAT_ONLY_CURRENT_PROJECT, true).toBool();
    s->endGroup();
    s->endGroup();
}

void GeneralSettings::save()
{
    QSettings *s = Core::ICore::settings();
    s->beginGroup(Utils::Constants::BEAUTIFIER_SETTINGS_GROUP);
    s->beginGroup(Utils::Constants::BEAUTIFIER_GENERAL_GROUP);
    s->setValue(Utils::Constants::BEAUTIFIER_AUTO_FORMAT_ON_SAVE, m_autoFormatOnSave);
    s->setValue(AUTO_FORMAT_TOOL, m_autoFormatTool);
    s->setValue(AUTO_FORMAT_MIME, autoFormatMimeAsString());
    s->setValue(AUTO_FORMAT_ONLY_CURRENT_PROJECT, m_autoFormatOnlyCurrentProject);
    s->endGroup();
    s->endGroup();
}

bool GeneralSettings::autoFormatOnSave() const
{
    return m_autoFormatOnSave;
}

void GeneralSettings::setAutoFormatOnSave(bool autoFormatOnSave)
{
    m_autoFormatOnSave = autoFormatOnSave;
}

QString GeneralSettings::autoFormatTool() const
{
    return m_autoFormatTool;
}

void GeneralSettings::setAutoFormatTool(const QString &autoFormatTool)
{
    m_autoFormatTool = autoFormatTool;
}

QList<Utils::MimeType> GeneralSettings::autoFormatMime() const
{
    return m_autoFormatMime;
}

QString GeneralSettings::autoFormatMimeAsString() const
{
    return Utils::transform(m_autoFormatMime, &Utils::MimeType::name).join("; ");
}

void GeneralSettings::setAutoFormatMime(const QList<Utils::MimeType> &autoFormatMime)
{
    m_autoFormatMime = autoFormatMime;
}

void GeneralSettings::setAutoFormatMime(const QString &mimeList)
{
    const QStringList stringTypes = mimeList.split(';');
    QList<Utils::MimeType> types;
    types.reserve(stringTypes.count());
    for (QString t : stringTypes) {
        t = t.trimmed();
        const Utils::MimeType mime = Utils::mimeTypeForName(t);
        if (mime.isValid())
            types << mime;
    }
    setAutoFormatMime(types);
}

bool GeneralSettings::autoFormatOnlyCurrentProject() const
{
    return m_autoFormatOnlyCurrentProject;
}

void GeneralSettings::setAutoFormatOnlyCurrentProject(bool autoFormatOnlyCurrentProject)
{
    m_autoFormatOnlyCurrentProject = autoFormatOnlyCurrentProject;
}

} // namespace Internal
} // namespace Beautifier
