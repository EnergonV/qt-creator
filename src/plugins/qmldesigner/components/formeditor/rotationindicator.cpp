// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "rotationindicator.h"

#include "formeditoritem.h"

namespace QmlDesigner {

RotationIndicator::RotationIndicator(LayerItem *layerItem)
    : m_layerItem(layerItem)
{
    Q_ASSERT(layerItem);
}

RotationIndicator::~RotationIndicator()
{
    m_itemControllerHash.clear();
}

void RotationIndicator::show()
{
    for (RotationController controller : qAsConst(m_itemControllerHash))
        controller.show();
}

void RotationIndicator::hide()
{
    for (RotationController controller : qAsConst(m_itemControllerHash))
        controller.hide();
}

void RotationIndicator::clear()
{
    m_itemControllerHash.clear();
}

static bool itemIsRotatable(const QmlItemNode &qmlItemNode)
{
    return qmlItemNode.isValid()
            && qmlItemNode.instanceIsResizable()
            && qmlItemNode.modelIsMovable()
            && qmlItemNode.modelIsRotatable()
            && !qmlItemNode.instanceIsInLayoutable()
            && !qmlItemNode.isFlowItem();
}

void RotationIndicator::setItems(const QList<FormEditorItem*> &itemList)
{
    clear();

    for (FormEditorItem *item : itemList) {
        if (item && itemIsRotatable(item->qmlItemNode())) {
            RotationController controller(m_layerItem, item);
            m_itemControllerHash.insert(item, controller);
        }
    }
}

void RotationIndicator::updateItems(const QList<FormEditorItem*> &itemList)
{
    for (FormEditorItem *item : itemList) {
        if (m_itemControllerHash.contains(item)) {
            if (!item || !itemIsRotatable(item->qmlItemNode())) {
                m_itemControllerHash.take(item);
            } else {
                RotationController controller(m_itemControllerHash.value(item));
                controller.updatePosition();
            }
        } else if (item && itemIsRotatable(item->qmlItemNode())) {
            RotationController controller(m_layerItem, item);
            m_itemControllerHash.insert(item, controller);
        }
    }
}

}
