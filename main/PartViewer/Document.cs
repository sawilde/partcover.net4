using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using PartViewer.Utils;
using PartViewer.Model;

namespace PartViewer
{
    public partial class Document
    {
        protected Document()
        {
            rows = new DocumentRowCollection();

            style = new Style
            {
                FontName = Control.DefaultFont.Name,
                FontHeight = Control.DefaultFont.SizeInPoints,
                Foreground = Color.Black,
                Background = Color.White
            };
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

        public Point EndPoint
        {
            get
            {
                return LineCount == 0
                    ? Point.Empty
                    : new Point(Rows[LineCount - 1].Length - 1, LineCount - 1);
            }
        }

        public static Document Create(params string[] source)
        {
            var doc = new Document();

            foreach (var s in source)
            {
                doc.rows.Add(StringSplitter.Split(s));
            }

            return doc;
        }

        public static Document CreateFromFile(string filePath)
        {
            var doc = new Document();
            doc.rows.Add(StringSplitter.Split(File.ReadAllText(filePath)));
            return doc;
        }
    }
}