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
            this.tcPanes = new System.Windows.Forms.TabControl();
            this.tpMessages = new System.Windows.Forms.TabPage();
            this.lbItems = new System.Windows.Forms.ListBox();
            this.tpLog = new System.Windows.Forms.TabPage();
            this.tbLog = new System.Windows.Forms.TextBox();
            this.lbExitCode = new System.Windows.Forms.Label();
            this.tcPanes.SuspendLayout();
            this.tpMessages.SuspendLayout();
            this.tpLog.SuspendLayout();
            this.SuspendLayout();
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
            this.tcPanes.Size = new System.Drawing.Size(538, 444);
            this.tcPanes.TabIndex = 2;
            // 
            // tpMessages
            // 
            this.tpMessages.Controls.Add(this.lbItems);
            this.tpMessages.Location = new System.Drawing.Point(4, 22);
            this.tpMessages.Name = "tpMessages";
            this.tpMessages.Padding = new System.Windows.Forms.Padding(3);
            this.tpMessages.Size = new System.Drawing.Size(530, 418);
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
            this.lbItems.Size = new System.Drawing.Size(524, 407);
            this.lbItems.TabIndex = 0;
            // 
            // tpLog
            // 
            this.tpLog.Controls.Add(this.tbLog);
            this.tpLog.Location = new System.Drawing.Point(4, 22);
            this.tpLog.Name = "tpLog";
            this.tpLog.Padding = new System.Windows.Forms.Padding(3);
            this.tpLog.Size = new System.Drawing.Size(530, 418);
            this.tpLog.TabIndex = 1;
            this.tpLog.Text = "Log File";
            this.tpLog.UseVisualStyleBackColor = true;
            // 
            // tbLog
            // 
            this.tbLog.BackColor = System.Drawing.Color.Black;
            this.tbLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbLog.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.tbLog.ForeColor = System.Drawing.Color.Lime;
            this.tbLog.Location = new System.Drawing.Point(3, 3);
            this.tbLog.MaxLength = 1024000000;
            this.tbLog.Multiline = true;
            this.tbLog.Name = "tbLog";
            this.tbLog.ReadOnly = true;
            this.tbLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.tbLog.Size = new System.Drawing.Size(524, 412);
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
        private System.Windows.Forms.TabControl tcPanes;
    }
}