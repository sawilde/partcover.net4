using System;
using System.Collections.Generic;

namespace PartViewer.Model
{
    public struct DocumentRange 
        : IEquatable<DocumentRange>
        , IComparable<DocumentRange>
    {
        private DocumentPoint start;
        private DocumentPoint end;

        public DocumentPoint Start
        { 
            get {return start;}
            set {start = value;}
        }

        public DocumentPoint End
        {
            get { return end; }
            set { end = value; }
        }

        public override int GetHashCode()
        {
            return Start.GetHashCode() ^ End.GetHashCode();
        }

        public override bool Equals(object obj)
        {
            if (obj is DocumentRange)
                return Equals((DocumentRange)obj);
            return base.Equals(obj);
        }

        public bool Equals(DocumentRange other)
        {
            return start.Equals(other.start) && end.Equals(other.end);
        }

        public static bool operator ==(DocumentRange left, DocumentRange right)
        {
            return left.Equals(right);
        }

        public static bool operator !=(DocumentRange left, DocumentRange right)
        {
            return !left.Equals(right);
        }

        public static bool operator < (DocumentRange left, DocumentRange right)
        {
            return left.CompareTo(right) < 0;
        }

        public static bool operator > (DocumentRange left, DocumentRange right)
        {
            return left.CompareTo(right) > 0;
        }

        public int CompareTo(DocumentRange other)
        {
            var test = start.CompareTo(other.start);
            return test == 0 ? end.CompareTo(other.end) : test;
        }

        public IEnumerable<int> Lines
        {
            get
            {
                var line = start.Line;
                while (line <= end.Line)
                {
                    yield return line;
                    line++;
                }
            }
        }
    }
}
