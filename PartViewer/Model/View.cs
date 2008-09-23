using System;
using System.Drawing;
using PartViewer.Styles;

namespace PartViewer.Model
{
    public interface View
    {
        void caretBackward();
        void caretForward();

        void caretUp();
        void caretDown();

        void caretPageUp();
        void caretPageDown();

        void caretEndOfLine();
        void caretBeginOfLine();

        void moveCaretTo(Point pt);
        void centerLine(int line);

        KeyActionMap KeyMap { get;}
        Point Caret { get;}

        ViewStyle ViewStyle { get; }
        SelectionRegion Selection { get; set;}

        event EventHandler<EventArgs> CaretMoved;
    }
}
