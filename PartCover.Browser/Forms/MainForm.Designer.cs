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
            this.tvItems = new ReportTree();
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
            // mmSep2
            // 
            this.mmSep2.Index = 1;
            this.mmSep2.Text = "-";
            // 
            // mmFileOpen
            // 
            this.mmFileOpen.Index = 2;
            this.mmFileOpen.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.mmFileOpen.Text = "&Open Report...";
            this.mmFileOpen.Click += new System.EventHandler(this.mmFileOpen_Click);
            // 
            // mmFileSaveAs
            // 
            this.mmFileSaveAs.Index = 3;
            this.mmFileSaveAs.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.mmFileSaveAs.Text = "&Save Report As ...";
            this.mmFileSaveAs.Click += new System.EventHandler(this.mmFileSaveAs_Click);
            // 
            // mmSep1
            // 
            this.mmSep1.Index = 4;
            this.mmSep1.Text = "-";
            // 
            // mmFileExit
            // 
            this.mmFileExit.Index = 5;
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
            this.splitter1.Location = new System.Drawing.Point(197, 0);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(3, 422);
            this.splitter1.TabIndex = 2;
            this.splitter1.TabStop = false;
            // 
            // tvItems
            // 
            this.tvItems.Dock = System.Windows.Forms.DockStyle.Left;
            this.tvItems.ImageIndex = 0;
            this.tvItems.Location = new System.Drawing.Point(0, 0);
            this.tvItems.Name = "tvItems";
            this.tvItems.SelectedImageIndex = 0;
            this.tvItems.ServiceContainer = null;
            this.tvItems.Size = new System.Drawing.Size(197, 422);
            this.tvItems.Sorted = true;
            this.tvItems.TabIndex = 0;
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(680, 422);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.tvItems);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.IsMdiContainer = true;
            this.Menu = this.mm;
            this.Name = "MainForm";
            this.Text = "PartCover coverage browser";
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
    }
}