using System;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using PartCover.Framework;

namespace PartCover.Coverage.Browser
{
	public class RunTargetForm : System.Windows.Forms.Form
	{
		private System.ComponentModel.Container components = null;
        private System.Windows.Forms.GroupBox gbTarget;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbArgs;
        private System.Windows.Forms.Button btnBrowseDir;
        private System.Windows.Forms.Button btnBrowseTarget;
        private System.Windows.Forms.Label lbDir;
        private System.Windows.Forms.Label lbTarget;
        private System.Windows.Forms.TextBox tbWorkingDir;
        private System.Windows.Forms.TextBox tbPath;
        private System.Windows.Forms.GroupBox gbRules;
        private System.Windows.Forms.TextBox tbRules;
        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.FolderBrowserDialog dlgBrowse;
        private System.Windows.Forms.Button btnLoad;
        private System.Windows.Forms.Button btnSave;
        private System.Windows.Forms.SaveFileDialog dlgSave;
        private System.Windows.Forms.OpenFileDialog dlgOpen;

        public RunTargetForm()
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

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(RunTargetForm));
            this.dlgOpen = new System.Windows.Forms.OpenFileDialog();
            this.gbTarget = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbArgs = new System.Windows.Forms.TextBox();
            this.btnBrowseDir = new System.Windows.Forms.Button();
            this.btnBrowseTarget = new System.Windows.Forms.Button();
            this.lbDir = new System.Windows.Forms.Label();
            this.lbTarget = new System.Windows.Forms.Label();
            this.tbWorkingDir = new System.Windows.Forms.TextBox();
            this.tbPath = new System.Windows.Forms.TextBox();
            this.gbRules = new System.Windows.Forms.GroupBox();
            this.tbRules = new System.Windows.Forms.TextBox();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.dlgBrowse = new System.Windows.Forms.FolderBrowserDialog();
            this.btnLoad = new System.Windows.Forms.Button();
            this.btnSave = new System.Windows.Forms.Button();
            this.dlgSave = new System.Windows.Forms.SaveFileDialog();
            this.gbTarget.SuspendLayout();
            this.gbRules.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbTarget
            // 
            this.gbTarget.Controls.Add(this.label1);
            this.gbTarget.Controls.Add(this.tbArgs);
            this.gbTarget.Controls.Add(this.btnBrowseDir);
            this.gbTarget.Controls.Add(this.btnBrowseTarget);
            this.gbTarget.Controls.Add(this.lbDir);
            this.gbTarget.Controls.Add(this.lbTarget);
            this.gbTarget.Controls.Add(this.tbWorkingDir);
            this.gbTarget.Controls.Add(this.tbPath);
            this.gbTarget.Location = new System.Drawing.Point(8, 8);
            this.gbTarget.Name = "gbTarget";
            this.gbTarget.Size = new System.Drawing.Size(592, 100);
            this.gbTarget.TabIndex = 9;
            this.gbTarget.TabStop = false;
            this.gbTarget.Text = "Target";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(16, 72);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(152, 16);
            this.label1.TabIndex = 15;
            this.label1.Text = "Working Arguments";
            // 
            // tbArgs
            // 
            this.tbArgs.Location = new System.Drawing.Point(176, 71);
            this.tbArgs.Name = "tbArgs";
            this.tbArgs.Size = new System.Drawing.Size(400, 20);
            this.tbArgs.TabIndex = 14;
            this.tbArgs.Text = "";
            // 
            // btnBrowseDir
            // 
            this.btnBrowseDir.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.btnBrowseDir.Location = new System.Drawing.Point(488, 43);
            this.btnBrowseDir.Name = "btnBrowseDir";
            this.btnBrowseDir.Size = new System.Drawing.Size(88, 20);
            this.btnBrowseDir.TabIndex = 13;
            this.btnBrowseDir.Text = "Browse";
            this.btnBrowseDir.Click += new System.EventHandler(this.btnBrowseDir_Click);
            // 
            // btnBrowseTarget
            // 
            this.btnBrowseTarget.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.btnBrowseTarget.Location = new System.Drawing.Point(488, 19);
            this.btnBrowseTarget.Name = "btnBrowseTarget";
            this.btnBrowseTarget.Size = new System.Drawing.Size(88, 20);
            this.btnBrowseTarget.TabIndex = 12;
            this.btnBrowseTarget.Text = "Browse";
            this.btnBrowseTarget.Click += new System.EventHandler(this.btnBrowseTarget_Click);
            // 
            // lbDir
            // 
            this.lbDir.Location = new System.Drawing.Point(16, 44);
            this.lbDir.Name = "lbDir";
            this.lbDir.Size = new System.Drawing.Size(152, 16);
            this.lbDir.TabIndex = 11;
            this.lbDir.Text = "Working Directory";
            // 
            // lbTarget
            // 
            this.lbTarget.Location = new System.Drawing.Point(16, 21);
            this.lbTarget.Name = "lbTarget";
            this.lbTarget.Size = new System.Drawing.Size(152, 16);
            this.lbTarget.TabIndex = 10;
            this.lbTarget.Text = "Executable File";
            // 
            // tbWorkingDir
            // 
            this.tbWorkingDir.Location = new System.Drawing.Point(176, 43);
            this.tbWorkingDir.Name = "tbWorkingDir";
            this.tbWorkingDir.Size = new System.Drawing.Size(304, 20);
            this.tbWorkingDir.TabIndex = 9;
            this.tbWorkingDir.Text = "";
            // 
            // tbPath
            // 
            this.tbPath.Location = new System.Drawing.Point(176, 19);
            this.tbPath.Name = "tbPath";
            this.tbPath.Size = new System.Drawing.Size(304, 20);
            this.tbPath.TabIndex = 8;
            this.tbPath.Text = "";
            // 
            // gbRules
            // 
            this.gbRules.Controls.Add(this.tbRules);
            this.gbRules.Location = new System.Drawing.Point(8, 112);
            this.gbRules.Name = "gbRules";
            this.gbRules.Size = new System.Drawing.Size(592, 176);
            this.gbRules.TabIndex = 10;
            this.gbRules.TabStop = false;
            this.gbRules.Text = "Rules";
            // 
            // tbRules
            // 
            this.tbRules.HideSelection = false;
            this.tbRules.Location = new System.Drawing.Point(8, 16);
            this.tbRules.Multiline = true;
            this.tbRules.Name = "tbRules";
            this.tbRules.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbRules.Size = new System.Drawing.Size(576, 152);
            this.tbRules.TabIndex = 9;
            this.tbRules.Text = "";
            this.tbRules.WordWrap = false;
            // 
            // btnOk
            // 
            this.btnOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOk.Location = new System.Drawing.Point(400, 296);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(96, 23);
            this.btnOk.TabIndex = 11;
            this.btnOk.Text = "Start";
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(504, 296);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(96, 23);
            this.btnCancel.TabIndex = 12;
            this.btnCancel.Text = "Cancel";
            // 
            // btnLoad
            // 
            this.btnLoad.Location = new System.Drawing.Point(112, 296);
            this.btnLoad.Name = "btnLoad";
            this.btnLoad.Size = new System.Drawing.Size(96, 23);
            this.btnLoad.TabIndex = 14;
            this.btnLoad.Text = "Load";
            this.btnLoad.Click += new System.EventHandler(this.btnLoad_Click);
            // 
            // btnSave
            // 
            this.btnSave.Location = new System.Drawing.Point(8, 296);
            this.btnSave.Name = "btnSave";
            this.btnSave.Size = new System.Drawing.Size(96, 23);
            this.btnSave.TabIndex = 13;
            this.btnSave.Text = "Save";
            this.btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // dlgSave
            // 
            this.dlgSave.DefaultExt = "xml";
            // 
            // RunTargetForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(610, 328);
            this.Controls.Add(this.btnLoad);
            this.Controls.Add(this.btnSave);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.gbRules);
            this.Controls.Add(this.gbTarget);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "RunTargetForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Run Target Settings";
            this.Closing += new System.ComponentModel.CancelEventHandler(this.RunTargetForm_Closing);
            this.gbTarget.ResumeLayout(false);
            this.gbRules.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion

        #region Properties

        private int logLevel = 0;
        public int LogLevel {
            get { return logLevel; }
        }

        public string[] IncludeItems {
            get { 
                ArrayList inc = new ArrayList();
                foreach(string s in tbRules.Lines) {
                    string rule = s.Trim();
                    if (rule.Length > 0 && rule[0] == '+')
                        inc.Add(rule.Substring(1));
                }
                return (string[])inc.ToArray(typeof(string));
            }
        }

        public string[] ExcludeItems {
            get { 
                ArrayList inc = new ArrayList();
                foreach(string s in tbRules.Lines) {
                    string rule = s.Trim();
                    if (rule.Length > 0 && rule[0] == '-')
                        inc.Add(rule.Substring(1));
                }
                return (string[])inc.ToArray(typeof(string));
            }
        }

        public string TargetPath {
            get { return tbPath.Text; }
        }

        public string TargetWorkingDir {
            get { return tbWorkingDir.Text; }
        }

        public string TargetArgs {
            get { return tbArgs.Text; }
        }

        private string outputFile = null;

        public string FileNameForReport {
            get { return outputFile; }
        }

        public bool OutputToFile {
            get { return FileNameForReport != null; }
        }

        #endregion //Properties

        private void btnBrowseTarget_Click(object sender, System.EventArgs e) {
            dlgOpen.Filter = "Executable files (*.exe)|*.exe";
            if(dlgOpen.ShowDialog(this) == DialogResult.OK) {
                tbPath.Text = Path.GetFullPath(dlgOpen.FileName);
                tbWorkingDir.Text = Path.GetDirectoryName(dlgOpen.FileName);
            }
        }

        private void btnBrowseDir_Click(object sender, System.EventArgs e) {
            if(dlgBrowse.ShowDialog(this) == DialogResult.OK) {
                tbWorkingDir.Text = dlgBrowse.SelectedPath;
            }
        }

        private void ShowInformation(string error) {
            MessageBox.Show(this, error, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void RunTargetForm_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
            if (DialogResult != DialogResult.OK)
                return;

            e.Cancel = true;

            if (!File.Exists(TargetPath)) {
                ShowInformation("Set target executable file");
                return;
            }

            if (IncludeItems.Length == 0) {
                ShowInformation("You should include at least one rule in report.");
                return;
            }

            e.Cancel = false;
        }

        private void btnSave_Click(object sender, System.EventArgs e) {
            WorkSettings settings = new WorkSettings();
            settings.TargetPath = TargetPath;
            settings.TargetArgs = TargetArgs;
            settings.TargetWorkingDir = TargetWorkingDir;
            settings.FileNameForReport = FileNameForReport;

            settings.IncludeRules( IncludeItems );
            settings.ExcludeRules( ExcludeItems );

            dlgSave.Filter = "Settings files (*.xml)|*.xml";
            if (dlgSave.ShowDialog(this) == DialogResult.OK) {
                try {
                    settings.GenerateSettingsFileName = dlgSave.FileName;
                    settings.GenerateSettingsFile();
                    ShowInformation("Settings were saved!");
                } catch(Exception ex) {
                    ShowInformation("Cannot save settings (" + ex.Message + ")");
                }
            }
        }

        private void btnLoad_Click(object sender, System.EventArgs e) {
            dlgOpen.Filter = "Settings files (*.xml)|*.xml";
            if (dlgOpen.ShowDialog(this) != DialogResult.OK) {
                return;
            }

            WorkSettings settings = new WorkSettings();

            settings.SettingsFile = dlgOpen.FileName;
            try {
                settings.ReadSettingsFile();
            } catch(Exception ex) {
                ShowInformation("Cannot load settings (" + ex.Message + ")");
                return;
            }

            tbPath.Text = settings.TargetPath;
            tbWorkingDir.Text = settings.TargetWorkingDir;
            tbArgs.Text = settings.TargetArgs;

            tbRules.Text =  string.Empty;

            foreach(string s in settings.IncludeItems)  {
                if (tbRules.Text.Length > 0) tbRules.Text = tbRules.Text + "\r\n";
                tbRules.Text = tbRules.Text + "+" + s;
            }
            foreach(string s in settings.ExcludeItems)  {
                if (tbRules.Text.Length > 0) tbRules.Text = tbRules.Text + "\r\n";
                tbRules.Text = tbRules.Text + "-" + s;
            }
        }
	}
}
