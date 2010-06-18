using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using PartCover.Browser.Api;
using PartCover.Browser.Helpers;
using PartCover.Browser.Properties;
using PartCover.Framework;
using PartCover.Framework.Data;
using PartViewer;
using PartViewer.Model;

namespace PartCover.Browser.Features.Views
{
    public partial class CoverageView 
        : ReportView
        , ITreeItemSelectionHandler
    {
        public MethodEntry Method { get; private set; }

        public CoverageView()
        {
            InitializeComponent();
        }

        public void ShowMethodBlocks(MethodEntry methodEntry)
        {
            Method = methodEntry;
            if (methodEntry != null)
            {
                BindMethodBlocks(methodEntry.Blocks);
            }
            else
            {
                ClearPane();
            }
        }

        private void ClearPane()
        {
            lbBlocks.Clear();
        }

        private void BindMethodBlocks(IEnumerable<MethodBlock> blocks)
        {
            lbBlocks.BeginUpdate();
            lbBlocks.Clear();

            if (lbBlocks.Columns.Count < 4)
            {
                lbBlocks.Columns.Add("Block", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Block Length", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Visit Count", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Have Source", 100, HorizontalAlignment.Right);
            }

            foreach (var ib in blocks)
            {
                var itemColor = ColorProvider.GetForeColorForBlock(ReportHelper.GetBlockCoverage(ib));

                var lvi = new ListViewItem
                {
                    ForeColor = itemColor,
                    Text = ("Block " + ib.Offset)
                };

                lvi.SubItems.Add(ib.Length.ToString());
                lvi.SubItems.Add(ib.VisitCount.ToString());
                lvi.SubItems.Add(ib.File > 0 ? "yes" : "no");

                lbBlocks.Items.Add(lvi);
            }

            BuildSourceViews();

            lbBlocks.EndUpdate();

            Invalidate();

            if (lbBlocks.Items.Count > 0)
                lbBlocks.Items[0].Selected = true;
        }

        private void BuildSourceViews()
        {
            var files = new List<int>();
            foreach (var ib in Method.Blocks)
            {
                if (ib.File > 0 && !files.Contains(ib.File)) files.Add(ib.File);
            }

            foreach (var file in files)
            {
                var u = file;
                ParseFile(file,
                    new List<MethodBlock>(Method.Blocks).FindAll(b => b.File == u));
            }
        }

        private void ParseFile(int file, IEnumerable<MethodBlock> filePoints)
        {
            var filePath = Services.getService<IReportService>().Report.ResolveFilePath(file);
            if (filePath == null || !File.Exists(filePath))
                return;

            var sourceViewer = GetFileTextBox(file);
            if (sourceViewer == null)
            {
                sourceViewer = new ViewControl
                {
                    Dock = DockStyle.Fill,
                    Document = CreateDocument(filePath),
                    BorderStyle = BorderStyle.FixedSingle
                };
                sourceViewer.View.ViewStyle.HideInactiveCursor = false;
                sourceViewer.View.ViewStyle.HideInactiveSelection = false;

                var record = new FileTag
                {
                    FileId = file
                };

                var page = new TabPage
                {
                    Tag = record,
                    Text = Path.GetFileName(filePath)
                };
                page.Controls.Add(sourceViewer);

                pnTabs.TabPages.Add(page);

                pnTabs.SelectedTab = page;
            }

            List<MethodBlock> bList;

            if (Settings.Default.HighlightAllFile)
            {
                bList = new List<MethodBlock>();

                ReportHelper.ForEachBlock(Services.getService<IReportService>().Report,
                    bd => { if (bd.File == file) bList.Add(bd); });
            }
            else
            {
                sourceViewer.Document.RemoveStylizerAll();
                bList = new List<MethodBlock>(filePoints);
            }

            sourceViewer.Document.Add(new BlockStylizer(bList.ToArray()));
        }

        private ViewControl GetFileTextBox(int file)
        {
            var page = GetFileTab(file);
            if (page != null)
                return (ViewControl)page.Controls[0];
            return null;
        }

        private static Document CreateDocument(string filePath)
        {
            var document = Document.CreateFromFile(filePath);
            document.Style.FontName = Settings.Default.ViewPaneFont.Name;
            document.Style.FontHeight = Settings.Default.ViewPaneFont.SizeInPoints;
            document.Style.Foreground = Settings.Default.ViewPaneForeground;
            document.Style.Background = Settings.Default.ViewPaneBackground;
            return document;
        }

        private TabPage GetFileTab(int file)
        {
            foreach (TabPage page in pnTabs.TabPages)
            {
                if (!(page.Tag is FileTag))
                {
                    continue;
                }

                var tag = (FileTag)page.Tag;
                if (tag.FileId == file)
                    return page;
            }
            return null;
        }

        private void lbBlocks_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lbBlocks.SelectedIndices.Count == 0)
                return;

            var blockIndex = lbBlocks.SelectedIndices[0];
            var ib = Method.Blocks[blockIndex];
            if (ib.File == 0)
                return;

            var page = GetFileTab(ib.File);
            if (page == null)
                return;

            pnTabs.SelectedTab = page;
            lbBlocks.Focus();
            lbBlocks.Select();

            var rtb = GetFileTextBox(ib.File);
            if (rtb == null)
                return;

            rtb.View.MoveCaretTo(new Point(ib.Start.Column - 1, ib.Start.Line - 1));
        }


        private class FileTag
        {
            public int FileId { get; set; }
        }

        private class BlockStylizer : IStylizer
        {
            private const string CoveredStyle = "blockstylizer-good";
            private const string UncoveredStyle = "blockstylizer-bad";

            private readonly MethodBlock[] points;

            public BlockStylizer(MethodBlock[] points)
            {
                this.points = points;
            }

            public void Stylize(IStylizerSource source)
            {
                foreach (var b in points)
                {
                    if (b.File <= 0)
                        continue;

                    var range = new DocumentRange
                    {
                        Start = new DocumentPoint(b.Start.Line - 1, b.Start.Column - 1),
                        End = new DocumentPoint(b.End.Line - 1, b.End.Column - 1)
                    };

                    source.AssignFace(range, getFace(source, b.VisitCount));
                }
            }

            private IStyleFace getFace(IStylizerSource source, int count)
            {
                var faceName = count > 0 ? CoveredStyle : UncoveredStyle;

                var face = source.Document.CreateFace(this, faceName);
                if (face == null)
                    return source.Document.FindFace(faceName);

                face.FaceStyle.Background = count > 0
                    ? Settings.Default.ViewPaneCoveredBlockBackroung
                    : Settings.Default.ViewPaneUncoveredBlockBackroung;

                return face;
            }
        }

        public void Select(AssemblyEntry assembly)
        {
            Deselect();
        }

        public void Select(AssemblyEntry assembly, string namespacePath)
        {
            Deselect();
        }

        public void Select(TypedefEntry typedef)
        {
            Deselect();
        }

        public void Select(MethodEntry method)
        {
            ShowMethodBlocks(method);
        }

        public void Deselect()
        {
            ShowMethodBlocks(null);
        }
    }
}
