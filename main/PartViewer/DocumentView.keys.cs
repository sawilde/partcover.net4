using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Forms;

using PartViewer.Model;
using PartViewer.Utils;

namespace PartViewer
{
    partial class DocumentView
    {
        private readonly KeyActionMap keyActionMap;

        public KeyActionMap KeyMap
        {
            [DebuggerHidden]
            get { return keyActionMap; }
        }

        private readonly KeyActionMap attachMap = new KeyActionMap();

        private void createKeyCommands()
        {
            attachMap.Add(KeySelector.Create(KeyCode.Down), kCaretDown);
            attachMap.Add(KeySelector.Create(KeyCode.Up), kCaretUp);
            attachMap.Add(KeySelector.Create(KeyCode.Right), kCaretForward);
            attachMap.Add(KeySelector.Create(KeyCode.Left), kCaretBackward);

            attachMap.Add(KeySelector.Create(KeyCode.PageDown), kCaretPageDown);
            attachMap.Add(KeySelector.Create(KeyCode.PageUp), kCaretPageUp);

            attachMap.Add(KeySelector.Create(KeyCode.Home), kCaretLineBegin);
            attachMap.Add(KeySelector.Create(KeyCode.End), kCaretLineEnd);

            attachMap.Add(KeySelector.Create(KeyChar.C, true, false, false), kCopySelection);
            attachMap.Add(KeySelector.Create(KeyCode.Insert, true, false, false), kCopySelection);
        }

        private void attachKeyCommands()
        {
            foreach (var item in attachMap)
                keyActionMap.Add(item.Key, item.Value);
        }

        private void detachKeyCommands()
        {
            foreach (KeyValuePair<KeySelector, List<KeyActionHandler>> item in attachMap)
                keyActionMap.Remove(item.Key, item.Value);
        }

        internal void keyDown(int charCode, Keys keyData)
        {
            var s = KeySelector.Create(charCode,
                (keyData & Keys.Control) == Keys.Control,
                (keyData & Keys.Alt) == Keys.Alt,
                (keyData & Keys.Shift) == Keys.Shift);

            trace.keyDown(charCode, keyData);
            keyActionMap.Execute(s, ActionKeyKind.KeyDown);
        }

        internal void keyUp(int charCode, Keys keyData)
        {
            var s = KeySelector.Create(charCode,
                (keyData & Keys.Control) == Keys.Control,
                (keyData & Keys.Alt) == Keys.Alt,
                (keyData & Keys.Shift) == Keys.Shift);

            trace.keyUp(charCode, keyData);
            keyActionMap.Execute(s, ActionKeyKind.KeyUp);
        }
    }
}
