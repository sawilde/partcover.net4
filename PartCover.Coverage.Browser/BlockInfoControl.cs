using System;
using System.Text;
using System.IO;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Runtime.InteropServices;

using PartCover.Framework.Walkers;

namespace Win32 {
    class Helper {
        public const int EM_LINEINDEX = 0x00BB;

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int SendMessage(System.IntPtr hWnd, System.UInt32 Msg, System.UInt32 wParam, System.UInt32 lParam);
    }
}

namespace PartCover.Coverage.Browser
{
	public class BlockInfoControl : System.Windows.Forms.UserControl
	{
        private System.Windows.Forms.ListView lbBlocks;
        private System.Windows.Forms.Panel pnFiles;
        private System.Windows.Forms.TabControl pnTabs;
		private System.ComponentModel.Container components = null;

		public BlockInfoControl()
		{
			InitializeComponent();
		}

		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.lbBlocks = new System.Windows.Forms.ListView();
            this.pnFiles = new System.Windows.Forms.Panel();
            this.pnTabs = new System.Windows.Forms.TabControl();
            this.pnFiles.SuspendLayout();
            this.SuspendLayout();
            // 
            // lbBlocks
            // 
            this.lbBlocks.Dock = System.Windows.Forms.DockStyle.Top;
            this.lbBlocks.FullRowSelect = true;
            this.lbBlocks.GridLines = true;
            this.lbBlocks.Location = new System.Drawing.Point(0, 0);
            this.lbBlocks.Name = "lbBlocks";
            this.lbBlocks.Size = new System.Drawing.Size(536, 112);
            this.lbBlocks.TabIndex = 0;
            this.lbBlocks.View = System.Windows.Forms.View.Details;
            this.lbBlocks.SelectedIndexChanged += new System.EventHandler(this.lbBlocks_SelectedIndexChanged);
            // 
            // pnFiles
            // 
            this.pnFiles.Controls.Add(this.pnTabs);
            this.pnFiles.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pnFiles.Location = new System.Drawing.Point(0, 112);
            this.pnFiles.Name = "pnFiles";
            this.pnFiles.Size = new System.Drawing.Size(536, 368);
            this.pnFiles.TabIndex = 1;
            // 
            // pnTabs
            // 
            this.pnTabs.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pnTabs.Location = new System.Drawing.Point(0, 0);
            this.pnTabs.Name = "pnTabs";
            this.pnTabs.SelectedIndex = 0;
            this.pnTabs.Size = new System.Drawing.Size(536, 368);
            this.pnTabs.TabIndex = 0;
            // 
            // BlockInfoControl
            // 
            this.BackColor = System.Drawing.SystemColors.Window;
            this.Controls.Add(this.pnFiles);
            this.Controls.Add(this.lbBlocks);
            this.ForeColor = System.Drawing.SystemColors.WindowText;
            this.Name = "BlockInfoControl";
            this.Size = new System.Drawing.Size(536, 480);
            this.pnFiles.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion

        CoverageReport report;
        CoverageReport.InnerBlockData bData;

        public void SetBlockData(CoverageReport report, CoverageReport.InnerBlockData bData) {
            this.lbBlocks.BeginUpdate();

            this.report = report;
            this.bData = bData;

            lbBlocks.Columns.Add("Block", 100, HorizontalAlignment.Right);
            lbBlocks.Columns.Add("Block Length", 100, HorizontalAlignment.Right);
            lbBlocks.Columns.Add("Visit Count", 100, HorizontalAlignment.Right);
            lbBlocks.Columns.Add("Have Source", 100, HorizontalAlignment.Right);

            foreach(CoverageReport.InnerBlock ib in bData.blocks) {
                Color itemColor = Helpers.ColorProvider.GetForeColorForBlock(CoverageReportHelper.GetBlockCoverage(ib));

                ListViewItem lvi = new ListViewItem();
                lvi.ForeColor = itemColor;

                lvi.Text = "Block " + ib.position;
                lvi.SubItems.Add(ib.blockLen.ToString());
                lvi.SubItems.Add(ib.visitCount.ToString());
                lvi.SubItems.Add(ib.fileId > 0 ? "yes" : "no");

                lbBlocks.Items.Add(lvi);

                if(ib.fileId > 0) {
                    RichTextBox rtb = GetFileTextBox(CoverageReportHelper.GetFileUrl(report, ib.fileId));
                    if (rtb != null) {
                        long slchar = Win32.Helper.SendMessage(rtb.Handle, Win32.Helper.EM_LINEINDEX, ib.startLine - 1, 0) + ib.startColumn - 1;
                        long elchar = Win32.Helper.SendMessage(rtb.Handle, Win32.Helper.EM_LINEINDEX, ib.endLine - 1, 0) + ib.endColumn - 1;
                        rtb.Select((int)slchar, (int)(elchar - slchar + 1));
                        rtb.SelectionColor = itemColor;
                    }
                }
            }

            if (lbBlocks.Items.Count > 0)
                lbBlocks.Items[0].Selected = true;
            this.lbBlocks.EndUpdate();
        }

        public RichTextBox GetFileTextBox(string fileUrl) {
            foreach(TabPage tabPage in pnTabs.TabPages) {
                if (tabPage.Tag != null && (string)tabPage.Tag == fileUrl) 
                    return tabPage.Controls[0] as RichTextBox;
            }

            TextReader tr = new StreamReader(fileUrl);

            RichTextBox rtb = new RichTextBox();
            rtb.Dock = DockStyle.Fill;
            rtb.Text = tr.ReadToEnd();
            rtb.HideSelection = false;
            rtb.ReadOnly = true;
            rtb.WordWrap = false;
            tr.Close();

            TabPage page = new TabPage();
            page.Tag = fileUrl;
            page.Text = Path.GetFileName(fileUrl);
            page.Controls.Add(rtb);
            pnTabs.TabPages.Add(page);

            return rtb;
        }

        private void lbBlocks_SelectedIndexChanged(object sender, System.EventArgs e) {
            if (lbBlocks.SelectedIndices.Count == 0)
                return;

            int blockIndex = lbBlocks.SelectedIndices[0];
            CoverageReport.InnerBlock ib = bData.blocks[blockIndex];
            if (ib.fileId == 0)
                return;

            RichTextBox rtb = GetFileTextBox(CoverageReportHelper.GetFileUrl(report, ib.fileId));
            if (rtb == null)
                return;

            long slchar = Win32.Helper.SendMessage(rtb.Handle, Win32.Helper.EM_LINEINDEX, ib.startLine - 1, 0) + ib.startColumn - 1;
            long elchar = Win32.Helper.SendMessage(rtb.Handle, Win32.Helper.EM_LINEINDEX, ib.endLine - 1, 0) + ib.endColumn - 1;
            rtb.Focus();
            rtb.Select((int)slchar, (int)(elchar - slchar + 1));
            rtb.ScrollToCaret();
            ((Control)sender).Focus();
        }
	}
}
