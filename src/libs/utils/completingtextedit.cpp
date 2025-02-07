// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "completingtextedit.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>

static bool isEndOfWordChar(const QChar &c)
{
    return !c.isLetterOrNumber() && c.category() != QChar::Punctuation_Connector;
}

/*! \class Utils::CompletingTextEdit

  \brief The CompletingTextEdit class is a QTextEdit with auto-completion
  support.

  Excerpted from Qt examples/tools/customcompleter.
*/

namespace Utils {

class CompletingTextEditPrivate
{
public:
    CompletingTextEditPrivate(CompletingTextEdit *textEdit);

    void insertCompletion(const QString &completion);
    QString textUnderCursor() const;

    bool acceptsCompletionPrefix(const QString &prefix) const;

    QCompleter *m_completer = nullptr;
    int m_completionLengthThreshold = 3;

private:
    CompletingTextEdit *m_backPointer;
};

CompletingTextEditPrivate::CompletingTextEditPrivate(CompletingTextEdit *textEdit)
    : m_backPointer(textEdit)
{
}

void CompletingTextEditPrivate::insertCompletion(const QString &completion)
{
    if (m_completer->widget() != m_backPointer)
        return;
    QTextCursor tc = m_backPointer->textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    m_backPointer->setTextCursor(tc);
}

QString CompletingTextEditPrivate::textUnderCursor() const
{
    QTextCursor tc = m_backPointer->textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

bool CompletingTextEditPrivate::acceptsCompletionPrefix(const QString &prefix) const
{
    return prefix.length() >= m_completionLengthThreshold;
}

CompletingTextEdit::CompletingTextEdit(QWidget *parent)
    : QTextEdit(parent),
      d(new CompletingTextEditPrivate(this))
{
}

CompletingTextEdit::~CompletingTextEdit()
{
    delete d;
}

void CompletingTextEdit::setCompleter(QCompleter *c)
{
    if (completer())
        disconnect(completer(), nullptr, this, nullptr);

    d->m_completer = c;

    if (!c)
        return;

    completer()->setWidget(this);
    completer()->setCompletionMode(QCompleter::PopupCompletion);
    connect(completer(), QOverload<const QString &>::of(&QCompleter::activated),
            this, [this](const QString &str) { d->insertCompletion(str); });
}

QCompleter *CompletingTextEdit::completer() const
{
    return d->m_completer;
}

int CompletingTextEdit::completionLengthThreshold() const
{
    return d->m_completionLengthThreshold;
}

void CompletingTextEdit::setCompletionLengthThreshold(int len)
{
    d->m_completionLengthThreshold = len;
}

void CompletingTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (completer() && completer()->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    const bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (completer() == nullptr || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    const QString text = e->text();
    if (completer() == nullptr || (ctrlOrShift && text.isEmpty()))
        return;

    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    const QString newCompletionPrefix = d->textUnderCursor();
    const QChar lastChar = text.isEmpty() ? QChar() : text.right(1).at(0);

    if (!isShortcut && (hasModifier || text.isEmpty() || isEndOfWordChar(lastChar)
                        || !d->acceptsCompletionPrefix(newCompletionPrefix))) {
        completer()->popup()->hide();
        return;
    }

    if (newCompletionPrefix != completer()->completionPrefix()) {
        completer()->setCompletionPrefix(newCompletionPrefix);
        completer()->popup()->setCurrentIndex(completer()->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(completer()->popup()->sizeHintForColumn(0)
                + completer()->popup()->verticalScrollBar()->sizeHint().width());
    completer()->complete(cr); // popup it up!
}

void CompletingTextEdit::focusInEvent(QFocusEvent *e)
{
    if (completer() != nullptr)
        completer()->setWidget(this);
    QTextEdit::focusInEvent(e);
}

bool CompletingTextEdit::event(QEvent *e)
{
    // workaround for QTCREATORBUG-9453
    if (e->type() == QEvent::ShortcutOverride && completer()
            && completer()->popup() && completer()->popup()->isVisible()) {
        auto ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Escape && !ke->modifiers()) {
            ke->accept();
            return true;
        }
    }
    return QTextEdit::event(e);
}

} // namespace Utils

#include "moc_completingtextedit.cpp"
