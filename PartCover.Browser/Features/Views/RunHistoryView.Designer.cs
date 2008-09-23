namespace PartCover.Browser.Features.Views
{
    partial class RunHistoryView
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
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
            System.Windows.Forms.Button btnSave;
            System.Windows.Forms.Button btnLoad;
            this.tcPanes = new System.Windows.Forms.TabControl();
            this.tpMessages = new System.Windows.Forms.TabPage();
            this.lbItems = new System.Windows.Forms.ListBox();
            this.tpLog = new System.Windows.Forms.TabPage();
            this.tbLog = new System.Windows.Forms.TextBox();
            this.lbExitCode = new System.Windows.Forms.Label();
            this.ofd = new System.Windows.Forms.OpenFileDialog();
            this.sfd = new System.Windows.Forms.SaveFileDialog();
            btnSave = new System.Windows.Forms.Button();
            btnLoad = new System.Windows.Forms.Button();
            this.tcPanes.SuspendLayout();
            this.tpMessages.SuspendLayout();
            this.tpLog.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnSave
            // 
            btnSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            btnSave.Location = new System.Drawing.Point(19, 446);
            btnSave.Name = "btnSave";
            btnSave.Size = new System.Drawing.Size(75, 23);
            btnSave.TabIndex = 3;
            btnSave.Text = "&Save ...";
            btnSave.Click += new System.EventHandler(this.btnSave_Click);
            // 
            // btnLoad
            // 
            btnLoad.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            btnLoad.Location = new System.Drawing.Point(100, 446);
            btnLoad.Name = "btnLoad";
            btnLoad.Size = new System.Drawing.Size(75, 23);
            btnLoad.TabIndex = 4;
            btnLoad.Text = "&Load ...";
            btnLoad.Click += new System.EventHandler(this.btnLoad_Click);
            // 
            // tcPanes
            // 
            this.tcPanes.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tcPanes.Controls.Add(this.tpMessages);
            this.tcPanes.Controls.Add(this.tpLog);
            this.tcPanes.Location = new System.Drawing.Point(12, 25);
            this.tcPanes.Name = "tcPanes";
            this.tcPanes.SelectedIndex = 0;
            this.tcPanes.Size = new System.Drawing.Size(538, 415);
            this.tcPanes.TabIndex = 2;
            // 
            // tpMessages
            // 
            this.tpMessages.Controls.Add(this.lbItems);
            this.tpMessages.Location = new System.Drawing.Point(4, 22);
            this.tpMessages.Name = "tpMessages";
            this.tpMessages.Padding = new System.Windows.Forms.Padding(3);
            this.tpMessages.Size = new System.Drawing.Size(530, 389);
            this.tpMessages.TabIndex = 0;
            this.tpMessages.Text = "Run Tracker";
            this.tpMessages.UseVisualStyleBackColor = true;
            // 
            // lbItems
            // 
            this.lbItems.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lbItems.FormattingEnabled = true;
            this.lbItems.Location = new System.Drawing.Point(3, 3);
            this.lbItems.Name = "lbItems";
            this.lbItems.Size = new System.Drawing.Size(524, 381);
            this.lbItems.TabIndex = 0;
            // 
            // tpLog
            // 
            this.tpLog.Controls.Add(this.tbLog);
            this.tpLog.Location = new System.Drawing.Point(4, 22);
            this.tpLog.Name = "tpLog";
            this.tpLog.Padding = new System.Windows.Forms.Padding(3);
            this.tpLog.Size = new System.Drawing.Size(530, 389);
            this.tpLog.TabIndex = 1;
            this.tpLog.Text = "Log File";
            this.tpLog.UseVisualStyleBackColor = true;
            // 
            // tbLog
            // 
            this.tbLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbLog.Location = new System.Drawing.Point(3, 3);
            this.tbLog.MaxLength = 1024000000;
            this.tbLog.Multiline = true;
            this.tbLog.Name = "tbLog";
            this.tbLog.ReadOnly = true;
            this.tbLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbLog.Size = new System.Drawing.Size(524, 383);
            this.tbLog.TabIndex = 0;
            this.tbLog.WordWrap = false;
            // 
            // lbExitCode
            // 
            this.lbExitCode.AutoSize = true;
            this.lbExitCode.Location = new System.Drawing.Point(12, 9);
            this.lbExitCode.Name = "lbExitCode";
            this.lbExitCode.Size = new System.Drawing.Size(55, 13);
            this.lbExitCode.TabIndex = 1;
            this.lbExitCode.Text = "Exit Code:";
            // 
            // RunHistoryView
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(562, 481);
            this.Controls.Add(btnLoad);
            this.Controls.Add(btnSave);
            this.Controls.Add(this.tcPanes);
            this.Controls.Add(this.lbExitCode);
            this.Name = "RunHistoryView";
            this.Text = "Messages from Run Tracker";
            this.tcPanes.ResumeLayout(false);
            this.tpMessages.ResumeLayout(false);
            this.tpLog.ResumeLayout(false);
            this.tpLog.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox lbItems;
        private System.Windows.Forms.Label lbExitCode;
        private System.Windows.Forms.TabPage tpMessages;
        private System.Windows.Forms.TabPage tpLog;
        private System.Windows.Forms.TextBox tbLog;
        private System.Windows.Forms.OpenFileDialog ofd;
        private System.Windows.Forms.SaveFileDialog sfd;
        private System.Windows.Forms.TabControl tcPanes;
    }
}