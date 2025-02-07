// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "qmldesignercorelib_global.h"
#include "abstractproperty.h"

namespace QmlDesigner {

class QMLDESIGNERCORE_EXPORT SignalHandlerProperty : public AbstractProperty
{
    friend ModelNode;
    friend Internal::ModelPrivate;
    friend AbstractProperty;

public:
    void setSource(const QString &source);
    QString source() const;

    SignalHandlerProperty();
    SignalHandlerProperty(const SignalHandlerProperty &property, AbstractView *view);

    static PropertyName prefixAdded(const PropertyName &propertyName);
    static PropertyName prefixRemoved(const PropertyName &propertyName);

protected:
    SignalHandlerProperty(const PropertyName &propertyName, const Internal::InternalNodePointer &internalNode, Model* model, AbstractView *view);
};

class QMLDESIGNERCORE_EXPORT SignalDeclarationProperty : public AbstractProperty
{
    friend ModelNode;
    friend Internal::ModelPrivate;
    friend AbstractProperty;

public:
    void setSignature(const QString &source);
    QString signature() const;

    SignalDeclarationProperty();
    SignalDeclarationProperty(const SignalDeclarationProperty &property, AbstractView *view);

protected:
    SignalDeclarationProperty(const PropertyName &propertyName, const Internal::InternalNodePointer &internalNode, Model* model, AbstractView *view);
};

} // namespace QmlDesigner
