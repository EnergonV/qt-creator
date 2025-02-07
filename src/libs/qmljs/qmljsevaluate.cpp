// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "qmljsevaluate.h"
#include "qmljscontext.h"
#include "qmljsscopechain.h"
#include "qmljsvalueowner.h"
#include "parser/qmljsast_p.h"

#include <QDebug>

using namespace QmlJS;

/*!
    \class QmlJS::Evaluate
    \brief The Evaluate class evaluates \l{AST::Node}s to \l{Value}s.
    \sa Value ScopeChain

    The Evaluate visitor is constructed with a ScopeChain and accepts JavaScript
    expressions as well as some other AST::Nodes. It evaluates the expression in
    the given ScopeChain and returns a Value representing the result.

    Example: Pass in the AST for "1 + 2" and NumberValue will be returned.

    In normal cases only the call operator (or the equivalent value() function)
    will be used.

    The reference() function has the special behavior of not resolving \l{Reference}s
    which can be useful when interested in the identity of a variable instead
    of its value.

    Example: In a scope where "var a = 1"
    \list
    \li value(Identifier-a) will return NumberValue
    \li reference(Identifier-a) will return the ASTVariableReference for the declaration of "a"
    \endlist
*/

Evaluate::Evaluate(const ScopeChain *scopeChain, ReferenceContext *referenceContext)
    : _valueOwner(scopeChain->context()->valueOwner()),
      _context(scopeChain->context()),
      _referenceContext(referenceContext),
      _scopeChain(scopeChain),
      _result(nullptr)
{
}

Evaluate::~Evaluate()
{
}

const Value *Evaluate::operator()(AST::Node *ast)
{
    return value(ast);
}

const Value *Evaluate::value(AST::Node *ast)
{
    const Value *result = reference(ast);

    if (const Reference *ref = value_cast<Reference>(result)) {
        if (_referenceContext)
            result = _referenceContext->lookupReference(ref);
        else
            result = _context->lookupReference(ref);
    }

    // if evaluation fails, return an unknown value
    if (! result)
        result = _valueOwner->unknownValue();

    return result;
}

const Value *Evaluate::reference(AST::Node *ast)
{
    // save the result
    const Value *previousResult = switchResult(nullptr);

    // process the expression
    accept(ast);

    // restore the previous result
    return switchResult(previousResult);
}

const Value *Evaluate::switchResult(const Value *result)
{
    const Value *previousResult = _result;
    _result = result;
    return previousResult;
}

void Evaluate::accept(AST::Node *node)
{
    AST::Node::accept(node, this);
}

bool Evaluate::visit(AST::UiProgram *)
{
    return false;
}

bool Evaluate::visit(AST::UiHeaderItemList *)
{
    return false;
}

bool Evaluate::visit(AST::UiPragma *)
{
    return false;
}

bool Evaluate::visit(AST::UiImport *)
{
    return false;
}

bool Evaluate::visit(AST::UiPublicMember *)
{
    return false;
}

bool Evaluate::visit(AST::UiSourceElement *)
{
    return false;
}

bool Evaluate::visit(AST::UiObjectDefinition *)
{
    return false;
}

bool Evaluate::visit(AST::UiObjectInitializer *)
{
    return false;
}

bool Evaluate::visit(AST::UiObjectBinding *)
{
    return false;
}

bool Evaluate::visit(AST::UiScriptBinding *)
{
    return false;
}

bool Evaluate::visit(AST::UiArrayBinding *)
{
    return false;
}

bool Evaluate::visit(AST::UiObjectMemberList *)
{
    return false;
}

bool Evaluate::visit(AST::UiArrayMemberList *)
{
    return false;
}

bool Evaluate::visit(AST::UiQualifiedId *ast)
{
    if (ast->name.isEmpty())
         return false;

    const Value *value = _scopeChain->lookup(ast->name.toString());
    if (! ast->next) {
        _result = value;

    } else {
        const ObjectValue *base = value_cast<ObjectValue>(value);

        for (AST::UiQualifiedId *it = ast->next; base && it; it = it->next) {
            const QString &name = it->name.toString();
            if (name.isEmpty())
                break;

            const Value *value = base->lookupMember(name, _context);
            if (! it->next)
                _result = value;
            else
                base = value_cast<ObjectValue>(value);
        }
    }

    return false;
}

bool Evaluate::visit(AST::TemplateLiteral *ast)
{
    Q_UNUSED(ast)
    _result = _valueOwner->stringValue();
    return false;
}

bool Evaluate::visit(AST::ThisExpression *)
{
    return false;
}

bool Evaluate::visit(AST::IdentifierExpression *ast)
{
    if (ast->name.isEmpty())
        return false;

    _result = _scopeChain->lookup(ast->name.toString());
    return false;
}

bool Evaluate::visit(AST::NullExpression *)
{
    _result = _valueOwner->nullValue();
    return false;
}

bool Evaluate::visit(AST::TrueLiteral *)
{
    _result = _valueOwner->booleanValue();
    return false;
}

bool Evaluate::visit(AST::FalseLiteral *)
{
    _result = _valueOwner->booleanValue();
    return false;
}

bool Evaluate::visit(AST::StringLiteral *)
{
    _result = _valueOwner->stringValue();
    return false;
}

bool Evaluate::visit(AST::NumericLiteral *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::RegExpLiteral *)
{
    _result = _valueOwner->regexpCtor()->returnValue();
    return false;
}

bool Evaluate::visit(AST::ArrayPattern *)
{
    _result = _valueOwner->arrayCtor()->returnValue();
    return false;
}

bool Evaluate::visit(AST::ObjectPattern *)
{
    // ### properties
    _result = _valueOwner->newObject();
    return false;
}

bool Evaluate::visit(AST::PatternElementList *)
{
    return false;
}

bool Evaluate::visit(AST::Elision *)
{
    return false;
}

bool Evaluate::visit(AST::PatternPropertyList *)
{
    return false;
}

bool Evaluate::visit(AST::PatternProperty *)
{
    return false;
}

bool Evaluate::visit(AST::NestedExpression *)
{
    return true; // visit the child expression
}

bool Evaluate::visit(AST::IdentifierPropertyName *)
{
    return false;
}

bool Evaluate::visit(AST::StringLiteralPropertyName *)
{
    return false;
}

bool Evaluate::visit(AST::NumericLiteralPropertyName *)
{
    return false;
}

bool Evaluate::visit(AST::ArrayMemberExpression *)
{
    return false;
}

bool Evaluate::visit(AST::FieldMemberExpression *ast)
{
    if (ast->name.isEmpty())
        return false;

    if (const Value *base = _valueOwner->convertToObject(value(ast->base))) {
        if (const ObjectValue *obj = base->asObjectValue())
            _result = obj->lookupMember(ast->name.toString(), _context);
    }

    return false;
}

bool Evaluate::visit(AST::NewMemberExpression *ast)
{
    if (const FunctionValue *ctor = value_cast<FunctionValue>(value(ast->base)))
        _result = ctor->returnValue();
    return false;
}

bool Evaluate::visit(AST::NewExpression *ast)
{
    if (const FunctionValue *ctor = value_cast<FunctionValue>(value(ast->expression)))
        _result = ctor->returnValue();
    return false;
}

bool Evaluate::visit(AST::CallExpression *ast)
{
    if (const Value *base = value(ast->base)) {
        if (const FunctionValue *obj = base->asFunctionValue())
            _result = obj->returnValue();
    }
    return false;
}

bool Evaluate::visit(AST::ArgumentList *)
{
    return false;
}

bool Evaluate::visit(AST::PostIncrementExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::PostDecrementExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::DeleteExpression *)
{
    _result = _valueOwner->booleanValue();
    return false;
}

bool Evaluate::visit(AST::VoidExpression *)
{
    _result = _valueOwner->undefinedValue();
    return false;
}

bool Evaluate::visit(AST::TypeOfExpression *)
{
    _result = _valueOwner->stringValue();
    return false;
}

bool Evaluate::visit(AST::PreIncrementExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::PreDecrementExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::UnaryPlusExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::UnaryMinusExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::TildeExpression *)
{
    _result = _valueOwner->numberValue();
    return false;
}

bool Evaluate::visit(AST::NotExpression *)
{
    _result = _valueOwner->booleanValue();
    return false;
}

bool Evaluate::visit(AST::BinaryExpression *ast)
{
    const Value *lhs = nullptr;
    const Value *rhs = nullptr;
    switch (ast->op) {
    case QSOperator::Add:
    case QSOperator::InplaceAdd:
    //case QSOperator::And: // ### enable once implemented below
    //case QSOperator::Or:
        lhs = value(ast->left);
        Q_FALLTHROUGH();
    case QSOperator::Assign:
        rhs = value(ast->right);
        break;
    default:
        break;
    }

    switch (ast->op) {
    case QSOperator::Add:
    case QSOperator::InplaceAdd:
        if (lhs->asStringValue() || rhs->asStringValue())
            _result = _valueOwner->stringValue();
        else
            _result = _valueOwner->numberValue();
        break;

    case QSOperator::Sub:
    case QSOperator::InplaceSub:
    case QSOperator::Mul:
    case QSOperator::InplaceMul:
    case QSOperator::Div:
    case QSOperator::InplaceDiv:
    case QSOperator::Mod:
    case QSOperator::InplaceMod:
    case QSOperator::BitAnd:
    case QSOperator::InplaceAnd:
    case QSOperator::BitXor:
    case QSOperator::InplaceXor:
    case QSOperator::BitOr:
    case QSOperator::InplaceOr:
    case QSOperator::LShift:
    case QSOperator::InplaceLeftShift:
    case QSOperator::RShift:
    case QSOperator::InplaceRightShift:
    case QSOperator::URShift:
    case QSOperator::InplaceURightShift:
        _result = _valueOwner->numberValue();
        break;

    case QSOperator::Le:
    case QSOperator::Ge:
    case QSOperator::Lt:
    case QSOperator::Gt:
    case QSOperator::Equal:
    case QSOperator::NotEqual:
    case QSOperator::StrictEqual:
    case QSOperator::StrictNotEqual:
    case QSOperator::InstanceOf:
    case QSOperator::In:
        _result = _valueOwner->booleanValue();
        break;

    case QSOperator::And:
    case QSOperator::Or:
        // ### either lhs or rhs
        _result = _valueOwner->unknownValue();
        break;

    case QSOperator::Assign:
        _result = rhs;
        break;

    default:
        break;
    }

    return false;
}

bool Evaluate::visit(AST::ConditionalExpression *)
{
    return false;
}

bool Evaluate::visit(AST::Expression *)
{
    return false;
}

bool Evaluate::visit(AST::Block *)
{
    return false;
}

bool Evaluate::visit(AST::VariableStatement *)
{
    return false;
}

bool Evaluate::visit(AST::VariableDeclarationList *)
{
    return false;
}

bool Evaluate::visit(AST::PatternElement *)
{
    return false;
}

bool Evaluate::visit(AST::EmptyStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ExpressionStatement *)
{
    return true;
}

bool Evaluate::visit(AST::IfStatement *)
{
    return false;
}

bool Evaluate::visit(AST::DoWhileStatement *)
{
    return false;
}

bool Evaluate::visit(AST::WhileStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ForStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ForEachStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ContinueStatement *)
{
    return false;
}

bool Evaluate::visit(AST::BreakStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ReturnStatement *)
{
    return true;
}

bool Evaluate::visit(AST::WithStatement *)
{
    return false;
}

bool Evaluate::visit(AST::SwitchStatement *)
{
    return false;
}

bool Evaluate::visit(AST::CaseBlock *)
{
    return false;
}

bool Evaluate::visit(AST::CaseClauses *)
{
    return false;
}

bool Evaluate::visit(AST::CaseClause *)
{
    return false;
}

bool Evaluate::visit(AST::DefaultClause *)
{
    return false;
}

bool Evaluate::visit(AST::LabelledStatement *)
{
    return false;
}

bool Evaluate::visit(AST::ThrowStatement *)
{
    return false;
}

bool Evaluate::visit(AST::TryStatement *)
{
    return false;
}

bool Evaluate::visit(AST::Catch *)
{
    return false;
}

bool Evaluate::visit(AST::Finally *)
{
    return false;
}

bool Evaluate::visit(AST::FunctionDeclaration *)
{
    return false;
}

bool Evaluate::visit(AST::FunctionExpression *)
{
    return false;
}

bool Evaluate::visit(AST::FormalParameterList *)
{
    return false;
}

bool Evaluate::visit(AST::Program *)
{
    return false;
}

bool Evaluate::visit(AST::StatementList *)
{
    return false;
}

bool Evaluate::visit(AST::DebuggerStatement *)
{
    return false;
}

void Evaluate::throwRecursionDepthError()
{
    qWarning("Evaluate hit maximum recursion error when visiting AST");
}
