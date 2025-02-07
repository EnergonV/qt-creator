// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "qmljswrapinloader.h"
#include "qmljsquickfixassist.h"

#include <coreplugin/idocument.h>

#include <qmljs/parser/qmljsast_p.h>
#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsscopechain.h>
#include <qmljs/qmljsutils.h>
#include <qmljs/qmljsbind.h>
#include <qmljstools/qmljsrefactoringchanges.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

using namespace QmlJS;
using namespace QmlJS::AST;
using namespace QmlJSTools;

namespace QmlJSEditor {

using namespace Internal;

namespace {

class FindIds : protected Visitor
{
public:
    using Result = QHash<QString, SourceLocation>;

    Result operator()(Node *node)
    {
        result.clear();
        Node::accept(node, this);
        return result;
    }

protected:
    bool visit(UiObjectInitializer *ast) override
    {
        UiScriptBinding *idBinding;
        QString id = idOfObject(ast, &idBinding);
        if (!id.isEmpty())
            result[id] = locationFromRange(idBinding->statement);
        return true;
    }

    void throwRecursionDepthError() override
    {
        qWarning("Warning: Hit maximum recursion depth while visitin AST in FindIds");
    }

    Result result;
};

template <typename T>
class Operation: public QmlJSQuickFixOperation
{
    Q_DECLARE_TR_FUNCTIONS(QmlJSEditor::Internal::Operation)

    T *m_objDef;

public:
    Operation(const QSharedPointer<const QmlJSQuickFixAssistInterface> &interface,
              T *objDef)
        : QmlJSQuickFixOperation(interface, 0)
        , m_objDef(objDef)
    {
        Q_ASSERT(m_objDef);

        setDescription(tr("Wrap Component in Loader"));
    }

    QString findFreeName(const QString &base)
    {
        QString tryName = base;
        int extraNumber = 1;
        const ObjectValue *found = nullptr;
        const ScopeChain &scope = assistInterface()->semanticInfo().scopeChain();
        forever {
            scope.lookup(tryName, &found);
            if (!found || extraNumber > 1000)
                break;
            tryName = base + QString::number(extraNumber++);
        }
        return tryName;
    }

    void performChanges(QmlJSRefactoringFilePtr currentFile,
                        const QmlJSRefactoringChanges &) override
    {
        UiScriptBinding *idBinding;
        const QString id = idOfObject(m_objDef, &idBinding);
        QString baseName = id;
        if (baseName.isEmpty()) {
            for (UiQualifiedId *it = m_objDef->qualifiedTypeNameId; it; it = it->next) {
                if (!it->next)
                    baseName = it->name.toString();
            }
        }

        // find ids
        const QString componentId = findFreeName(QLatin1String("component_") + baseName);
        const QString loaderId = findFreeName(QLatin1String("loader_") + baseName);

        Utils::ChangeSet changes;

        FindIds::Result innerIds = FindIds()(m_objDef);
        innerIds.remove(id);

        QString comment = tr("// TODO: Move position bindings from the component to the Loader.\n"
                             "//       Check all uses of 'parent' inside the root element of the component.")
                          + QLatin1Char('\n');
        if (idBinding) {
            comment += tr("//       Rename all outer uses of the id \"%1\" to \"%2.item\".").arg(
                        id, loaderId) + QLatin1Char('\n');
        }

        // handle inner ids
        QString innerIdForwarders;
        for (auto it = innerIds.cbegin(), end = innerIds.cend(); it != end; ++it) {
            const QString innerId = it.key();
            comment += tr("//       Rename all outer uses of the id \"%1\" to \"%2.item.%1\".\n").arg(
                        innerId, loaderId);
            changes.replace(it.value().begin(), it.value().end(), QString::fromLatin1("inner_%1").arg(innerId));
            innerIdForwarders += QString::fromLatin1("\nproperty alias %1: inner_%1").arg(innerId);
        }
        if (!innerIdForwarders.isEmpty()) {
            innerIdForwarders.append(QLatin1Char('\n'));
            const int afterOpenBrace = m_objDef->initializer->lbraceToken.end();
            changes.insert(afterOpenBrace, innerIdForwarders);
        }

        const int objDefStart = m_objDef->qualifiedTypeNameId->firstSourceLocation().begin();
        const int objDefEnd = m_objDef->lastSourceLocation().end();
        changes.insert(objDefStart, comment +
                       QString::fromLatin1("Component {\n"
                                           "    id: %1\n").arg(componentId));
        changes.insert(objDefEnd, QString::fromLatin1("\n"
                                                      "}\n"
                                                      "Loader {\n"
                                                      "    id: %2\n"
                                                      "    sourceComponent: %1\n"
                                                      "}\n").arg(componentId, loaderId));
        currentFile->setChangeSet(changes);
        currentFile->appendIndentRange(Range(objDefStart, objDefEnd));
        currentFile->apply();
    }
};

} // end of anonymous namespace


void matchWrapInLoaderQuickFix(const QmlJSQuickFixInterface &interface, QuickFixOperations &result)
{
    const int pos = interface->currentFile()->cursor().position();

    QList<Node *> path = interface->semanticInfo().rangePath(pos);
    for (int i = path.size() - 1; i >= 0; --i) {
        Node *node = path.at(i);
        if (auto objDef = cast<UiObjectDefinition *>(node)) {
            if (!interface->currentFile()->isCursorOn(objDef->qualifiedTypeNameId))
                return;
             // check that the node is not the root node
            if (i > 0 && !cast<UiProgram*>(path.at(i - 1))) {
                result << new Operation<UiObjectDefinition>(interface, objDef);
                return;
            }
        } else if (auto objBinding = cast<UiObjectBinding *>(node)) {
            if (!interface->currentFile()->isCursorOn(objBinding->qualifiedTypeNameId))
                return;
            result << new Operation<UiObjectBinding>(interface, objBinding);
            return;
        }
    }
}

} // namespace QmlJSEditor
