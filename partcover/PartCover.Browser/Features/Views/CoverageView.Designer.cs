namespace PartCover.Browser.Features.Views
{
    partial class CoverageView
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

        #region Component Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lbBlocks = new System.Windows.Forms.ListView();
            this.pnTabs = new System.Windows.Forms.TabControl();
            this.SuspendLayout();
            // 
            // lbBlocks
            // 
            this.lbBlocks.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lbBlocks.FullRowSelect = true;
            this.lbBlocks.GridLines = true;
            this.lbBlocks.Location = new System.Drawing.Point(12, 12);
            this.lbBlocks.Name = "lbBlocks";
            this.lbBlocks.Size = new System.Drawing.Size(568, 95);
            this.lbBlocks.TabIndex = 4;
            this.lbBlocks.UseCompatibleStateImageBehavior = false;
            this.lbBlocks.View = System.Windows.Forms.View.Details;
            this.lbBlocks.SelectedIndexChanged += new System.EventHandler(this.lbBlocks_SelectedIndexChanged);
            // 
            // pnTabs
            // 
            this.pnTabs.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.pnTabs.Location = new System.Drawing.Point(12, 113);
            this.pnTabs.Name = "pnTabs";
            this.pnTabs.SelectedIndex = 0;
            this.pnTabs.Size = new System.Drawing.Size(568, 267);
            this.pnTabs.TabIndex = 3;
            // 
            // CoverageView
            // 
            this.ClientSize = new System.Drawing.Size(592, 392);
            this.Controls.Add(this.lbBlocks);
            this.Controls.Add(this.pnTabs);
            this.Name = "CoverageView";
            this.Text = "Coverage Details";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lbBlocks;
        private System.Windows.Forms.TabControl pnTabs;
    }
}
