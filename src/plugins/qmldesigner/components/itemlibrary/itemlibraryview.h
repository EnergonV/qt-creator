// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <abstractview.h>

#include <QPointer>

namespace QmlDesigner {

class ItemLibraryWidget;

class ItemLibraryView : public AbstractView
{
    Q_OBJECT

public:
    ItemLibraryView(class AsynchronousImageCache &imageCache);
    ~ItemLibraryView() override;

    bool hasWidget() const override;
    WidgetInfo widgetInfo() override;

    // AbstractView
    void modelAttached(Model *model) override;
    void modelAboutToBeDetached(Model *model) override;
    void importsChanged(const QList<Import> &addedImports, const QList<Import> &removedImports) override;
    void possibleImportsChanged(const QList<Import> &possibleImports) override;
    void usedImportsChanged(const QList<Import> &usedImports) override;
    void documentMessagesChanged(const QList<DocumentMessage> &errors, const QList<DocumentMessage> &warnings) override;
    void updateImport3DSupport(const QVariantMap &supportMap) override;
    void customNotification(const AbstractView *view, const QString &identifier,
                            const QList<ModelNode> &nodeList, const QList<QVariant> &data) override;

protected:
    void updateImports();

private:
    AsynchronousImageCache &m_imageCache;
    QPointer<ItemLibraryWidget> m_widget;
    bool m_hasErrors = false;
    QVariantMap m_importableExtensions3DMap;
    QVariantMap m_importOptions3DMap;
};

}
