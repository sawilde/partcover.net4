using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Text;
using PartViewer.Model;
using PartViewer.Styles;
using PartViewer.Utils;

namespace PartViewer
{
    public struct SelectionRegion : IEquatable<SelectionRegion>
    {
        private Point start;
        private Point end;

        public Point Start
        {
            get { return start; }
            set { start = value; }
        }
        public Point End
        {
            get { return end; }
            set { end = value; }
        }

        public static Point minimum(Point pt1, Point pt2)
        {
            if (pt1.Y < pt2.Y) return pt1;
            if (pt1.Y > pt2.Y) return pt2;
            return (pt1.X <= pt2.X) ? pt1 : pt2;
        }

        public static Point maximum(Point pt1, Point pt2)
        {
            if (pt1.Y > pt2.Y) return pt1;
            if (pt1.Y < pt2.Y) return pt2;
            return (pt1.X <= pt2.X) ? pt2 : pt1;
        }

        public bool IsEmpty {
            get { return start.IsEmpty && end.IsEmpty; }
        }

        public static readonly SelectionRegion Empty = new SelectionRegion();

        public bool Equals(SelectionRegion other)
        {
            if (IsEmpty && other.IsEmpty) return true;
            if (!IsEmpty || !other.IsEmpty) return false;

            return Start == other.Start && End == other.End;
        }

        public CharacterRange extractLineRange(int line, int lineLength)
        {
            CharacterRange res = new CharacterRange();
            if (IsEmpty || line < Start.Y || line > End.Y)
                return res;

            int realStartX = Math.Max(Math.Min(Start.X, lineLength), 0);
            int realEndX = Math.Max(Math.Min(End.X, lineLength), 0);

            if (Start.Y == End.Y)
            {
                res.First = realStartX;
                res.Length = realEndX - res.First;
            }
            else if (line == Start.Y)
            {
                res.First = realStartX;
                res.Length = lineLength - res.First;
            }
            else if (line == End.Y)
            {
                res.First = 0;
                res.Length = realEndX;
            }
            else 
            {
                res.First = 0;
                res.Length = lineLength;
            }

            return res;
        }

        public IEnumerable<int> getLines()
        {
            int line = start.Y;
            while (line <= end.Y)
            {
                yield return line;
                line++;
            }
        }

        public override string ToString()
        {
            if (IsEmpty)
                return "SelectionRegion [empty]";
            return "SelectionRegion [" + Start + "; " + End + "]";
        }
    }

    partial class DocumentView
    {
        SelectionRegion selection;

        public SelectionRegion Selection
        {
            get { return selection; }
            set
            {
                if (selection.Equals(value)) return;

                Trace.WriteLine(Selection);

                selection = value;
                surface.invalidate();
            }
        }

        private void kCopySelection(ActionKeyKind kind)
        {
            if (kind != ActionKeyKind.KeyDown) return;

            copySelection();
        }

        public void copySelection()
        {
            ClipboardManager.putPlain(getSelectionText());
        }

        public string getSelectionText()
        {
            if (Selection.IsEmpty) return string.Empty;

            StringBuilder builder=  new StringBuilder();
            foreach (int line in Selection.getLines())
            {
                DocumentRow row = Document.Rows[line];
                CharacterRange chRange = Selection.extractLineRange(line, row.Length);

                if (chRange.Length == 0 && line == Selection.End.Y)
                    break;

                string substr = row.Raw.Substring(chRange.First, chRange.Length);
                if (substr.EndsWith("\n"))
                {
                    substr = substr.TrimEnd('\n');
                    builder.AppendLine(substr);
                }
                else
                {
                    builder.Append(substr);
                }
            }

            return builder.ToString();
        }
    }
}
