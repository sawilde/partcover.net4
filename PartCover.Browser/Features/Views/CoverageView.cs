using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;
using PartCover.Browser.Helpers;
using PartCover.Browser.Properties;
using PartCover.Framework.Walkers;
using PartViewer;
using PartViewer.Model;

namespace PartCover.Browser.Features.Views
{
    public partial class CoverageView : ReportView
    {
        private ICoveredVariant variant;

        public CoverageView()
        {
            InitializeComponent();
        }

        public override void attach(IServiceContainer container, IProgressTracker tracker)
        {
            base.attach(container, tracker);
            tracker.AppendMessage("Advise for selection events");
            Services.getService<IReportItemSelectionService>().SelectionChanged += ReportItemSelectionChanged;
        }

        public override void detach(IServiceContainer container, IProgressTracker tracker)
        {
            tracker.AppendMessage("Unadvise for selection events");
            Services.getService<IReportItemSelectionService>().SelectionChanged -= ReportItemSelectionChanged;
            base.detach(container, tracker);
        }

        void ReportItemSelectionChanged(object sender, EventArgs e)
        {
            var service = Services.getService<IReportItemSelectionService>();

            if (service.SelectedItem is IMethod)
            {
                ShowMethodBlocks((IMethod)service.SelectedItem);
            }
            else if (service.SelectedItem is ICoveredVariant)
            {
                ShowMethodBlocks((ICoveredVariant)service.SelectedItem);
            }
            else
            {
                ClearPane();
            }
        }

        private void ShowMethodBlocks(IMethod iMethod)
        {
            if (iMethod.CoveredVariants.Length == 1)
            {
                ShowMethodBlocks(iMethod.CoveredVariants[0]);
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

        private void ShowMethodBlocks(ICoveredVariant coveredVariant)
        {
            variant = coveredVariant;

            lbBlocks.BeginUpdate();
            lbBlocks.Clear();

            if (lbBlocks.Columns.Count < 4)
            {
                lbBlocks.Columns.Add("Block", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Block Length", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Visit Count", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Have Source", 100, HorizontalAlignment.Right);
            }

            foreach (var ib in coveredVariant.Blocks)
            {
                var itemColor = ColorProvider.GetForeColorForBlock(CoverageReportHelper.GetBlockCoverage(ib));

                var lvi = new ListViewItem {
                    ForeColor = itemColor, 
                    Text = ("Block " + ib.position)
                };

                lvi.SubItems.Add(ib.blockLen.ToString());
                lvi.SubItems.Add(ib.visitCount.ToString());
                lvi.SubItems.Add(ib.fileId > 0 ? "yes" : "no");

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
            var files = new List<uint>();
            foreach (var ib in variant.Blocks)
            {
                if (!files.Contains(ib.fileId)) files.Add(ib.fileId);
            }

            foreach (var file in files)
            {
                var u = file;
                ParseFile(file, Array.FindAll(variant.Blocks, b => b.fileId == u));
            }
        }

        private void ParseFile(uint file, IEnumerable<CoverageReport.InnerBlock> filePoints)
        {
            var filePath = Services.getService<ICoverageReportService>().Report.ResolveFilePath(file);
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

            List<CoverageReport.InnerBlock> bList;

            if (Settings.Default.HighlightAllFile)
            {
                bList = new List<CoverageReport.InnerBlock>();

                Services.getService<ICoverageReportService>().Report.ForEachBlock(
                    bd => { if (bd.fileId == file) bList.Add(bd); });
            }
            else
            {
                sourceViewer.Document.RemoveStylizerAll();
                bList = new List<CoverageReport.InnerBlock>(filePoints);
            }

            sourceViewer.Document.Add(new BlockStylizer(bList.ToArray()));
        }

        private ViewControl GetFileTextBox(uint file)
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

        private TabPage GetFileTab(uint file)
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
            var ib = variant.Blocks[blockIndex];
            if (ib.fileId == 0)
                return;

            var page = GetFileTab(ib.fileId);
            if (page == null)
                return;

            pnTabs.SelectedTab = page;
            lbBlocks.Focus();
            lbBlocks.Select();

            var rtb = GetFileTextBox(ib.fileId);
            if (rtb == null)
                return;

            rtb.View.MoveCaretTo(new Point((int)ib.startColumn - 1, (int)ib.startLine - 1));
        }


        private class FileTag
        {
            public uint FileId { get; set; }
        }

        private class BlockStylizer : IStylizer
        {
            private const string CoveredStyle = "blockstylizer-good";
            private const string UncoveredStyle = "blockstylizer-bad";

            private readonly CoverageReport.InnerBlock[] points;

            public BlockStylizer(CoverageReport.InnerBlock[] points)
            {
                this.points = points;
            }

            public void Stylize(IStylizerSource source)
            {
                foreach (var b in points)
                {
                    if (b.fileId <= 0)
                        continue;

                    var range = new DocumentRange
                    {
                        Start = new DocumentPoint((int)(b.startLine - 1), (int)(b.startColumn - 1)),
                        End = new DocumentPoint((int)(b.endLine - 1), (int)(b.endColumn - 1))
                    };

                    source.AssignFace(range, getFace(source, b.visitCount));
                }
            }

            private IStyleFace getFace(IStylizerSource source, uint count)
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
    }
}
