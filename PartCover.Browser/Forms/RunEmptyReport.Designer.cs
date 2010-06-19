namespace PartCover.Browser.Forms
{
    partial class RunEmptyReport
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
            this.btnClose = new System.Windows.Forms.Button();
            this.lbText = new System.Windows.Forms.Label();
            this.btnGotoInputs = new System.Windows.Forms.Button();
            this.btnViewSkipped = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnClose
            // 
            this.btnClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnClose.Location = new System.Drawing.Point(362, 61);
            this.btnClose.Name = "btnClose";
            this.btnClose.Size = new System.Drawing.Size(75, 23);
            this.btnClose.TabIndex = 0;
            this.btnClose.Text = "&Close";
            this.btnClose.UseVisualStyleBackColor = true;
            // 
            // lbText
            // 
            this.lbText.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.lbText.Location = new System.Drawing.Point(12, 9);
            this.lbText.Name = "lbText";
            this.lbText.Size = new System.Drawing.Size(425, 49);
            this.lbText.TabIndex = 1;
            this.lbText.Text = "Report has no items to show. Do you want to see items skipped by rules? \r\nOr mayb" +
                "e to correct input manually?";
            this.lbText.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnGotoInputs
            // 
            this.btnGotoInputs.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnGotoInputs.DialogResult = System.Windows.Forms.DialogResult.Retry;
            this.btnGotoInputs.Location = new System.Drawing.Point(131, 61);
            this.btnGotoInputs.Name = "btnGotoInputs";
            this.btnGotoInputs.Size = new System.Drawing.Size(110, 23);
            this.btnGotoInputs.TabIndex = 2;
            this.btnGotoInputs.Text = "&Go to inputs";
            this.btnGotoInputs.UseVisualStyleBackColor = true;
            // 
            // btnViewSkipped
            // 
            this.btnViewSkipped.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnViewSkipped.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnViewSkipped.Location = new System.Drawing.Point(15, 61);
            this.btnViewSkipped.Name = "btnViewSkipped";
            this.btnViewSkipped.Size = new System.Drawing.Size(110, 23);
            this.btnViewSkipped.TabIndex = 3;
            this.btnViewSkipped.Text = "&Show skipped";
            this.btnViewSkipped.UseVisualStyleBackColor = true;
            // 
            // RunEmptyReport
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(449, 96);
            this.ControlBox = false;
            this.Controls.Add(this.btnViewSkipped);
            this.Controls.Add(this.btnGotoInputs);
            this.Controls.Add(this.lbText);
            this.Controls.Add(this.btnClose);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "RunEmptyReport";
            this.Text = "Your report is empty";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnClose;
        private System.Windows.Forms.Label lbText;
        private System.Windows.Forms.Button btnGotoInputs;
        private System.Windows.Forms.Button btnViewSkipped;
    }
}