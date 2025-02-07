// Copyright (C) 2016 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "dclonevisitor.h"

#include "qmt/diagram/delement.h"
#include "qmt/diagram/dobject.h"
#include "qmt/diagram/dpackage.h"
#include "qmt/diagram/dclass.h"
#include "qmt/diagram/dcomponent.h"
#include "qmt/diagram/ddiagram.h"
#include "qmt/diagram/ditem.h"
#include "qmt/diagram/drelation.h"
#include "qmt/diagram/dinheritance.h"
#include "qmt/diagram/ddependency.h"
#include "qmt/diagram/dassociation.h"
#include "qmt/diagram/dconnection.h"
#include "qmt/diagram/dannotation.h"
#include "qmt/diagram/dboundary.h"
#include "qmt/diagram/dswimlane.h"
#include "qmt/infrastructure/qmtassert.h"

namespace qmt {

DCloneVisitor::DCloneVisitor()
{
}

void DCloneVisitor::visitDElement(const DElement *element)
{
    Q_UNUSED(element)
    QMT_CHECK(m_cloned);
}

void DCloneVisitor::visitDObject(const DObject *object)
{
    QMT_CHECK(m_cloned);
    visitDElement(object);
}

void DCloneVisitor::visitDPackage(const DPackage *package)
{
    if (!m_cloned)
        m_cloned = new DPackage(*package);
    visitDObject(package);
}

void DCloneVisitor::visitDClass(const DClass *klass)
{
    if (!m_cloned)
        m_cloned = new DClass(*klass);
    visitDObject(klass);
}

void DCloneVisitor::visitDComponent(const DComponent *component)
{
    if (!m_cloned)
        m_cloned = new DComponent(*component);
    visitDObject(component);
}

void DCloneVisitor::visitDDiagram(const DDiagram *diagram)
{
    if (!m_cloned)
        m_cloned = new DDiagram(*diagram);
    visitDObject(diagram);
}

void DCloneVisitor::visitDItem(const DItem *item)
{
    if (!m_cloned)
        m_cloned = new DItem(*item);
    visitDObject(item);

}

void DCloneVisitor::visitDRelation(const DRelation *relation)
{
    QMT_CHECK(m_cloned);
    visitDElement(relation);
}

void DCloneVisitor::visitDInheritance(const DInheritance *inheritance)
{
    if (!m_cloned)
        m_cloned = new DInheritance(*inheritance);
    visitDRelation(inheritance);
}

void DCloneVisitor::visitDDependency(const DDependency *dependency)
{
    if (!m_cloned)
        m_cloned = new DDependency(*dependency);
    visitDRelation(dependency);
}

void DCloneVisitor::visitDAssociation(const DAssociation *association)
{
    if (!m_cloned)
        m_cloned = new DAssociation(*association);
    visitDRelation(association);
}

void DCloneVisitor::visitDConnection(const DConnection *connection)
{
    if (!m_cloned)
        m_cloned = new DConnection(*connection);
    visitDRelation(connection);
}

void DCloneVisitor::visitDAnnotation(const DAnnotation *annotation)
{
    if (!m_cloned)
        m_cloned = new DAnnotation(*annotation);
    visitDElement(annotation);
}

void DCloneVisitor::visitDBoundary(const DBoundary *boundary)
{
    if (!m_cloned)
        m_cloned = new DBoundary(*boundary);
    visitDElement(boundary);
}

void DCloneVisitor::visitDSwimlane(const DSwimlane *swimlane)
{
    if (!m_cloned)
        m_cloned = new DSwimlane(*swimlane);
    visitDElement(swimlane);
}


DCloneDeepVisitor::DCloneDeepVisitor()
{
}

void DCloneDeepVisitor::visitDElement(const DElement *element)
{
    Q_UNUSED(element)
    QMT_CHECK(m_cloned);
}

void DCloneDeepVisitor::visitDObject(const DObject *object)
{
    QMT_CHECK(m_cloned);
    visitDElement(object);
}

void DCloneDeepVisitor::visitDPackage(const DPackage *package)
{
    if (!m_cloned)
        m_cloned = new DPackage(*package);
    visitDObject(package);
}

void DCloneDeepVisitor::visitDClass(const DClass *klass)
{
    if (!m_cloned)
        m_cloned = new DClass(*klass);
    visitDObject(klass);
}

void DCloneDeepVisitor::visitDComponent(const DComponent *component)
{
    if (!m_cloned)
        m_cloned = new DComponent(*component);
    visitDObject(component);
}

void DCloneDeepVisitor::visitDDiagram(const DDiagram *diagram)
{
    if (!m_cloned)
        m_cloned = new DDiagram(*diagram);
    visitDObject(diagram);
}

void DCloneDeepVisitor::visitDItem(const DItem *item)
{
    if (!m_cloned)
        m_cloned = new DItem(*item);
    visitDObject(item);
}

void DCloneDeepVisitor::visitDRelation(const DRelation *relation)
{
    QMT_CHECK(m_cloned);
    visitDElement(relation);
}

void DCloneDeepVisitor::visitDInheritance(const DInheritance *inheritance)
{
    if (!m_cloned)
        m_cloned = new DInheritance(*inheritance);
    visitDRelation(inheritance);
}

void DCloneDeepVisitor::visitDDependency(const DDependency *dependency)
{
    if (!m_cloned)
        m_cloned = new DDependency(*dependency);
    visitDRelation(dependency);
}

void DCloneDeepVisitor::visitDAssociation(const DAssociation *association)
{
    if (!m_cloned)
        m_cloned = new DAssociation(*association);
    visitDRelation(association);
}

void DCloneDeepVisitor::visitDConnection(const DConnection *connection)
{
    if (!m_cloned)
        m_cloned = new DConnection(*connection);
    visitDRelation(connection);
}

void DCloneDeepVisitor::visitDAnnotation(const DAnnotation *annotation)
{
    if (!m_cloned)
        m_cloned = new DAnnotation(*annotation);
    visitDElement(annotation);
}

void DCloneDeepVisitor::visitDBoundary(const DBoundary *boundary)
{
    if (!m_cloned)
        m_cloned = new DBoundary(*boundary);
    visitDElement(boundary);
}

void DCloneDeepVisitor::visitDSwimlane(const DSwimlane *swimlane)
{
    if (!m_cloned)
        m_cloned = new DSwimlane(*swimlane);
    visitDElement(swimlane);
}

} // namespace qmt
