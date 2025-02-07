// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <QGraphicsItem>

#include "qmt/diagram_scene/capabilities/resizable.h"
#include "qmt/diagram_scene/capabilities/moveable.h"
#include "qmt/diagram_scene/capabilities/selectable.h"
#include "qmt/diagram_scene/capabilities/editable.h"

namespace qmt {

class DAnnotation;
class DiagramSceneModel;
class RectangularSelectionItem;
class Style;

class AnnotationItem :
        public QGraphicsItem,
        public IResizable,
        public IMoveable,
        public ISelectable,
        public IEditable
{
    class AnnotationTextItem;

public:
    AnnotationItem(DAnnotation *annotation, DiagramSceneModel *diagramSceneModel,
                   QGraphicsItem *parent = nullptr);
    ~AnnotationItem() override;

    DAnnotation *annotation() const { return m_annotation; }
    DiagramSceneModel *diagramSceneModel() const { return m_diagramSceneModel; }
    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    virtual void update();

    QPointF pos() const override;
    QRectF rect() const override;
    QSizeF minimumSize() const override;

    void setPosAndRect(const QPointF &originalPos, const QRectF &originalRect,
                       const QPointF &topLeftDelta, const QPointF &bottomRightDelta) override;
    void alignItemSizeToRaster(Side adjustHorizontalSide, Side adjustVerticalSide,
                               double rasterWidth, double rasterHeight) override;

    void moveDelta(const QPointF &delta) override;
    void alignItemPositionToRaster(double rasterWidth, double rasterHeight) override;

    bool isSecondarySelected() const override;
    void setSecondarySelected(bool secondarySelected) override;
    bool isFocusSelected() const override;
    void setFocusSelected(bool focusSelected) override;
    QRectF getSecondarySelectionBoundary() override;
    void setBoundarySelected(const QRectF &boundary, bool secondary) override;

    bool isEditable() const override;
    bool isEditing() const override;
    void edit() override;
    void finishEdit() override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void updateSelectionMarker();
    void updateSelectionMarkerGeometry(const QRectF &annotationRect);
    const Style *adaptedStyle();

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
    void onContentsChanged();

    QSizeF calcMinimumGeometry() const;
    void updateGeometry();

    DAnnotation *m_annotation = nullptr;
    DiagramSceneModel *m_diagramSceneModel = nullptr;
    bool m_isSecondarySelected = false;
    bool m_isFocusSelected = false;
    RectangularSelectionItem *m_selectionMarker = nullptr;
    QGraphicsRectItem *m_noTextItem = nullptr;
    AnnotationTextItem *m_textItem = nullptr;
    bool m_isUpdating = false;
    bool m_isChanged = false;
};

} // namespace qmt
