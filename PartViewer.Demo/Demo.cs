using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using PartViewer.Model;

namespace PartViewer.Demo
{
    public partial class Demo : Form
    {
        public Demo()
        {
            InitializeComponent();
        }

        private void tsmLoadFile_Click(object sender, EventArgs e)
        {
            if (dLoadFile.ShowDialog() != DialogResult.OK)
                return;

            ViewControl view = new ViewControl();
            view.Dock = DockStyle.Fill;
            view.Document = Document.createFromFile(dLoadFile.FileName);

            view.Document.Style.FontName = "Courier New";
            view.Document.Style.FontHeight = 10f;
            view.Document.Style.Foreground = Color.Black;
            view.Document.Style.Background = Color.FromArgb(239, 239, 228);

            TabPage page = new TabPage(Path.GetFileName(dLoadFile.FileName));
            page.Controls.Add(view);

            tbPages.TabPages.Add(page);
            tbPages.SelectedTab = page;
        }

        private void cToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            ViewControl view = (ViewControl) tbPages.SelectedTab.Controls[0];

            view.Document.removeStylizers();
            view.Document.addStylizer(new CsharpKeywordStylizer());
        }

        private void cToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            ViewControl view = (ViewControl) tbPages.SelectedTab.Controls[0];

            view.Document.removeStylizers();
            view.Document.addStylizer(new CKeywordStylizer());
        }

        private void offToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            ViewControl view = (ViewControl)tbPages.SelectedTab.Controls[0];

            view.Document.removeStylizers();
        }
    }

    class CsharpKeywordStylizer : Stylizer
    {
        static readonly string[] Keywors = { 
            "int", "long", "class", "private", "void", "public", "namespace", "using", "string"
        };

        StyleFace keywordFace;

        public void stylize(StylizerSource source)
        {
            if (source.Document.createFace(this, "scharp-keyword") != null)
            {
                keywordFace = source.Document.getFace("scharp-keyword");
                keywordFace.FaceStyle.Background = Color.GhostWhite;
                keywordFace.FaceStyle.Foreground = Color.FromArgb(0, 0, 150);
                keywordFace.FaceStyle.FontStyle = FontStyle.Bold;
            }

            foreach (int line in source.Range.getLines())
            {
                stylizeLine(source, line);
            }
        }

        private void stylizeLine(StylizerSource source, int line)
        {
            DocumentRow row = source.Document.Rows[line];
            foreach (string s in Keywors)
            {
                int index = 0;
                while (-1 != (index = row.Raw.IndexOf(s, index)))
                {
                    DocumentRange docRange = new DocumentRange();
                    docRange.Start = new DocumentPoint(line, index);
                    docRange.End = new DocumentPoint(line, index + s.Length);

                    source.setFace(docRange, keywordFace);

                    index += s.Length;
                }
            }
        }
    }

    class CKeywordStylizer : Stylizer
    {
        static readonly string[] Keywors = { 
            "int", "long", "private", "void", "public", "namespace"
        };

        StyleFace keywordFace;

        public void stylize(StylizerSource source)
        {
            if (source.Document.createFace(this, "c-keyword") != null)
            {
                keywordFace = source.Document.getFace("c-keyword");
                keywordFace.FaceStyle.Foreground = Color.Green;
                keywordFace.FaceStyle.FontStyle = FontStyle.Bold;
            }

            foreach (int line in source.Range.getLines())
            {
                stylizeLine(source, line);
            }
        }

        private void stylizeLine(StylizerSource source, int line)
        {
            DocumentRow row = source.Document.Rows[line];
            foreach (string s in Keywors)
            {
                int index = 0;
                while (-1 != (index = row.Raw.IndexOf(s, index)))
                {
                    DocumentRange docRange = new DocumentRange();
                    docRange.Start = new DocumentPoint(line, index);
                    docRange.End = new DocumentPoint(line, index + s.Length);

                    source.setFace(docRange, keywordFace);

                    index += s.Length;
                }
            }
        }
    }
}