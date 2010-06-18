using System;

using PartViewer.Model;
using System.Drawing;

namespace PartViewer
{
    partial class DocumentView
    {

        private void kCaretLineEnd(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretEndOfLine();
        }

        private void kCaretLineBegin(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretBeginOfLine();
        }

        private void kCaretBackward(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretBackward();
        }

        private void kCaretPageUp(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretPageUp();
        }

        private void kCaretPageDown(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretPageDown();
        }

        private void kCaretForward(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretForward();
        }

        private void kCaretUp(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretUp();
        }

        private void kCaretDown(ActionKeyKind kind)
        {
            if (kind == ActionKeyKind.KeyDown)
                CaretDown();
        }

        public void CaretBackward()
        {
            moveCursorHorizont(-1);
        }

        public void CaretForward()
        {
            moveCursorHorizont(1);
        }

        public void CaretUp()
        {
            moveCursorVertical(-1, true);
        }

        public void CaretDown()
        {
            moveCursorVertical(1, true);
        }

        public void CaretPageUp()
        {
            moveCursorVertical(renderStuff.FirstRow - renderStuff.LastRow, true);
        }

        public void CaretPageDown()
        {
            moveCursorVertical(renderStuff.LastRow - renderStuff.FirstRow, true);
        }

        public void CaretEndOfLine()
        {
            if (CurrentRow == null || Position.X == CurrentRow.Length - 1)
                return;
            moveCursorHorizont(CurrentRow.Length - Position.X - 1);
        }

        public void CaretBeginOfLine()
        {
            moveCursorHorizont(-Position.X);
        }

        private void moveCursorHorizont(int offset)
        {
            if (CurrentRow == null)
                return;

            int newCaretX;
            if (Position.X + offset < 0)
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
            else if (Position.X + offset >= CurrentRow.Length)
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
                newCaretX = Position.X + offset;
            }

            Position = new Point { X = newCaretX, Y = Position.Y };
            fireCaretMoved();

            surface.invalidate();
            ensureCaretInView();
        }

        private void moveCursorVertical(int offset, bool ensureInView)
        {
            if (CurrentRow == null)
                return;

            int newCaretY;

            if (Position.Y + offset < 0)
            {
                newCaretY = 0;
            }
            else if (Position.Y + offset >= Document.LineCount)
            {
                newCaretY = Document.LineCount - 1;
            }
            else
            {
                newCaretY = Position.Y + offset;
            }

            Position = new Point { X = Position.X, Y = newCaretY };
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

            if (Position.Y + ViewStyle.CaretPaddingBottom >= renderStuff.LastRow)
            {
                int rowBand = renderStuff.LastRow - renderStuff.FirstRow + 1;

                row = Math.Min(Document.LineCount, Position.Y + ViewStyle.CaretPaddingBottom + 1) - rowBand;
            }
            else if (Position.Y - ViewStyle.CaretPaddingTop < renderStuff.FirstRow)
            {
                row = Math.Max(0, Position.Y - ViewStyle.CaretPaddingTop);
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
            DocumentRowView rowView = getDocumentRowView(Position.Y);
            return (rowView == null) ? 0 : findCharacterPositionOffset(rowView, Position.X);
        }
    }
}
