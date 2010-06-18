using System;
using System.Drawing;

namespace PartViewer.Model
{
    public interface IView
    {
        void CaretBackward();
        void CaretForward();

        void CaretUp();
        void CaretDown();

        void CaretPageUp();
        void CaretPageDown();

        void CaretEndOfLine();
        void CaretBeginOfLine();

        void MoveCaretTo(Point pt);
        void CenterLine(int line);

        KeyActionMap KeyMap { get;}
        Point Position { get;}

        ViewStyle ViewStyle { get; }
        SelectionRegion Selection { get; set;}

        event EventHandler<EventArgs> CaretMoved;
    }
}
