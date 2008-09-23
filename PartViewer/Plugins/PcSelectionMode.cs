using System;
using System.Drawing;

using PartViewer.Model;
using PartViewer.Utils;

namespace PartViewer.Plugins
{
    internal class PcSelectionMode : ViewPlugin
    {
        class SelectionReset : IDisposable
        {
            PcSelectionMode owner;

            SelectionRegion oldSelection;
            readonly Point oldCaret;

            public SelectionReset(PcSelectionMode owner)
            {
                this.owner = owner;

                owner.processing = true;
                oldSelection = owner.view.Selection;
                oldCaret = owner.view.Caret;
            }

            public void Dispose()
            {
                if (oldSelection.IsEmpty)
                {
                    oldSelection.Start = SelectionRegion.minimum(oldCaret, owner.view.Caret);
                    oldSelection.End = SelectionRegion.maximum(oldCaret, owner.view.Caret);
                }
                else if (oldSelection.Start == oldCaret)
                {
                    Point pt = oldSelection.End;
                    oldSelection.Start = SelectionRegion.minimum(pt, owner.view.Caret);
                    oldSelection.End = SelectionRegion.maximum(pt, owner.view.Caret);
                }
                else if (oldSelection.End == oldCaret)
                {
                    Point pt = oldSelection.Start;
                    oldSelection.Start = SelectionRegion.minimum(pt, owner.view.Caret);
                    oldSelection.End = SelectionRegion.maximum(pt, owner.view.Caret);
                }

                owner.view.Selection = oldSelection;
                owner.processing = false;
                owner = null;
            }
        }

        private View view;
        private bool processing;

        public void attach(View target)
        {
            view = target;

            view.CaretMoved += view_CaretMoved;
            view.KeyMap.add(KeySelector.create(KeyCode.Left, false, false, true), kCaretBackwardSelection);
            view.KeyMap.add(KeySelector.create(KeyCode.Right, false, false, true), kCaretForwardSelection);

            view.KeyMap.add(KeySelector.create(KeyCode.Down, false, false, true), kCaretDown);
            view.KeyMap.add(KeySelector.create(KeyCode.Up, false, false, true), kCaretUp);

            view.KeyMap.add(KeySelector.create(KeyCode.PageDown, false, false, true), kCaretPageDown);
            view.KeyMap.add(KeySelector.create(KeyCode.PageUp, false, false, true), kCaretPageUp);

            view.KeyMap.add(KeySelector.create(KeyCode.Home, false, false, true), kCaretLineBegin);
            view.KeyMap.add(KeySelector.create(KeyCode.End, false, false, true), kCaretLineEnd);
        }

        public void detach(View target)
        {
            view.KeyMap.remove(KeySelector.create(KeyCode.Down, false, false, true), kCaretDown);
            view.KeyMap.remove(KeySelector.create(KeyCode.Up, false, false, true), kCaretUp);

            view.KeyMap.remove(KeySelector.create(KeyCode.PageDown, false, false, true), kCaretPageDown);
            view.KeyMap.remove(KeySelector.create(KeyCode.PageUp, false, false, true), kCaretPageUp);

            view.KeyMap.remove(KeySelector.create(KeyCode.Home, false, false, true), kCaretLineBegin);
            view.KeyMap.remove(KeySelector.create(KeyCode.End, false, false, true), kCaretLineEnd);

            view.KeyMap.remove(KeySelector.create(KeyCode.Left, false, false, true), kCaretBackwardSelection);
            view.KeyMap.remove(KeySelector.create(KeyCode.Right, false, false, true), kCaretForwardSelection);
            view.CaretMoved -= view_CaretMoved;

            view = null;
        }

        void view_CaretMoved(object sender, EventArgs e)
        {
            if (!processing) view.Selection = SelectionRegion.Empty;
        }

        private void kCaretBackwardSelection(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretBackward();
        }

        private void kCaretForwardSelection(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretForward();
        }

        private void kCaretDown(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretDown();
        }
        private void kCaretUp(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretUp();
        }
        private void kCaretPageDown(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretPageDown();
        }
        private void kCaretPageUp(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretPageUp();
        }
        private void kCaretLineBegin(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretBeginOfLine();
        }
        private void kCaretLineEnd(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.caretEndOfLine();
        }
    }
}
