using System;
using System.IO;
using System.Threading;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using PartCover.Framework.Walkers;

namespace PartCover.Coverage.Browser
{
	public class GenerationForm : System.Windows.Forms.Form
	{
        private System.Windows.Forms.Label label1;
		private System.ComponentModel.Container components = null;

		public GenerationForm()
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
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(240, 54);
            this.label1.TabIndex = 1;
            this.label1.Text = "Generate report";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // GenerationForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(240, 54);
            this.ControlBox = false;
            this.Controls.Add(this.label1);
            this.Name = "GenerationForm";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "GenerationForm";
            this.Load += new System.EventHandler(this.GenerationForm_Load);
            this.ResumeLayout(false);

        }
		#endregion

        bool success = false;
        public bool Success {
            get { return success; }
        }

        private delegate string GetStringDelegate();

        string targetPath;
        string targetDir;
        string targetArgs;
        public string TargetPath {
            set { targetPath = value; }
            get { return GetTargetPath(); }
        }

        private string GetTargetPath() {
            if (InvokeRequired)
                return (string) Invoke(new GetStringDelegate(GetTargetPath));
            return targetPath;
        }

        public string TargetDir {
            set { targetDir = value; }
            get { return GetTargetDir(); }
        }

        private string GetTargetDir() {
            if (InvokeRequired)
                return (string) Invoke(new GetStringDelegate(GetTargetDir));
            return targetDir;
        }

        public string TargetArgs {
            set { targetArgs = value; }
            get { return GetTargetArgs(); }
        }

        private string GetTargetArgs() {
            if (InvokeRequired)
                return (string) Invoke(new GetStringDelegate(GetTargetArgs));
            return targetArgs;
        }

        string[] include = new string[0];
        string[] exclude = new string[0];
        public string[] IncludeRules { 
            set { include = value; } 
            get { return GetIncludeString(); } 
        }
        public string[] ExcludeRules { 
            set { exclude = value; } 
            get { return GetExcludeString(); } 
        }

        CoverageReport report;
        public CoverageReport Report {
            set { SetReport(value); }
            get { return report; }
        }

        private delegate void SetReportDelegate(CoverageReport re);
        private void SetReport(CoverageReport re) {
            if (InvokeRequired)
                Invoke(new SetReportDelegate(SetReport), new object[] { re });
            else 
                report = re;
        }

        private delegate string[] GetStringArrayDelegate();
        private string[] GetIncludeString() {
            if (InvokeRequired)
                return (string[])Invoke(new GetStringArrayDelegate(GetIncludeString));
            else return include;
        }

        private string[] GetExcludeString() {
            if (InvokeRequired)
                return (string[])Invoke(new GetStringArrayDelegate(GetExcludeString));
            else return exclude;
        }

        private delegate void EndGenerationDelegate(bool status);
        private void EndGeneration(bool status) {
            if (InvokeRequired) {
                Invoke(new EndGenerationDelegate(EndGeneration), new object[] { status });
            } else {
                this.BringToFront();
                if (!status)
                    MessageBox.Show(this, "Report wasn't generated");
                success = status;
                this.Close();
            }
        }

        private void ReportThread() {
            try {
                Framework.Connector connector = new Framework.Connector();

                foreach(string s in IncludeRules) connector.IncludeItem(s);
                foreach(string s in ExcludeRules) connector.ExcludeItem(s);

                connector.StartTarget(TargetPath, TargetDir, TargetArgs, false, false);

                Report = connector.BlockWalker.Report;

                EndGeneration(true);
            } 
            catch 
            {
                EndGeneration(false);
            }
        }

        private void GenerationForm_Load(object sender, System.EventArgs e) {
            System.Threading.Thread thread = new Thread(new ThreadStart(ReportThread));
            thread.Start();
        }

        private void btnClose_Click(object sender, System.EventArgs e) {
            this.Close();
        }
	}
}
