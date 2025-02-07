// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "tracing_global.h"
#include "timelinerenderpass.h"
#include <QVariant>
#include <QColor>
#include <QtQml/qqml.h>

#include <memory>

namespace Timeline {
class TimelineModelAggregator;

class TRACING_EXPORT TimelineModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int modelId READ modelId CONSTANT FINAL)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged FINAL)
    Q_PROPERTY(QString tooltip READ tooltip NOTIFY tooltipChanged FINAL)
    Q_PROPERTY(QColor categoryColor READ categoryColor NOTIFY categoryColorChanged FINAL)
    Q_PROPERTY(bool hasMixedTypesInExpandedState READ hasMixedTypesInExpandedState NOTIFY hasMixedTypesInExpandedStateChanged FINAL)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY contentChanged FINAL)
    Q_PROPERTY(bool hidden READ hidden WRITE setHidden NOTIFY hiddenChanged FINAL)
    Q_PROPERTY(bool expanded READ expanded WRITE setExpanded NOTIFY expandedChanged FINAL)
    Q_PROPERTY(int height READ height NOTIFY heightChanged FINAL)
    Q_PROPERTY(int expandedRowCount READ expandedRowCount NOTIFY contentChanged FINAL)
    Q_PROPERTY(int collapsedRowCount READ collapsedRowCount NOTIFY contentChanged FINAL)
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged FINAL)
    Q_PROPERTY(QVariantList labels READ labels NOTIFY labelsChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY contentChanged FINAL)
    Q_PROPERTY(int defaultRowHeight READ defaultRowHeight CONSTANT FINAL)
    QML_ELEMENT

public:
    class TimelineModelPrivate;

    TimelineModel(TimelineModelAggregator *parent);
    ~TimelineModel() override;

    // Methods implemented by the abstract model itself
    bool isEmpty() const;
    int modelId() const;

    Q_INVOKABLE int collapsedRowHeight(int rowNumber) const;
    Q_INVOKABLE int expandedRowHeight(int rowNumber) const;
    Q_INVOKABLE int rowHeight(int rowNumber) const;
    Q_INVOKABLE void setExpandedRowHeight(int rowNumber, int height);

    Q_INVOKABLE int collapsedRowOffset(int rowNumber) const;
    Q_INVOKABLE int expandedRowOffset(int rowNumber) const;
    Q_INVOKABLE int rowOffset(int rowNumber) const;

    int height() const;
    int count() const;
    Q_INVOKABLE qint64 duration(int index) const;
    Q_INVOKABLE qint64 startTime(int index) const;
    Q_INVOKABLE qint64 endTime(int index) const;
    Q_INVOKABLE int selectionId(int index) const;

    int firstIndex(qint64 startTime) const;
    int lastIndex(qint64 endTime) const;
    int bestIndex(qint64 timestamp) const;
    int parentIndex(int index) const;

    bool expanded() const;
    bool hidden() const;
    void setExpanded(bool expanded);
    void setHidden(bool hidden);
    void setDisplayName(const QString &displayName);
    QString displayName() const;
    int expandedRowCount() const;
    int collapsedRowCount() const;
    int rowCount() const;

    QString tooltip() const;
    void setTooltip(const QString &text);

    QColor categoryColor() const;
    void setCategoryColor(const QColor &color);

    // if this is disabled, a click on the row label will select the single type it contains
    bool hasMixedTypesInExpandedState() const;
    void setHasMixedTypesInExpandedState(bool value);

    // Methods which can optionally be implemented by child models.
    Q_INVOKABLE virtual QRgb color(int index) const;
    virtual QVariantList labels() const;
    Q_INVOKABLE virtual QVariantMap details(int index) const;
    Q_INVOKABLE virtual QVariantMap orderedDetails(int index) const;
    Q_INVOKABLE virtual int expandedRow(int index) const;
    Q_INVOKABLE virtual int collapsedRow(int index) const;
    Q_INVOKABLE int row(int index) const;

    // returned map should contain "file", "line", "column" properties, or be empty
    Q_INVOKABLE virtual QVariantMap location(int index) const;
    Q_INVOKABLE virtual int typeId(int index) const;
    Q_INVOKABLE virtual bool handlesTypeId(int typeId) const;
    Q_INVOKABLE virtual float relativeHeight(int index) const;
    Q_INVOKABLE virtual qint64 rowMinValue(int rowNumber) const;
    Q_INVOKABLE virtual qint64 rowMaxValue(int rowNumber) const;

    Q_INVOKABLE int nextItemBySelectionId(int selectionId, qint64 time, int currentItem) const;
    Q_INVOKABLE int nextItemByTypeId(int typeId, qint64 time, int currentItem) const;
    Q_INVOKABLE int prevItemBySelectionId(int selectionId, qint64 time, int currentItem) const;
    Q_INVOKABLE int prevItemByTypeId(int typeId, qint64 time, int currentItem) const;

    static int defaultRowHeight();
    virtual QList<const TimelineRenderPass *> supportedRenderPasses() const;

signals:
    void expandedChanged();
    void hiddenChanged();
    void expandedRowHeightChanged(int row, int height);
    void contentChanged();
    void heightChanged();
    void rowCountChanged();
    void displayNameChanged();
    void tooltipChanged();
    void categoryColorChanged();
    void hasMixedTypesInExpandedStateChanged();
    void labelsChanged();
    void detailsChanged();

protected:
    QRgb colorBySelectionId(int index) const;
    QRgb colorByFraction(double fraction) const;
    QRgb colorByHue(int hue) const;

    int insert(qint64 startTime, qint64 duration, int selectionId);
    int insertStart(qint64 startTime, int selectionId);
    void insertEnd(int index, qint64 duration);
    void computeNesting();

    void setCollapsedRowCount(int rows);
    void setExpandedRowCount(int rows);

    virtual void clear();

private:
    std::unique_ptr<TimelineModelPrivate> d;
};

} // namespace Timeline
