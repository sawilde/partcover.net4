using System;
using System.Drawing;

using PartViewer.Model;
using PartViewer.Utils;

namespace PartViewer.Plugins
{
    internal class PcSelectionMode : IViewPlugin
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
                oldCaret = owner.view.Position;
            }

            public void Dispose()
            {
                if (oldSelection.IsEmpty)
                {
                    oldSelection.Start = SelectionRegion.Min(oldCaret, owner.view.Position);
                    oldSelection.End = SelectionRegion.Max(oldCaret, owner.view.Position);
                }
                else if (oldSelection.Start == oldCaret)
                {
                    var pt = oldSelection.End;
                    oldSelection.Start = SelectionRegion.Min(pt, owner.view.Position);
                    oldSelection.End = SelectionRegion.Max(pt, owner.view.Position);
                }
                else if (oldSelection.End == oldCaret)
                {
                    var pt = oldSelection.Start;
                    oldSelection.Start = SelectionRegion.Min(pt, owner.view.Position);
                    oldSelection.End = SelectionRegion.Max(pt, owner.view.Position);
                }

                owner.view.Selection = oldSelection;
                owner.processing = false;
                owner = null;

                GC.SuppressFinalize(this);
            }
        }

        private IView view;
        private bool processing;

        public void Attach(IView target)
        {
            view = target;

            view.CaretMoved += view_CaretMoved;
            view.KeyMap.Add(KeySelector.Create(KeyCode.Left, false, false, true), kCaretBackwardSelection);
            view.KeyMap.Add(KeySelector.Create(KeyCode.Right, false, false, true), kCaretForwardSelection);

            view.KeyMap.Add(KeySelector.Create(KeyCode.Down, false, false, true), kCaretDown);
            view.KeyMap.Add(KeySelector.Create(KeyCode.Up, false, false, true), kCaretUp);

            view.KeyMap.Add(KeySelector.Create(KeyCode.PageDown, false, false, true), kCaretPageDown);
            view.KeyMap.Add(KeySelector.Create(KeyCode.PageUp, false, false, true), kCaretPageUp);

            view.KeyMap.Add(KeySelector.Create(KeyCode.Home, false, false, true), kCaretLineBegin);
            view.KeyMap.Add(KeySelector.Create(KeyCode.End, false, false, true), kCaretLineEnd);
        }

        public void Detach(IView target)
        {
            view.KeyMap.Remove(KeySelector.Create(KeyCode.Down, false, false, true), kCaretDown);
            view.KeyMap.Remove(KeySelector.Create(KeyCode.Up, false, false, true), kCaretUp);

            view.KeyMap.Remove(KeySelector.Create(KeyCode.PageDown, false, false, true), kCaretPageDown);
            view.KeyMap.Remove(KeySelector.Create(KeyCode.PageUp, false, false, true), kCaretPageUp);

            view.KeyMap.Remove(KeySelector.Create(KeyCode.Home, false, false, true), kCaretLineBegin);
            view.KeyMap.Remove(KeySelector.Create(KeyCode.End, false, false, true), kCaretLineEnd);

            view.KeyMap.Remove(KeySelector.Create(KeyCode.Left, false, false, true), kCaretBackwardSelection);
            view.KeyMap.Remove(KeySelector.Create(KeyCode.Right, false, false, true), kCaretForwardSelection);
            view.CaretMoved -= view_CaretMoved;

            view = null;
        }

        void view_CaretMoved(object sender, EventArgs e)
        {
            if (!processing) view.Selection = new SelectionRegion();
        }

        private void kCaretBackwardSelection(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretBackward();
        }

        private void kCaretForwardSelection(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretForward();
        }

        private void kCaretDown(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretDown();
        }
        private void kCaretUp(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretUp();
        }
        private void kCaretPageDown(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretPageDown();
        }
        private void kCaretPageUp(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretPageUp();
        }
        private void kCaretLineBegin(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretBeginOfLine();
        }
        private void kCaretLineEnd(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            using (new SelectionReset(this))
                view.CaretEndOfLine();
        }
    }
}
