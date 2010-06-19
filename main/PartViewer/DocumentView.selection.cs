using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Text;
using PartViewer.Model;
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

        public static Point Min(Point pt1, Point pt2)
        {
            if (pt1.Y < pt2.Y) return pt1;
            if (pt1.Y > pt2.Y) return pt2;
            return (pt1.X <= pt2.X) ? pt1 : pt2;
        }

        public static Point Max(Point pt1, Point pt2)
        {
            if (pt1.Y > pt2.Y) return pt1;
            if (pt1.Y < pt2.Y) return pt2;
            return (pt1.X <= pt2.X) ? pt2 : pt1;
        }

        public bool IsEmpty
        {
            get { return start.IsEmpty && end.IsEmpty; }
        }

        public override int GetHashCode()
        {
            return Start.GetHashCode() ^ End.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            if (obj is SelectionRegion)
                return Equals((SelectionRegion)obj);
            return base.Equals(obj);
        }

        public static bool operator ==(SelectionRegion left, SelectionRegion right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(SelectionRegion left, SelectionRegion right)
        {
            return !left.Equals(right);
        }

        public bool Equals(SelectionRegion other)
        {
            if (IsEmpty && other.IsEmpty) return true;
            if (!IsEmpty || !other.IsEmpty) return false;

            return Start == other.Start && End == other.End;
        }

        public CharacterRange ExtractLineRange(int line, int lineLength)
        {
            var res = new CharacterRange();
            if (IsEmpty || line < Start.Y || line > End.Y)
                return res;

            var realStartX = Math.Max(Math.Min(Start.X, lineLength), 0);
            var realEndX = Math.Max(Math.Min(End.X, lineLength), 0);

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

        public ICollection<int> Lines
        {
            get
            {
                var result = new List<int>();
                for (var line = start.Y; line <= end.Y; line++)
                {
                    result.Add(line);
                }
                return result;
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

            CopySelection();
        }

        public void CopySelection()
        {
            ClipboardManager.PutPlain(SelectionText);
        }

        public string SelectionText
        {
            get
            {
                if (Selection.IsEmpty) return string.Empty;

                var builder = new StringBuilder();
                foreach (var line in Selection.Lines)
                {
                    var row = Document.Rows[line];
                    var chRange = Selection.ExtractLineRange(line, row.Length);

                    if (chRange.Length == 0 && line == Selection.End.Y)
                        break;

                    var substr = row.Raw.Substring(chRange.First, chRange.Length);
                    if (substr.EndsWith("\n", StringComparison.OrdinalIgnoreCase))
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
}
