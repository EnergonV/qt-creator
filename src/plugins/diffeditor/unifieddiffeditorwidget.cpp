// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "unifieddiffeditorwidget.h"

#include "diffeditorconstants.h"
#include "diffeditordocument.h"
#include "diffutils.h"

#include <QHash>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>

#include <coreplugin/icore.h>

#include <texteditor/textdocument.h>
#include <texteditor/textdocumentlayout.h>
#include <texteditor/texteditorsettings.h>
#include <texteditor/fontsettings.h>
#include <texteditor/displaysettings.h>

#include <utils/tooltip/tooltip.h>

using namespace Core;
using namespace TextEditor;
using namespace Utils;

namespace DiffEditor {
namespace Internal {

UnifiedDiffEditorWidget::UnifiedDiffEditorWidget(QWidget *parent)
    : SelectableTextEditorWidget("DiffEditor.UnifiedDiffEditor", parent)
    , m_controller(this)
{
    DisplaySettings settings = displaySettings();
    settings.m_textWrapping = false;
    settings.m_displayLineNumbers = true;
    settings.m_markTextChanges = false;
    settings.m_highlightBlocks = false;
    SelectableTextEditorWidget::setDisplaySettings(settings);

    setReadOnly(true);
    connect(TextEditorSettings::instance(), &TextEditorSettings::displaySettingsChanged,
            this, &UnifiedDiffEditorWidget::setDisplaySettings);
    setDisplaySettings(TextEditorSettings::displaySettings());
    setCodeStyle(TextEditorSettings::codeStyle());

    connect(TextEditorSettings::instance(), &TextEditorSettings::fontSettingsChanged,
            this, &UnifiedDiffEditorWidget::setFontSettings);
    setFontSettings(TextEditorSettings::fontSettings());

    clear(tr("No document"));

    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &UnifiedDiffEditorWidget::slotCursorPositionChangedInEditor);

    auto context = new Core::IContext(this);
    context->setWidget(this);
    context->setContext(Core::Context(Constants::UNIFIED_VIEW_ID));
    Core::ICore::addContextObject(context);
    setCodeFoldingSupported(true);
}

void UnifiedDiffEditorWidget::setDocument(DiffEditorDocument *document)
{
    m_controller.setDocument(document);
    clear();
    QList<FileData> diffFileList;
    if (document)
        diffFileList = document->diffFiles();
    setDiff(diffFileList);
}

DiffEditorDocument *UnifiedDiffEditorWidget::diffDocument() const
{
    return m_controller.document();
}

void UnifiedDiffEditorWidget::saveState()
{
    if (!m_state.isNull())
        return;

    m_state = SelectableTextEditorWidget::saveState();
}

void UnifiedDiffEditorWidget::restoreState()
{
    if (m_state.isNull())
        return;

    const GuardLocker locker(m_controller.m_ignoreChanges);
    SelectableTextEditorWidget::restoreState(m_state);
    m_state.clear();
}

void UnifiedDiffEditorWidget::setDisplaySettings(const DisplaySettings &ds)
{
    DisplaySettings settings = displaySettings();
    settings.m_visualizeWhitespace = ds.m_visualizeWhitespace;
    settings.m_displayFoldingMarkers = ds.m_displayFoldingMarkers;
    settings.m_scrollBarHighlights = ds.m_scrollBarHighlights;
    settings.m_highlightCurrentLine = ds.m_highlightCurrentLine;
    SelectableTextEditorWidget::setDisplaySettings(settings);
}

void UnifiedDiffEditorWidget::setFontSettings(const FontSettings &fontSettings)
{
    m_controller.setFontSettings(fontSettings);
}

void UnifiedDiffEditorWidget::slotCursorPositionChangedInEditor()
{
    if (m_controller.m_ignoreChanges.isLocked())
        return;

    const GuardLocker locker(m_controller.m_ignoreChanges);
    emit currentDiffFileIndexChanged(fileIndexForBlockNumber(textCursor().blockNumber()));
}

void UnifiedDiffEditorWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && !(e->modifiers() & Qt::ShiftModifier)) {
        QTextCursor cursor = cursorForPosition(e->pos());
        jumpToOriginalFile(cursor);
        e->accept();
        return;
    }
    SelectableTextEditorWidget::mouseDoubleClickEvent(e);
}

void UnifiedDiffEditorWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        jumpToOriginalFile(textCursor());
        e->accept();
        return;
    }
    SelectableTextEditorWidget::keyPressEvent(e);
}

void UnifiedDiffEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QPointer<QMenu> menu = createStandardContextMenu();

    const QTextCursor tc = textCursor();
    QTextCursor start = tc;
    start.setPosition(tc.selectionStart());
    QTextCursor end = tc;
    end.setPosition(tc.selectionEnd());
    const int startBlockNumber = start.blockNumber();
    const int endBlockNumber = end.blockNumber();

    QTextCursor cursor = cursorForPosition(e->pos());
    const int blockNumber = cursor.blockNumber();

    const int fileIndex = fileIndexForBlockNumber(blockNumber);
    const int chunkIndex = chunkIndexForBlockNumber(blockNumber);

    const ChunkData chunkData = m_controller.chunkData(fileIndex, chunkIndex);

    QList<int> leftSelection, rightSelection;

    for (int i = startBlockNumber; i <= endBlockNumber; ++i) {
        const int currentFileIndex = fileIndexForBlockNumber(i);
        if (currentFileIndex < fileIndex)
            continue;

        if (currentFileIndex > fileIndex)
            break;

        const int currentChunkIndex = chunkIndexForBlockNumber(i);
        if (currentChunkIndex < chunkIndex)
            continue;

        if (currentChunkIndex > chunkIndex)
            break;

        const int leftRow = m_leftLineNumbers.value(i, qMakePair(-1, -1)).second;
        const int rightRow = m_rightLineNumbers.value(i, qMakePair(-1, -1)).second;

        if (leftRow >= 0)
            leftSelection.append(leftRow);
        if (rightRow >= 0)
            rightSelection.append(rightRow);
    }

    const ChunkSelection selection(leftSelection, rightSelection);

    addContextMenuActions(menu, fileIndexForBlockNumber(blockNumber),
                          chunkIndexForBlockNumber(blockNumber), selection);

    connect(this, &UnifiedDiffEditorWidget::destroyed, menu.data(), &QMenu::deleteLater);
    menu->exec(e->globalPos());
    delete menu;
}

void UnifiedDiffEditorWidget::addContextMenuActions(QMenu *menu,
                                                    int fileIndex,
                                                    int chunkIndex,
                                                    const ChunkSelection &selection)
{
    menu->addSeparator();

    m_controller.addCodePasterAction(menu, fileIndex, chunkIndex);
    m_controller.addApplyAction(menu, fileIndex, chunkIndex);
    m_controller.addRevertAction(menu, fileIndex, chunkIndex);
    m_controller.addExtraActions(menu, fileIndex, chunkIndex, selection);
}

void UnifiedDiffEditorWidget::clear(const QString &message)
{
    m_leftLineNumberDigits = 1;
    m_rightLineNumberDigits = 1;
    m_leftLineNumbers.clear();
    m_rightLineNumbers.clear();
    m_fileInfo.clear();
    m_chunkInfo.clear();
    setSelections(QMap<int, QList<DiffSelection> >());

    const GuardLocker locker(m_controller.m_ignoreChanges);
    SelectableTextEditorWidget::clear();
    m_controller.m_contextFileData.clear();
    setPlainText(message);
}

QString UnifiedDiffEditorWidget::lineNumber(int blockNumber) const
{
    QString lineNumberString;

    const bool leftLineExists = m_leftLineNumbers.contains(blockNumber);
    const bool rightLineExists = m_rightLineNumbers.contains(blockNumber);

    if (leftLineExists || rightLineExists) {
        const QString leftLine = leftLineExists
                ? QString::number(m_leftLineNumbers.value(blockNumber).first)
                : QString();
        lineNumberString += QString(m_leftLineNumberDigits - leftLine.count(),
                                    ' ') + leftLine;

        lineNumberString += '|';

        const QString rightLine = rightLineExists
                ? QString::number(m_rightLineNumbers.value(blockNumber).first)
                : QString();
        lineNumberString += QString(m_rightLineNumberDigits - rightLine.count(),
                                    ' ') + rightLine;
    }
    return lineNumberString;
}

int UnifiedDiffEditorWidget::lineNumberDigits() const
{
    return m_leftLineNumberDigits + m_rightLineNumberDigits + 1;
}

void UnifiedDiffEditorWidget::setLeftLineNumber(int blockNumber, int lineNumber,
                                                int rowNumberInChunk)
{
    const QString lineNumberString = QString::number(lineNumber);
    m_leftLineNumbers.insert(blockNumber, qMakePair(lineNumber, rowNumberInChunk));
    m_leftLineNumberDigits = qMax(m_leftLineNumberDigits,
                                  lineNumberString.count());
}

void UnifiedDiffEditorWidget::setRightLineNumber(int blockNumber, int lineNumber,
                                                 int rowNumberInChunk)
{
    const QString lineNumberString = QString::number(lineNumber);
    m_rightLineNumbers.insert(blockNumber, qMakePair(lineNumber, rowNumberInChunk));
    m_rightLineNumberDigits = qMax(m_rightLineNumberDigits,
                                   lineNumberString.count());
}

void UnifiedDiffEditorWidget::setFileInfo(int blockNumber,
                                          const DiffFileInfo &leftFileInfo,
                                          const DiffFileInfo &rightFileInfo)
{
    m_fileInfo[blockNumber] = qMakePair(leftFileInfo, rightFileInfo);
}

void UnifiedDiffEditorWidget::setChunkIndex(int startBlockNumber,
                                            int blockCount,
                                            int chunkIndex)
{
    m_chunkInfo.insert(startBlockNumber, qMakePair(blockCount, chunkIndex));
}

void UnifiedDiffEditorWidget::setDiff(const QList<FileData> &diffFileList)
{
    const GuardLocker locker(m_controller.m_ignoreChanges);
    clear();
    m_controller.m_contextFileData = diffFileList;
    showDiff();
}

QString UnifiedDiffEditorWidget::showChunk(const ChunkData &chunkData,
                                           bool lastChunk,
                                           int *blockNumber,
                                           int *charNumber,
                                           QMap<int, QList<DiffSelection> > *selections)
{
    if (chunkData.contextChunk)
        return QString();

    QString diffText;
    int leftLineCount = 0;
    int rightLineCount = 0;
    int blockCount = 0;
    int charCount = 0;
    QList<TextLineData> leftBuffer, rightBuffer;
    QList<int> leftRowsBuffer, rightRowsBuffer;

    (*selections)[*blockNumber].append(DiffSelection(&m_controller.m_chunkLineFormat));

    int lastEqualRow = -1;
    if (lastChunk) {
        for (int i = chunkData.rows.count(); i > 0; i--) {
            if (chunkData.rows.at(i - 1).equal) {
                if (i != chunkData.rows.count())
                    lastEqualRow = i - 1;
                break;
            }
        }
    }

    for (int i = 0; i <= chunkData.rows.count(); i++) {
        const RowData &rowData = i < chunkData.rows.count()
                ? chunkData.rows.at(i)
                : RowData(TextLineData(TextLineData::Separator)); // dummy,
                                       // ensure we process buffers to the end.
                                       // rowData will be equal
        if (rowData.equal && i != lastEqualRow) {
            if (!leftBuffer.isEmpty()) {
                for (int j = 0; j < leftBuffer.count(); j++) {
                    const TextLineData &lineData = leftBuffer.at(j);
                    const QString line = DiffUtils::makePatchLine(
                                '-',
                                lineData.text,
                                lastChunk,
                                i == chunkData.rows.count()
                                && j == leftBuffer.count() - 1);

                    const int blockDelta = line.count('\n'); // no new line
                                                     // could have been added
                    for (int k = 0; k < blockDelta; k++)
                        (*selections)[*blockNumber + blockCount + 1 + k].append(&m_controller.m_leftLineFormat);

                    for (auto it = lineData.changedPositions.cbegin(),
                              end = lineData.changedPositions.cend(); it != end; ++it) {
                        const int startPos = it.key() < 0
                                ? 1 : it.key() + 1;
                        const int endPos = it.value() < 0
                                ? it.value() : it.value() + 1;
                        (*selections)[*blockNumber + blockCount + 1].append(
                                    DiffSelection(startPos, endPos, &m_controller.m_leftCharFormat));
                    }

                    if (!line.isEmpty()) {
                        setLeftLineNumber(*blockNumber + blockCount + 1,
                                          chunkData.leftStartingLineNumber
                                          + leftLineCount + 1,
                                          leftRowsBuffer.at(j));
                        blockCount += blockDelta;
                        ++leftLineCount;
                    }

                    diffText += line;

                    charCount += line.count();
                }
                leftBuffer.clear();
                leftRowsBuffer.clear();
            }
            if (!rightBuffer.isEmpty()) {
                for (int j = 0; j < rightBuffer.count(); j++) {
                    const TextLineData &lineData = rightBuffer.at(j);
                    const QString line = DiffUtils::makePatchLine(
                                '+',
                                lineData.text,
                                lastChunk,
                                i == chunkData.rows.count()
                                && j == rightBuffer.count() - 1);

                    const int blockDelta = line.count('\n'); // no new line
                                                     // could have been added

                    for (int k = 0; k < blockDelta; k++)
                        (*selections)[*blockNumber + blockCount + 1 + k].append(&m_controller.m_rightLineFormat);

                    for (auto it = lineData.changedPositions.cbegin(),
                              end = lineData.changedPositions.cend(); it != end; ++it) {
                        const int startPos = it.key() < 0
                                ? 1 : it.key() + 1;
                        const int endPos = it.value() < 0
                                ? it.value() : it.value() + 1;
                        (*selections)[*blockNumber + blockCount + 1].append
                                (DiffSelection(startPos, endPos, &m_controller.m_rightCharFormat));
                    }

                    if (!line.isEmpty()) {
                        setRightLineNumber(*blockNumber + blockCount + 1,
                                           chunkData.rightStartingLineNumber
                                           + rightLineCount + 1,
                                           rightRowsBuffer.at(j));
                        blockCount += blockDelta;
                        ++rightLineCount;
                    }

                    diffText += line;

                    charCount += line.count();
                }
                rightBuffer.clear();
                rightRowsBuffer.clear();
            }
            if (i < chunkData.rows.count()) {
                const QString line = DiffUtils::makePatchLine(' ',
                                          rowData.rightLine.text,
                                          lastChunk,
                                          i == chunkData.rows.count() - 1);

                if (!line.isEmpty()) {
                    setLeftLineNumber(*blockNumber + blockCount + 1,
                                      chunkData.leftStartingLineNumber
                                      + leftLineCount + 1,
                                      i);
                    setRightLineNumber(*blockNumber + blockCount + 1,
                                       chunkData.rightStartingLineNumber
                                       + rightLineCount + 1,
                                       i);
                    blockCount += line.count('\n');
                    ++leftLineCount;
                    ++rightLineCount;
                }

                diffText += line;

                charCount += line.count();
            }
        } else {
            if (rowData.leftLine.textLineType == TextLineData::TextLine) {
                leftBuffer.append(rowData.leftLine);
                leftRowsBuffer.append(i);
            }
            if (rowData.rightLine.textLineType == TextLineData::TextLine) {
                rightBuffer.append(rowData.rightLine);
                rightRowsBuffer.append(i);
            }
        }
    }

    const QString chunkLine = "@@ -"
            + QString::number(chunkData.leftStartingLineNumber + 1)
            + ','
            + QString::number(leftLineCount)
            + " +"
            + QString::number(chunkData.rightStartingLineNumber+ 1)
            + ','
            + QString::number(rightLineCount)
            + " @@"
            + chunkData.contextInfo
            + '\n';

    diffText.prepend(chunkLine);

    *blockNumber += blockCount + 1; // +1 for chunk line
    *charNumber += charCount + chunkLine.count();
    return diffText;
}

void UnifiedDiffEditorWidget::showDiff()
{
    QString diffText;

    int blockNumber = 0;
    int charNumber = 0;

    // 'foldingIndent' is populated with <block number> and folding indentation
    // value where 1 indicates start of new file and 2 indicates a diff chunk.
    // Remaining lines (diff contents) are assigned 3.
    QHash<int, int> foldingIndent;

    QMap<int, QList<DiffSelection> > selections;

    for (const FileData &fileData : qAsConst(m_controller.m_contextFileData)) {
        const QString leftFileInfo = "--- " + fileData.leftFileInfo.fileName + '\n';
        const QString rightFileInfo = "+++ " + fileData.rightFileInfo.fileName + '\n';
        setFileInfo(blockNumber, fileData.leftFileInfo, fileData.rightFileInfo);
        foldingIndent.insert(blockNumber, 1);
        selections[blockNumber].append(DiffSelection(&m_controller.m_fileLineFormat));
        blockNumber++;
        foldingIndent.insert(blockNumber, 1);
        selections[blockNumber].append(DiffSelection(&m_controller.m_fileLineFormat));
        blockNumber++;

        diffText += leftFileInfo;
        diffText += rightFileInfo;
        charNumber += leftFileInfo.count() + rightFileInfo.count();

        if (fileData.binaryFiles) {
            foldingIndent.insert(blockNumber, 2);
            selections[blockNumber].append(DiffSelection(&m_controller.m_chunkLineFormat));
            blockNumber++;
            const QString binaryLine = "Binary files "
                    + fileData.leftFileInfo.fileName
                    + " and "
                    + fileData.rightFileInfo.fileName
                    + " differ\n";
            diffText += binaryLine;
            charNumber += binaryLine.count();
        } else {
            for (int j = 0; j < fileData.chunks.count(); j++) {
                const int oldBlockNumber = blockNumber;
                foldingIndent.insert(blockNumber, 2);
                diffText += showChunk(fileData.chunks.at(j),
                                      (j == fileData.chunks.count() - 1)
                                      && fileData.lastChunkAtTheEndOfFile,
                                      &blockNumber,
                                      &charNumber,
                                      &selections);
                if (!fileData.chunks.at(j).contextChunk)
                    setChunkIndex(oldBlockNumber, blockNumber - oldBlockNumber, j);
            }
        }

    }

    if (diffText.isEmpty()) {
        setPlainText(tr("No difference."));
        return;
    }

    diffText.replace('\r', ' ');
    {
        const GuardLocker locker(m_controller.m_ignoreChanges);
        setPlainText(diffText);

        QTextBlock block = document()->firstBlock();
        for (int b = 0; block.isValid(); block = block.next(), ++b)
            setFoldingIndent(block, foldingIndent.value(b, 3));
    }

    setSelections(selections);
}

int UnifiedDiffEditorWidget::blockNumberForFileIndex(int fileIndex) const
{
    if (fileIndex < 0 || fileIndex >= m_fileInfo.count())
        return -1;

    return std::next(m_fileInfo.constBegin(), fileIndex).key();
}

int UnifiedDiffEditorWidget::fileIndexForBlockNumber(int blockNumber) const
{
    int i = -1;
    for (auto it = m_fileInfo.cbegin(), end = m_fileInfo.cend(); it != end; ++it, ++i) {
        if (it.key() > blockNumber)
            break;
    }

    return i;
}

int UnifiedDiffEditorWidget::chunkIndexForBlockNumber(int blockNumber) const
{
    if (m_chunkInfo.isEmpty())
        return -1;

    auto it = m_chunkInfo.upperBound(blockNumber);
    if (it == m_chunkInfo.constBegin())
        return -1;

    --it;

    if (blockNumber < it.key() + it.value().first)
        return it.value().second;

    return -1;
}

void UnifiedDiffEditorWidget::jumpToOriginalFile(const QTextCursor &cursor)
{
    if (m_fileInfo.isEmpty())
        return;

    const int blockNumber = cursor.blockNumber();
    const int fileIndex = fileIndexForBlockNumber(blockNumber);
    if (fileIndex < 0)
        return;

    const FileData fileData = m_controller.m_contextFileData.at(fileIndex);
    const QString leftFileName = fileData.leftFileInfo.fileName;
    const QString rightFileName = fileData.rightFileInfo.fileName;

    const int columnNumber = cursor.positionInBlock() - 1; // -1 for the first character in line

    const int rightLineNumber = m_rightLineNumbers.value(blockNumber, qMakePair(-1, 0)).first;
    if (rightLineNumber >= 0) {
        m_controller.jumpToOriginalFile(rightFileName, rightLineNumber, columnNumber);
        return;
    }

    const int leftLineNumber = m_leftLineNumbers.value(blockNumber, qMakePair(-1, 0)).first;
    if (leftLineNumber >= 0) {
        if (leftFileName == rightFileName) {
            for (const ChunkData &chunkData : fileData.chunks) {

                int newLeftLineNumber = chunkData.leftStartingLineNumber;
                int newRightLineNumber = chunkData.rightStartingLineNumber;

                for (const RowData &rowData : chunkData.rows) {
                    if (rowData.leftLine.textLineType == TextLineData::TextLine)
                        newLeftLineNumber++;
                    if (rowData.rightLine.textLineType == TextLineData::TextLine)
                        newRightLineNumber++;
                    if (newLeftLineNumber == leftLineNumber) {
                        m_controller.jumpToOriginalFile(leftFileName, newRightLineNumber, 0);
                        return;
                    }
                }
            }
        } else {
            m_controller.jumpToOriginalFile(leftFileName, leftLineNumber, columnNumber);
        }
        return;
    }
}

void UnifiedDiffEditorWidget::setCurrentDiffFileIndex(int diffFileIndex)
{
    if (m_controller.m_ignoreChanges.isLocked())
        return;

    const GuardLocker locker(m_controller.m_ignoreChanges);
    const int blockNumber = blockNumberForFileIndex(diffFileIndex);

    QTextBlock block = document()->findBlockByNumber(blockNumber);
    QTextCursor cursor = textCursor();
    cursor.setPosition(block.position());
    setTextCursor(cursor);
    verticalScrollBar()->setValue(blockNumber);
}

} // namespace Internal
} // namespace DiffEditor
