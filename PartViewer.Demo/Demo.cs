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

            var view = new ViewControl
            {
                Dock = DockStyle.Fill, 
                Document = Document.CreateFromFile(dLoadFile.FileName)
            };

            view.Document.Style.FontName = "Courier New";
            view.Document.Style.FontHeight = 10f;
            view.Document.Style.Foreground = Color.Black;
            view.Document.Style.Background = Color.FromArgb(239, 239, 228);

            var page = new TabPage(Path.GetFileName(dLoadFile.FileName));
            page.Controls.Add(view);

            tbPages.TabPages.Add(page);
            tbPages.SelectedTab = page;
        }

        private void cToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            var view = (ViewControl) tbPages.SelectedTab.Controls[0];

            view.Document.RemoveStylizerAll();
            view.Document.Add(new CsharpKeywordStylizer());
        }

        private void cToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            var view = (ViewControl) tbPages.SelectedTab.Controls[0];

            view.Document.RemoveStylizerAll();
            view.Document.Add(new CKeywordStylizer());
        }

        private void offToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (tbPages.SelectedTab == null) return;
            var view = (ViewControl)tbPages.SelectedTab.Controls[0];

            view.Document.RemoveStylizerAll();
        }
    }

    class CsharpKeywordStylizer : IStylizer
    {
        static readonly string[] Keywors = { 
            "int", "long", "class", "private", "void", "public", "namespace", "using", "string"
        };

        IStyleFace keywordFace;

        public void Stylize(IStylizerSource source)
        {
            if (source.Document.CreateFace(this, "scharp-keyword") != null)
            {
                keywordFace = source.Document.FindFace("scharp-keyword");
                keywordFace.FaceStyle.Background = Color.GhostWhite;
                keywordFace.FaceStyle.Foreground = Color.FromArgb(0, 0, 150);
                keywordFace.FaceStyle.FontStyle = FontStyle.Bold;
            }

            foreach (var line in source.Range.Lines)
            {
                stylizeLine(source, line);
            }
        }

        private void stylizeLine(IStylizerSource source, int line)
        {
            var row = source.Document.Rows[line];
            foreach (var s in Keywors)
            {
                var index = 0;
                while (-1 != (index = row.Raw.IndexOf(s, index, StringComparison.OrdinalIgnoreCase)))
                {
                    var docRange = new DocumentRange
                    {
                        Start = new DocumentPoint(line, index),
                        End = new DocumentPoint(line, index + s.Length)
                    };

                    source.AssignFace(docRange, keywordFace);

                    index += s.Length;
                }
            }
        }
    }

    class CKeywordStylizer : IStylizer
    {
        static readonly string[] Keywors = { 
            "int", "long", "private", "void", "public", "namespace"
        };

        IStyleFace keywordFace;

        public void Stylize(IStylizerSource source)
        {
            if (source.Document.CreateFace(this, "c-keyword") != null)
            {
                keywordFace = source.Document.FindFace("c-keyword");
                keywordFace.FaceStyle.Foreground = Color.Green;
                keywordFace.FaceStyle.FontStyle = FontStyle.Bold;
            }

            foreach (var line in source.Range.Lines)
            {
                stylizeLine(source, line);
            }
        }

        private void stylizeLine(IStylizerSource source, int line)
        {
            var row = source.Document.Rows[line];
            foreach (var s in Keywors)
            {
                var index = 0;
                while (-1 != (index = row.Raw.IndexOf(s, index, StringComparison.OrdinalIgnoreCase)))
                {
                    var docRange = new DocumentRange
                    {
                        Start = new DocumentPoint(line, index),
                        End = new DocumentPoint(line, index + s.Length)
                    };

                    source.AssignFace(docRange, keywordFace);

                    index += s.Length;
                }
            }
        }
    }
}