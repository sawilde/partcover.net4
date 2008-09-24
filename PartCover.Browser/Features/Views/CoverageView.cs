using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
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

        protected override void OnPaint(PaintEventArgs pe)
        {
            base.OnPaint(pe);
        }

        public override void attach(IServiceContainer container, IProgressTracker tracker)
        {
            base.attach(container, tracker);
            tracker.setMessage("Advise for selection events");
            Services.getService<IReportItemSelectionService>().SelectionChanged += ReportItemSelectionChanged;
        }

        public override void detach(IServiceContainer container, IProgressTracker tracker)
        {
            tracker.setMessage("Unadvise for selection events");
            Services.getService<IReportItemSelectionService>().SelectionChanged -= ReportItemSelectionChanged;
            base.detach(container, tracker);
        }

        void ReportItemSelectionChanged(object sender, EventArgs e)
        {
            IReportItemSelectionService service = Services.getService<IReportItemSelectionService>();

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

        private void ClearPane() {
            lbBlocks.Clear();
        }

        private void ShowMethodBlocks(ICoveredVariant variant)
        {
            this.variant = variant;

            lbBlocks.BeginUpdate();
            lbBlocks.Clear();

            if (lbBlocks.Columns.Count < 4)
            {
                lbBlocks.Columns.Add("Block", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Block Length", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Visit Count", 100, HorizontalAlignment.Right);
                lbBlocks.Columns.Add("Have Source", 100, HorizontalAlignment.Right);
            }

            foreach (CoverageReport.InnerBlock ib in variant.Blocks)
            {
                Color itemColor = ColorProvider.GetForeColorForBlock(CoverageReportHelper.GetBlockCoverage(ib));

                ListViewItem lvi = new ListViewItem();
                lvi.ForeColor = itemColor;

                lvi.Text = "Block " + ib.position;
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
            List<uint> files = new List<uint>();
            foreach (CoverageReport.InnerBlock ib in variant.Blocks)
            {
                if (!files.Contains(ib.fileId)) files.Add(ib.fileId);
            }

            foreach (uint file in files)
            {
                ParseFile(file, Array.FindAll(variant.Blocks, delegate(CoverageReport.InnerBlock b)
                    {
                        return b.fileId == file;
                    }));
            }
        }

        private void ParseFile(uint file, IEnumerable<CoverageReport.InnerBlock> filePoints)
        {
            string filePath = Services.getService<ICoverageReportService>().Report.getFilePath(file);
            if (filePath == null || !File.Exists(filePath))
                return;

            ViewControl sourceViewer = GetFileTextBox(file);

            if (sourceViewer == null)
            {
                sourceViewer = new ViewControl();
                sourceViewer.Dock = DockStyle.Fill;
                sourceViewer.Document = CreateDocument(filePath);
                sourceViewer.BorderStyle = BorderStyle.FixedSingle;
                sourceViewer.View.ViewStyle.HideInactiveCursor = false;
                sourceViewer.View.ViewStyle.HideInactiveSelection = false;

                FileTag record = new FileTag();
                record.fileId = file;

                TabPage page = new TabPage();
                page.Tag = record;
                page.Text = Path.GetFileName(filePath);
                page.Controls.Add(sourceViewer);

                pnTabs.TabPages.Add(page);

                pnTabs.SelectedTab = page;
            }

            List<CoverageReport.InnerBlock> bList;

            if (Settings.Default.HighlightAllFile)
            {
                bList = new List<CoverageReport.InnerBlock>();

                Services.getService<ICoverageReportService>().Report.forEachBlock(
                    delegate(CoverageReport.InnerBlock bd)
                    {
                        if (bd.fileId == file) bList.Add(bd);
                    });
            }
            else
            {
                sourceViewer.Document.removeStylizers();
                bList = new List<CoverageReport.InnerBlock>(filePoints);
            }

            sourceViewer.Document.addStylizer(new BlockStylizer(bList.ToArray()));
        }

        private ViewControl GetFileTextBox(uint file)
        {
            TabPage page = GetFileTab(file);
            if (page != null)
                return (ViewControl)page.Controls[0];
            return null;
        }

        private static Document CreateDocument(string filePath)
        {
            Document document = Document.createFromFile(filePath);

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
                if (page.Tag is FileTag)
                {
                    FileTag tag = (FileTag)page.Tag;

                    if (tag.fileId == file)
                        return page;
                }
            }
            return null;
        }

        private void lbBlocks_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lbBlocks.SelectedIndices.Count == 0)
                return;

            int blockIndex = lbBlocks.SelectedIndices[0];
            CoverageReport.InnerBlock ib = variant.Blocks[blockIndex];
            if (ib.fileId == 0)
                return;

            TabPage page = GetFileTab(ib.fileId);
            if (page == null)
                return;

            pnTabs.SelectedTab = page;
            lbBlocks.Focus();
            lbBlocks.Select();

            ViewControl rtb = GetFileTextBox(ib.fileId);
            if (rtb == null)
                return;

            rtb.View.moveCaretTo(new Point((int)ib.startColumn - 1, (int)ib.startLine - 1));
        }


        private class FileTag
        {
            public uint fileId;
        }

        private class BlockStylizer : Stylizer
        {
            private const string CoveredStyle = "blockstylizer-good";
            private const string UncoveredStyle = "blockstylizer-bad";

            private readonly CoverageReport.InnerBlock[] points;

            public BlockStylizer(CoverageReport.InnerBlock[] points)
            {
                this.points = points;
            }

            public void stylize(StylizerSource source)
            {
                foreach (CoverageReport.InnerBlock b in points)
                {
                    if (b.fileId <= 0)
                        continue;

                    DocumentRange range = new DocumentRange();
                    range.Start = new DocumentPoint((int)(b.startLine - 1), (int)(b.startColumn - 1));
                    range.End = new DocumentPoint((int)(b.endLine - 1), (int)(b.endColumn - 1));

                    StyleFace face = getFace(source, b.visitCount);

                    source.setFace(range, face);
                }
            }

            private StyleFace getFace(StylizerSource source, uint count)
            {
                string faceName = count > 0 ? CoveredStyle : UncoveredStyle;

                StyleFace face = source.Document.createFace(this, faceName);
                if (face == null)
                    return source.Document.getFace(faceName);

                face.FaceStyle.Background = count > 0
                    ? Settings.Default.ViewPaneCoveredBlockBackroung
                    : Settings.Default.ViewPaneUncoveredBlockBackroung;

                return face;
            }
        }
    }
}
