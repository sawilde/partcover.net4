using System;

namespace PartViewer
{
	partial class DocumentView
	{
        private event EventHandler<EventArgs> eCaretMoved;

        public event EventHandler<EventArgs> CaretMoved
        {
            add { eCaretMoved += value; }
            remove { eCaretMoved -= value; }
        }

        private void fireCaretMoved()
        {
            if (eCaretMoved != null)
                eCaretMoved(this, EventArgs.Empty);
        }
	}
}
