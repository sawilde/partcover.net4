using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using PartViewer.Styles;
using PartViewer.Utils;

namespace PartViewer
{
    public partial class Document
    {
        protected Document()
        {
            rows = new DocumentRowCollection();

            style = new Style();

            style.FontName = Control.DefaultFont.Name;
            style.FontHeight = Control.DefaultFont.SizeInPoints;
            style.Foreground = Color.Black;
            style.Background = Color.White;
        }

        private readonly Style style;
        public Style Style
        {
            [DebuggerHidden]
            get { return style; }
        }

        [DebuggerBrowsable(DebuggerBrowsableState.Never)]
        private readonly DocumentRowCollection rows;

        public int LineCount
        {
            [DebuggerHidden]
            get { return Rows.Count; }
        }

        public DocumentRowCollection Rows
        {
            get { return rows; }
        }

        public Point StartPoint
        {
            get { return new Point(0, 0); }
        }

        public Point EndPoint
        {
            get {
                if (LineCount == 0)
                    return Point.Empty;
                return new Point(Rows[LineCount - 1].Length - 1, LineCount - 1);
            }
        }

        public static Document create(params string[] source)
        {
            Document doc = new Document();

            foreach (string s in source)
            {
                doc.rows.add(StringSplitter.split(s));
            }

            return doc;
        }

        public static Document createFromFile(string filePath)
        {
            Document doc = new Document();

            doc.rows.add(StringSplitter.split(File.ReadAllText(filePath)));

            return doc;
        }
    }
}