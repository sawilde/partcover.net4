using System;

namespace PartViewer.Model
{
    public struct DocumentPoint
        : IEquatable<DocumentPoint>
        , IComparable<DocumentPoint>
    {
        public DocumentPoint(int line, int column)
            : this()
        {
            Line = line;
            Column = column;
        }

        public int Line { get; set; }
        public int Column { get; set; }

        public override int GetHashCode()
        {
            return Line.GetHashCode() ^ Column.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            if (obj is DocumentPoint)
                return Equals((DocumentPoint)obj);
            return base.Equals(obj);
        }

        public static bool operator ==(DocumentPoint left, DocumentPoint right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(DocumentPoint left, DocumentPoint right)
        {
            return !left.Equals(right);
        }

        public static bool operator <(DocumentPoint left, DocumentPoint right)
        {
            return left.CompareTo(right) < 0;
        }

        public static bool operator >(DocumentPoint left, DocumentPoint right)
        {
            return left.CompareTo(right) > 0;
        }

        public bool Equals(DocumentPoint other)
        {
            return Line.Equals(other.Line) && Column.Equals(other.Column);
        }

        public int CompareTo(DocumentPoint other)
        {
            var test = Line.CompareTo(other.Line);
            return test == 0
                ? Column.CompareTo(other.Column)
                : test;
        }
    }
}
