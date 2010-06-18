namespace PartCover.Browser.Forms
{
    partial class SkippedItemsReport
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
            this.lbItems = new System.Windows.Forms.ListBox();
            this.btnClose = new System.Windows.Forms.Button();
            this.btnAddRule = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // lbItems
            // 
            this.lbItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lbItems.FormattingEnabled = true;
            this.lbItems.Location = new System.Drawing.Point(12, 12);
            this.lbItems.Name = "lbItems";
            this.lbItems.Size = new System.Drawing.Size(460, 212);
            this.lbItems.TabIndex = 0;
            this.lbItems.SelectedIndexChanged += new System.EventHandler(this.lbItems_SelectedIndexChanged);
            // 
            // btnClose
            // 
            this.btnClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnClose.Location = new System.Drawing.Point(397, 238);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 1;
            this.btnClose.Text = "Close";
            // 
            // btnAddRule
            // 
            this.btnAddRule.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnAddRule.Enabled = false;
            this.btnAddRule.Location = new System.Drawing.Point(12, 238);
            this.btnAddRule.Name = "btnAddRule";
            this.btnAddRule.Size = new System.Drawing.Size(130, 23);
            this.btnAddRule.TabIndex = 2;
            this.btnAddRule.Text = "Add rule for this item";
            this.btnAddRule.Click += new System.EventHandler(this.btnAddRule_Click);
            // 
            // SkippedItemsReport
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 273);
            this.Controls.Add(this.btnAddRule);
            this.Controls.Add(this.btnClose);
            this.Controls.Add(this.lbItems);
            this.Name = "SkippedItemsReport";
            this.Text = "SkippedItemsReport";
            this.Shown += new System.EventHandler(this.SkippedItemsReport_Shown);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox lbItems;
        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Button btnAddRule;
    }
}