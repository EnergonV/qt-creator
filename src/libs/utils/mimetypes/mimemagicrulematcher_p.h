// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR LGPL-3.0

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "mimemagicrule_p.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>


namespace Utils {
namespace Internal {

class MimeMagicRuleMatcher
{
public:
    explicit MimeMagicRuleMatcher(const QString &mime, unsigned priority = 65535);

    bool operator==(const MimeMagicRuleMatcher &other) const;

    void addRule(const MimeMagicRule &rule);
    void addRules(const QList<MimeMagicRule> &rules);
    QList<MimeMagicRule> magicRules() const;

    bool matches(const QByteArray &data) const;

    unsigned priority() const;

    QString mimetype() const { return m_mimetype; }

private:
    QList<MimeMagicRule> m_list;
    unsigned m_priority;
    QString m_mimetype;
};

} // Internal
} // Utils
