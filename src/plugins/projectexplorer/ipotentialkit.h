// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QObject>
#include <QMetaType>
#include "projectexplorer_export.h"

namespace ProjectExplorer {

class PROJECTEXPLORER_EXPORT IPotentialKit : public QObject
{
    Q_OBJECT

public:
    IPotentialKit();
    ~IPotentialKit() override;

    virtual QString displayName() const = 0;
    virtual void executeFromMenu() = 0;
    virtual QWidget *createWidget(QWidget *parent) const = 0;
    virtual bool isEnabled() const = 0;
};

}
