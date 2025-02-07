// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR LGPL-3.0

var stackView = [];

function push(p)
{
    if (!p)
        return
    stackView.push(p)
    __depth++
    return p
}

function pop()
{
    if (stackView.length === 0)
        return null
    var p = stackView.pop()
    __depth--
    return p
}

function current()
{
    if (stackView.length === 0)
        return null
    return stackView[stackView.length-1]
}

