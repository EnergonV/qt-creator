// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "colorsettings.h"
#include "scxmleditorconstants.h"

#include <QInputDialog>
#include <QMessageBox>

#include <coreplugin/icore.h>

using namespace ScxmlEditor::Common;

ColorSettings::ColorSettings(QWidget *parent)
    : QFrame(parent)
{
    m_ui.setupUi(this);

    m_ui.m_colorThemeView->setEnabled(false);

    const QSettings *s = Core::ICore::settings();

    m_colorThemes = s->value(Constants::C_SETTINGS_COLORSETTINGS_COLORTHEMES).toMap();

    m_ui.m_comboColorThemes->clear();
    for (auto it = m_colorThemes.cbegin(); it != m_colorThemes.cend(); ++it)
        m_ui.m_comboColorThemes->addItem(it.key());
    m_ui.m_comboColorThemes->setCurrentText(s->value(Constants::C_SETTINGS_COLORSETTINGS_CURRENTCOLORTHEME).toString());

    connect(m_ui.m_comboColorThemes,
            &QComboBox::currentIndexChanged,
            this,
            &ColorSettings::selectTheme);
    connect(m_ui.m_colorThemeView, &ColorThemeView::colorChanged, this, &ColorSettings::updateCurrentColors);
    connect(m_ui.m_addColorTheme, &QToolButton::clicked, this, &ColorSettings::createTheme);
    connect(m_ui.m_removeColorTheme, &QToolButton::clicked, this, &ColorSettings::removeTheme);
}

void ColorSettings::save()
{
    QSettings *s = Core::ICore::settings();
    s->setValue(Constants::C_SETTINGS_COLORSETTINGS_COLORTHEMES, m_colorThemes);
    s->setValue(Constants::C_SETTINGS_COLORSETTINGS_CURRENTCOLORTHEME, m_ui.m_comboColorThemes->currentText());
}

void ColorSettings::updateCurrentColors()
{
    m_colorThemes[m_ui.m_comboColorThemes->currentText()] = m_ui.m_colorThemeView->colorData();
}

void ColorSettings::selectTheme(int index)
{
    const QString name = m_ui.m_comboColorThemes->itemText(index);
    m_ui.m_colorThemeView->reset();
    if (!name.isEmpty() && m_colorThemes.contains(name)) {
        m_ui.m_colorThemeView->setEnabled(true);
        const QVariantMap colordata = m_colorThemes[name].toMap();
        for (auto it = colordata.cbegin(); it != colordata.cend(); ++it)
            m_ui.m_colorThemeView->setColor(it.key().toInt(), QColor(it.value().toString()));
    } else {
        m_ui.m_colorThemeView->setEnabled(false);
    }
}

void ColorSettings::createTheme()
{
    QString name = QInputDialog::getText(this, tr("Create New Color Theme"), tr("Theme ID"));
    if (!name.isEmpty()) {
        if (m_colorThemes.contains(name)) {
            QMessageBox::warning(this, tr("Cannot Create Theme"), tr("Theme %1 is already available.").arg(name));
        } else {
            m_ui.m_colorThemeView->reset();
            m_colorThemes[name] = QVariantMap();
            m_ui.m_comboColorThemes->addItem(name);
            m_ui.m_comboColorThemes->setCurrentText(name);
        }
    }
}

void ColorSettings::removeTheme()
{
    const QString name = m_ui.m_comboColorThemes->currentText();
    const QMessageBox::StandardButton result = QMessageBox::question(this, tr("Remove Color Theme"),
        tr("Are you sure you want to delete color theme %1?").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (result == QMessageBox::Yes) {
        m_ui.m_comboColorThemes->removeItem(m_ui.m_comboColorThemes->currentIndex());
        m_colorThemes.remove(name);
        m_ui.m_comboColorThemes->setCurrentIndex(0);
        if (m_colorThemes.isEmpty())
            m_ui.m_colorThemeView->setEnabled(false);
    }
}
