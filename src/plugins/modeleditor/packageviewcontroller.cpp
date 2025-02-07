// Copyright (C) 2018 Jochen Becher
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "packageviewcontroller.h"

#include "modelutilities.h"

#include "qmt/model/mpackage.h"
#include "qmt/model/mdependency.h"
#include "qmt/model_controller/modelcontroller.h"

#include <utils/algorithm.h>

#include <QSet>

namespace ModelEditor {
namespace Internal {

class PackageViewController::PackageViewControllerPrivate {
public:
    qmt::ModelController *m_modelController = nullptr;
    ModelUtilities *m_modelUtilities = nullptr;
};

PackageViewController::PackageViewController(QObject *parent)
    : QObject(parent),
      d(new PackageViewControllerPrivate)
{
}

PackageViewController::~PackageViewController()
{
    delete d;
}

void PackageViewController::setModelController(qmt::ModelController *modelController)
{
    d->m_modelController = modelController;
}

void PackageViewController::setModelUtilities(ModelUtilities *modelUtilities)
{
    d->m_modelUtilities = modelUtilities;
}

void PackageViewController::createAncestorDependencies(qmt::MObject *object1, qmt::MObject *object2)
{
    // search ancestors sharing a common owner
    QList<qmt::MPackage *> componentAncestors;
    auto ancestor = dynamic_cast<qmt::MPackage *>(object1->owner());
    while (ancestor) {
        componentAncestors.append(ancestor);
        ancestor = dynamic_cast<qmt::MPackage *>(ancestor->owner());
    }
    QList<qmt::MPackage *> includeComponentAncestors;
    ancestor = dynamic_cast<qmt::MPackage *>(object2->owner());
    while (ancestor) {
        includeComponentAncestors.append(ancestor);
        ancestor = dynamic_cast<qmt::MPackage *>(ancestor->owner());
    }

    int componentHighestAncestorIndex = componentAncestors.size() - 1;
    int includeComponentHighestAncestorIndex = includeComponentAncestors.size() - 1;
    QMT_ASSERT(componentAncestors.at(componentHighestAncestorIndex) == includeComponentAncestors.at(includeComponentHighestAncestorIndex), return);
    while (componentHighestAncestorIndex > 0 && includeComponentHighestAncestorIndex > 0) {
        if (componentAncestors.at(componentHighestAncestorIndex) != includeComponentAncestors.at(includeComponentHighestAncestorIndex))
            break;
        --componentHighestAncestorIndex;
        --includeComponentHighestAncestorIndex;
    }

    // add dependency between parent packages with same stereotype
    int index1 = 0;
    int includeComponentLowestIndex = 0;
    while (index1 <= componentHighestAncestorIndex
           && includeComponentLowestIndex <= includeComponentHighestAncestorIndex) {
        if (!componentAncestors.at(index1)->stereotypes().isEmpty()) {
            int index2 = includeComponentLowestIndex;
            while (index2 <= includeComponentHighestAncestorIndex) {
                if (haveMatchingStereotypes(componentAncestors.at(index1), includeComponentAncestors.at(index2))) {
                    if (!d->m_modelUtilities->haveDependency(componentAncestors.at(index1), includeComponentAncestors.at(index2))) {
                        auto dependency = new qmt::MDependency;
                        dependency->setFlags(qmt::MElement::ReverseEngineered);
                        // TODO set stereotype for testing purpose
                        dependency->setStereotypes({"same stereotype"});
                        dependency->setDirection(qmt::MDependency::AToB);
                        dependency->setSource(componentAncestors.at(index1)->uid());
                        dependency->setTarget(includeComponentAncestors.at(index2)->uid());
                        d->m_modelController->addRelation(componentAncestors.at(index1), dependency);
                    }
                    includeComponentLowestIndex = index2 + 1;
                    break;
                }
                ++index2;
            }
        }
        ++index1;
    }

    // add dependency between topmost packages with common owner
    if (componentAncestors.at(componentHighestAncestorIndex) != includeComponentAncestors.at(includeComponentHighestAncestorIndex)) {
        if (!d->m_modelUtilities->haveDependency(componentAncestors.at(componentHighestAncestorIndex), includeComponentAncestors)) {
            auto dependency = new qmt::MDependency;
            dependency->setFlags(qmt::MElement::ReverseEngineered);
            // TODO set stereotype for testing purpose
            dependency->setStereotypes({"ancestor"});
            dependency->setDirection(qmt::MDependency::AToB);
            dependency->setSource(componentAncestors.at(componentHighestAncestorIndex)->uid());
            dependency->setTarget(includeComponentAncestors.at(includeComponentHighestAncestorIndex)->uid());
            d->m_modelController->addRelation(componentAncestors.at(componentHighestAncestorIndex), dependency);
        }
    }

    // add dependency between parent packages
    if (componentHighestAncestorIndex > 0 && includeComponentHighestAncestorIndex > 0) { // check for valid parents
        if (componentAncestors.at(0) != includeComponentAncestors.at(0)) {
            if (!d->m_modelUtilities->haveDependency(componentAncestors.at(0), includeComponentAncestors)) {
                auto dependency = new qmt::MDependency;
                dependency->setFlags(qmt::MElement::ReverseEngineered);
                // TODO set stereotype for testing purpose
                dependency->setStereotypes({"parents"});
                dependency->setDirection(qmt::MDependency::AToB);
                dependency->setSource(componentAncestors.at(0)->uid());
                dependency->setTarget(includeComponentAncestors.at(0)->uid());
                d->m_modelController->addRelation(componentAncestors.at(0), dependency);
            }
        }
    }
}

bool PackageViewController::haveMatchingStereotypes(const qmt::MObject *object1,
                                                    const qmt::MObject *object2)
{
    return !(Utils::toSet(object1->stereotypes()) & Utils::toSet(object2->stereotypes())).isEmpty();
}


} // namespace Internal
} // namespace ModelEditor
