using System;

namespace PartViewer.Model
{
    public struct DocumentPoint
        : IEquatable<DocumentPoint>
        , IComparable<DocumentPoint>
    {
        private int line;
        private int column;

        public DocumentPoint(int line, int column)
        {
            this.line = line;
            this.column = column;
        }

        public int Line
        {
            get { return line; }
            set { line = value; }
        }

        public int Column
        {
            get { return column; }
            set { column = value; }
        }

        public bool Equals(DocumentPoint other)
        {
            return line.Equals(other.line) && column.Equals(other.column);
        }

        public int CompareTo(DocumentPoint other)
        {
            int test = line.CompareTo(other.line);
            return test == 0
                ? column.CompareTo(other.column)
                : test;
        }
    }
}
