using System;

using PartViewer.Model;

namespace PartViewer
{
    partial class DocumentView
    {

        private void kCaretLineEnd(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretEndOfLine();
        }

        private void kCaretLineBegin(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretBeginOfLine();
        }

        private void kCaretBackward(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretBackward();
        }

        private void kCaretPageUp(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretPageUp();
        }

        private void kCaretPageDown(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretPageDown();
        }

        private void kCaretForward(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretForward();
        }

        private void kCaretUp(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretUp();
        }

        private void kCaretDown(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                caretDown();
        }

        public void caretBackward()
        {
            moveCursorHorizont(-1);
        }

        public void caretForward()
        {
            moveCursorHorizont(1);
        }

        public void caretUp()
        {
            moveCursorVertical(-1, true);
        }

        public void caretDown()
        {
            moveCursorVertical(1, true);
        }

        public void caretPageUp()
        {
            moveCursorVertical(renderStuff.FirstRow - renderStuff.LastRow, true);
        }

        public void caretPageDown()
        {
            moveCursorVertical(renderStuff.LastRow - renderStuff.FirstRow, true);
        }

        public void caretEndOfLine()
        {
            if (CurrentRow == null || caret.X == CurrentRow.Length - 1)
                return;
            moveCursorHorizont(CurrentRow.Length - caret.X - 1);
        }

        public void caretBeginOfLine()
        {
            moveCursorHorizont(-caret.X);
        }

        private void moveCursorHorizont(int offset)
        {
            if (CurrentRow == null)
                return;

            int newCaretX;
            if (caret.X + offset < 0)
            {
                if (CurrentRow.Index > 0)
                {
                    moveCursorVertical(-1, false);
                    newCaretX = CurrentRow.Length - 1;
                }
                else
                {
                    newCaretX = 0;
                }
            }
            else if (caret.X + offset >= CurrentRow.Length)
            {
                if (CurrentRow.Index < Document.LineCount - 1)
                {
                    newCaretX = 0;
                    moveCursorVertical(1, false);
                }
                else
                {
                    newCaretX = CurrentRow.Length - 1;
                }
            }
            else
            {
                newCaretX = caret.X + offset;
            }

            caret.X = newCaretX;
            fireCaretMoved();

            surface.invalidate();
            ensureCaretInView();
        }

        private void moveCursorVertical(int offset, bool ensureInView)
        {
            if (CurrentRow == null)
                return;

            int newCaretY;

            if (caret.Y + offset < 0)
            {
                newCaretY = 0;
            }
            else if (caret.Y + offset >= Document.LineCount)
            {
                newCaretY = Document.LineCount - 1;
            }
            else
            {
                newCaretY = caret.Y + offset;
            }

            caret.Y = newCaretY;
            fireCaretMoved();

            if (ensureInView)
            {
                ensureCaretInView();
                surface.invalidate();
            }
        }

        private void ensureCaretInView()
        {
            int row = renderStuff.FirstRow;

            if (caret.Y + ViewStyle.CaretPaddingBottom >= renderStuff.LastRow)
            {
                int rowBand = renderStuff.LastRow - renderStuff.FirstRow + 1;

                row = Math.Min(Document.LineCount, caret.Y + ViewStyle.CaretPaddingBottom + 1) - rowBand;
            }
            else if (caret.Y - ViewStyle.CaretPaddingTop < renderStuff.FirstRow)
            {
                row = Math.Max(0, caret.Y - ViewStyle.CaretPaddingTop);
            }

            float paddingLeft = ViewStyle.CaretPaddingLeft * bounds.Width;
            float paddingRight = ViewStyle.CaretPaddingRight * bounds.Width;

            float leftOffset = renderStuff.LeftOffset;
            float caretOffset = measureCaretLeftOffset();

            if (leftOffset + paddingLeft > caretOffset)
            {
                leftOffset = Math.Max(0, caretOffset - paddingLeft);
            }
            else if (leftOffset + bounds.Width < caretOffset + paddingRight)
            {
                leftOffset = caretOffset + paddingRight - bounds.Width;
            }

            bool updateSurface = false;

            if (0.01 < Math.Abs(leftOffset - renderStuff.LeftOffset))
            {
                renderStuff.LeftOffset = leftOffset;
                updateSurface = true;
            }

            if (renderStuff.FirstRow != row)
            {
                renderStuff.FirstRow = row;
                updateSurface = true;
            }

            if (updateSurface)
                surface.invalidate();
        }

        private float measureCaretLeftOffset()
        {
            DocumentRowView rowView = getDocumentRowView(Caret.Y);
            return (rowView == null) ? 0 : findCharacterPositionOffset(rowView, Caret.X);
        }
    }
}
