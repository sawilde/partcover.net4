using System.Diagnostics;

namespace PartViewer
{
    public delegate void DocumentRowEventHandler(object sender, DocumentRow arg);

    public class DocumentRow
    {
        internal static DocumentRow create(string s)
        {
            return new DocumentRow(s);
        }

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private readonly string raw;

        private int index;

        internal DocumentRow(string s)
        {
            raw = s;
        }

        public string Raw
        {
            get { return raw; }
        }

        public int Length
        {
            get { return raw.Length; }
        }

        public int Index
        {
            get { return index; }
            internal set { index = value; }
        }
    }
}