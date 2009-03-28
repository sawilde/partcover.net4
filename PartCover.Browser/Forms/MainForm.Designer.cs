using PartCover.Browser.Features.Controls;

namespace PartCover.Browser.Forms
{
    partial class MainForm {
        private System.Windows.Forms.MainMenu mm;
        private System.Windows.Forms.MenuItem mmFile;
        private System.Windows.Forms.MenuItem mmFileOpen;
        private System.Windows.Forms.OpenFileDialog dlgOpen;
        private ReportTree tvItems;
        private System.Windows.Forms.MenuItem mmFileExit;
        private System.Windows.Forms.MenuItem mmFileSaveAs;
        private System.Windows.Forms.MenuItem mmRunTarget;
        private System.Windows.Forms.MenuItem mmSep1;
        private System.Windows.Forms.MenuItem mmSep2;
        private System.Windows.Forms.SaveFileDialog dlgSave;
        private System.ComponentModel.IContainer components;

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.mm = new System.Windows.Forms.MainMenu(this.components);
            this.mmFile = new System.Windows.Forms.MenuItem();
            this.mmRunTarget = new System.Windows.Forms.MenuItem();
            this.mmFileShowSkipped = new System.Windows.Forms.MenuItem();
            this.mmFileShowLog = new System.Windows.Forms.MenuItem();
            this.mmSep2 = new System.Windows.Forms.MenuItem();
            this.mmFileOpen = new System.Windows.Forms.MenuItem();
            this.mmFileSaveAs = new System.Windows.Forms.MenuItem();
            this.mmSep1 = new System.Windows.Forms.MenuItem();
            this.mmFileExit = new System.Windows.Forms.MenuItem();
            this.miViews = new System.Windows.Forms.MenuItem();
            this.miHtml = new System.Windows.Forms.MenuItem();
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.miWindows = new System.Windows.Forms.MenuItem();
            this.miHelp = new System.Windows.Forms.MenuItem();
            this.miSettings = new System.Windows.Forms.MenuItem();
            this.miAbout = new System.Windows.Forms.MenuItem();
            this.dlgOpen = new System.Windows.Forms.OpenFileDialog();
            this.dlgSave = new System.Windows.Forms.SaveFileDialog();
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.rtbNodeProps = new System.Windows.Forms.RichTextBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.gbProps = new System.Windows.Forms.GroupBox();
            this.tvItems = new PartCover.Browser.Features.Controls.ReportTree();
            this.panel1.SuspendLayout();
            this.gbProps.SuspendLayout();
            this.SuspendLayout();
            // 
            // mm
            // 
            this.mm.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.mmFile,
            this.miViews,
            this.miWindows,
            this.miHelp});
            // 
            // mmFile
            // 
            this.mmFile.Index = 0;
            this.mmFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.mmRunTarget,
            this.mmFileShowSkipped,
            this.mmFileShowLog,
            this.mmSep2,
            this.mmFileOpen,
            this.mmFileSaveAs,
            this.mmSep1,
            this.mmFileExit});
            this.mmFile.Text = "&File";
            // 
            // mmRunTarget
            // 
            this.mmRunTarget.Index = 0;
            this.mmRunTarget.Text = "&Run Target...";
            this.mmRunTarget.Click += new System.EventHandler(this.mmRunTarget_Click);
            // 
            // mmFileShowSkipped
            // 
            this.mmFileShowSkipped.Index = 1;
            this.mmFileShowSkipped.Text = "Show skipped items...";
            this.mmFileShowSkipped.Click += new System.EventHandler(this.mmFileShowSkipped_Click);
            // 
            // mmFileShowLog
            // 
            this.mmFileShowLog.Index = 2;
            this.mmFileShowLog.Text = "Show run log...";
            this.mmFileShowLog.Click += new System.EventHandler(this.mmFileShowLog_Click);
            // 
            // mmSep2
            // 
            this.mmSep2.Index = 3;
            this.mmSep2.Text = "-";
            // 
            // mmFileOpen
            // 
            this.mmFileOpen.Index = 4;
            this.mmFileOpen.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.mmFileOpen.Text = "&Open Report...";
            this.mmFileOpen.Click += new System.EventHandler(this.mmFileOpen_Click);
            // 
            // mmFileSaveAs
            // 
            this.mmFileSaveAs.Index = 5;
            this.mmFileSaveAs.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.mmFileSaveAs.Text = "&Save Report As ...";
            this.mmFileSaveAs.Click += new System.EventHandler(this.mmFileSaveAs_Click);
            // 
            // mmSep1
            // 
            this.mmSep1.Index = 6;
            this.mmSep1.Text = "-";
            // 
            // mmFileExit
            // 
            this.mmFileExit.Index = 7;
            this.mmFileExit.Text = "E&xit";
            this.mmFileExit.Click += new System.EventHandler(this.mmFileExit_Click);
            // 
            // miViews
            // 
            this.miViews.Index = 1;
            this.miViews.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miHtml,
            this.menuItem1});
            this.miViews.Text = "&Views";
            // 
            // miHtml
            // 
            this.miHtml.Index = 0;
            this.miHtml.Text = "&HTML View";
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 1;
            this.menuItem1.Text = "-";
            // 
            // miWindows
            // 
            this.miWindows.Index = 2;
            this.miWindows.MdiList = true;
            this.miWindows.Text = "&Windows";
            // 
            // miHelp
            // 
            this.miHelp.Index = 3;
            this.miHelp.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.miSettings,
            this.miAbout});
            this.miHelp.Text = "&Help";
            // 
            // miSettings
            // 
            this.miSettings.Index = 0;
            this.miSettings.Text = "&Settings...";
            this.miSettings.Click += new System.EventHandler(this.miSettings_Click);
            // 
            // miAbout
            // 
            this.miAbout.Index = 1;
            this.miAbout.Text = "&About...";
            this.miAbout.Click += new System.EventHandler(this.miAbout_Click);
            // 
            // dlgOpen
            // 
            this.dlgOpen.Filter = "PartCover report (*.xml)|*.xml";
            // 
            // dlgSave
            // 
            this.dlgSave.Filter = "PartCover report (*.xml)|*.xml";
            // 
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(200, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 380);
            this.splitter1.TabIndex = 2;
            this.splitter1.TabStop = false;
            // 
            // rtbNodeProps
            // 
            this.rtbNodeProps.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.rtbNodeProps.Dock = System.Windows.Forms.DockStyle.Fill;
            this.rtbNodeProps.Location = new System.Drawing.Point(3, 16);
            this.rtbNodeProps.Name = "rtbNodeProps";
            this.rtbNodeProps.ReadOnly = true;
            this.rtbNodeProps.Size = new System.Drawing.Size(184, 81);
            this.rtbNodeProps.TabIndex = 4;
            this.rtbNodeProps.Text = "";
            this.rtbNodeProps.WordWrap = false;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.tvItems);
            this.panel1.Controls.Add(this.gbProps);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Left;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Padding = new System.Windows.Forms.Padding(5);
            this.panel1.Size = new System.Drawing.Size(200, 380);
            this.panel1.TabIndex = 5;
            // 
            // gbProps
            // 
            this.gbProps.Controls.Add(this.rtbNodeProps);
            this.gbProps.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.gbProps.Location = new System.Drawing.Point(5, 275);
            this.gbProps.Name = "gbProps";
            this.gbProps.Size = new System.Drawing.Size(190, 100);
            this.gbProps.TabIndex = 5;
            this.gbProps.TabStop = false;
            this.gbProps.Text = "Node properties";
            // 
            // tvItems
            // 
            this.tvItems.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvItems.ImageIndex = 0;
            this.tvItems.Location = new System.Drawing.Point(5, 5);
            this.tvItems.Name = "tvItems";
            this.tvItems.SelectedImageIndex = 0;
            this.tvItems.ServiceContainer = null;
            this.tvItems.Size = new System.Drawing.Size(190, 270);
            this.tvItems.Sorted = true;
            this.tvItems.TabIndex = 0;
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(680, 380);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.IsMdiContainer = true;
            this.Menu = this.mm;
            this.Name = "MainForm";
            this.Text = "PartCover coverage browser";
            this.panel1.ResumeLayout(false);
            this.gbProps.ResumeLayout(false);
            this.ResumeLayout(false);

        }
        #endregion

        private System.Windows.Forms.MenuItem miSettings;
        private System.Windows.Forms.MenuItem miAbout;
        private System.Windows.Forms.MenuItem miHtml;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.MenuItem miWindows;
        private System.Windows.Forms.MenuItem miViews;
        private System.Windows.Forms.MenuItem menuItem1;
        private System.Windows.Forms.MenuItem miHelp;
        private System.Windows.Forms.MenuItem mmFileShowSkipped;
        private System.Windows.Forms.MenuItem mmFileShowLog;
        private System.Windows.Forms.RichTextBox rtbNodeProps;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox gbProps;
    }
}