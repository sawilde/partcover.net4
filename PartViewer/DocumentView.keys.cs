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
            attachMap.add(KeySelector.create(KeyCode.Down), kCaretDown);
            attachMap.add(KeySelector.create(KeyCode.Up), kCaretUp);
            attachMap.add(KeySelector.create(KeyCode.Right), kCaretForward);
            attachMap.add(KeySelector.create(KeyCode.Left), kCaretBackward);

            attachMap.add(KeySelector.create(KeyCode.PageDown), kCaretPageDown);
            attachMap.add(KeySelector.create(KeyCode.PageUp), kCaretPageUp);

            attachMap.add(KeySelector.create(KeyCode.Home), kCaretLineBegin);
            attachMap.add(KeySelector.create(KeyCode.End), kCaretLineEnd);

            attachMap.add(KeySelector.create(KeyChar.C, true, false, false), kCopySelection);
            attachMap.add(KeySelector.create(KeyCode.Insert, true, false, false), kCopySelection);
        }

        private void attachKeyCommands()
        {
            foreach (KeyValuePair<KeySelector, List<KeyActionHandler>> item in attachMap)
                keyActionMap.add(item.Key, item.Value);
        }

        private void detachKeyCommands()
        {
            foreach (KeyValuePair<KeySelector, List<KeyActionHandler>> item in attachMap)
                keyActionMap.remove(item.Key, item.Value);
        }

        internal void keyDown(int charCode, Keys keyData)
        {
            KeySelector s = KeySelector.create(charCode,
                (keyData & Keys.Control) == Keys.Control,
                (keyData & Keys.Alt) == Keys.Alt,
                (keyData & Keys.Shift) == Keys.Shift);

            trace.keyDown(charCode, keyData);
            keyActionMap.execute(s, ActionKeyKind.KeyDown);
        }

        internal void keyUp(int charCode, Keys keyData)
        {
            KeySelector s = KeySelector.create(charCode,
                (keyData & Keys.Control) == Keys.Control,
                (keyData & Keys.Alt) == Keys.Alt,
                (keyData & Keys.Shift) == Keys.Shift);

            trace.keyUp(charCode, keyData);
            keyActionMap.execute(s, ActionKeyKind.KeyUp);
        }
    }
}
