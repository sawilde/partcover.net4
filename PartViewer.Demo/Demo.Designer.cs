namespace PartViewer.Demo
{
    partial class Demo
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Demo));
            this.msMain = new System.Windows.Forms.MenuStrip();
            this.tsmLoadFile = new System.Windows.Forms.ToolStripMenuItem();
            this.stylizeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.dLoadFile = new System.Windows.Forms.OpenFileDialog();
            this.tbPages = new System.Windows.Forms.TabControl();
            this.offToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.msMain.SuspendLayout();
            this.SuspendLayout();
            // 
            // msMain
            // 
            this.msMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmLoadFile,
            this.stylizeToolStripMenuItem});
            this.msMain.Location = new System.Drawing.Point(0, 0);
            this.msMain.Name = "msMain";
            this.msMain.Size = new System.Drawing.Size(455, 24);
            this.msMain.TabIndex = 0;
            // 
            // tsmLoadFile
            // 
            this.tsmLoadFile.Image = ((System.Drawing.Image)(resources.GetObject("tsmLoadFile.Image")));
            this.tsmLoadFile.Name = "tsmLoadFile";
            this.tsmLoadFile.Size = new System.Drawing.Size(58, 20);
            this.tsmLoadFile.Text = "&Load";
            this.tsmLoadFile.Click += new System.EventHandler(this.tsmLoadFile_Click);
            // 
            // stylizeToolStripMenuItem
            // 
            this.stylizeToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cToolStripMenuItem,
            this.cToolStripMenuItem1,
            this.toolStripMenuItem1,
            this.offToolStripMenuItem});
            this.stylizeToolStripMenuItem.Name = "stylizeToolStripMenuItem";
            this.stylizeToolStripMenuItem.Size = new System.Drawing.Size(50, 20);
            this.stylizeToolStripMenuItem.Text = "Stylize";
            // 
            // cToolStripMenuItem
            // 
            this.cToolStripMenuItem.Name = "cToolStripMenuItem";
            this.cToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.cToolStripMenuItem.Text = "C#";
            this.cToolStripMenuItem.Click += new System.EventHandler(this.cToolStripMenuItem_Click);
            // 
            // cToolStripMenuItem1
            // 
            this.cToolStripMenuItem1.Name = "cToolStripMenuItem1";
            this.cToolStripMenuItem1.Size = new System.Drawing.Size(152, 22);
            this.cToolStripMenuItem1.Text = "C++";
            this.cToolStripMenuItem1.Click += new System.EventHandler(this.cToolStripMenuItem1_Click);
            // 
            // dLoadFile
            // 
            this.dLoadFile.AddExtension = false;
            this.dLoadFile.RestoreDirectory = true;
            this.dLoadFile.ShowReadOnly = true;
            // 
            // tbPages
            // 
            this.tbPages.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tbPages.Location = new System.Drawing.Point(0, 24);
            this.tbPages.Name = "tbPages";
            this.tbPages.SelectedIndex = 0;
            this.tbPages.Size = new System.Drawing.Size(455, 421);
            this.tbPages.TabIndex = 1;
            // 
            // offToolStripMenuItem
            // 
            this.offToolStripMenuItem.Name = "offToolStripMenuItem";
            this.offToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.offToolStripMenuItem.Text = "Off";
            this.offToolStripMenuItem.Click += new System.EventHandler(this.offToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(149, 6);
            // 
            // Demo
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Info;
            this.ClientSize = new System.Drawing.Size(455, 445);
            this.Controls.Add(this.tbPages);
            this.Controls.Add(this.msMain);
            this.MainMenuStrip = this.msMain;
            this.Name = "Demo";
            this.Text = "PartViewer Control Demo";
            this.msMain.ResumeLayout(false);
            this.msMain.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip msMain;
        private System.Windows.Forms.ToolStripMenuItem tsmLoadFile;
        private System.Windows.Forms.OpenFileDialog dLoadFile;
        private System.Windows.Forms.TabControl tbPages;
        private System.Windows.Forms.ToolStripMenuItem stylizeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem cToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem cToolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem offToolStripMenuItem;
    }
}

