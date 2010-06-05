namespace PartCover.Browser.Forms
{
    partial class RunTargetTracker
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
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.StatusStrip statusStrip1;
            this.tssTime = new System.Windows.Forms.ToolStripStatusLabel();
            this.tssMessage = new System.Windows.Forms.ToolStripStatusLabel();
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.tbLog = new System.Windows.Forms.TextBox();
            statusStrip1 = new System.Windows.Forms.StatusStrip();
            statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tssTime,
            this.tssMessage});
            statusStrip1.Location = new System.Drawing.Point(0, 137);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new System.Drawing.Size(600, 22);
            statusStrip1.TabIndex = 0;
            statusStrip1.Text = "statusStrip1";
            // 
            // tssTime
            // 
            this.tssTime.AutoSize = false;
            this.tssTime.Name = "tssTime";
            this.tssTime.Size = new System.Drawing.Size(100, 17);
            this.tssTime.Text = "TIME";
            // 
            // tssMessage
            // 
            this.tssMessage.Name = "tssMessage";
            this.tssMessage.Size = new System.Drawing.Size(485, 17);
            this.tssMessage.Spring = true;
            this.tssMessage.Text = "TRACK MESSAGE";
            this.tssMessage.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // timer
            // 
            this.timer.Interval = 150;
            this.timer.Tick += new System.EventHandler(this.timer_Tick);
            // 
            // tbLog
            // 
            this.tbLog.BackColor = System.Drawing.Color.Black;
            this.tbLog.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbLog.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.tbLog.ForeColor = System.Drawing.Color.Lime;
            this.tbLog.Location = new System.Drawing.Point(0, 0);
            this.tbLog.MaxLength = 1024000000;
            this.tbLog.Multiline = true;
            this.tbLog.Name = "tbLog";
            this.tbLog.ReadOnly = true;
            this.tbLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.tbLog.Size = new System.Drawing.Size(600, 137);
            this.tbLog.TabIndex = 1;
            this.tbLog.WordWrap = false;
            // 
            // RunTargetTracker
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(600, 159);
            this.ControlBox = false;
            this.Controls.Add(this.tbLog);
            this.Controls.Add(statusStrip1);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "RunTargetTracker";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "RunTargetTracker";
            statusStrip1.ResumeLayout(false);
            statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ToolStripStatusLabel tssTime;
        private System.Windows.Forms.ToolStripStatusLabel tssMessage;
        private System.Windows.Forms.Timer timer;
        private System.Windows.Forms.TextBox tbLog;
    }
}