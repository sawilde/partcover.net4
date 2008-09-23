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

        public bool Equals(DocumentRange other)
        {
            return start.Equals(other.start) && end.Equals(other.end);
        }

        public int CompareTo(DocumentRange other)
        {
            int test = start.CompareTo(other.start);
            return test == 0 ? end.CompareTo(other.end) : test;
        }

        public IEnumerable<int> getLines()
        {
            int line = start.Line;
            while (line <= end.Line)
            {
                yield return line;
                line++;
            }
        }
    }
}
