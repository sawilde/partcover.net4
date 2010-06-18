namespace PartCover.Browser.Dialogs
{
    partial class SettingsForm
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
            System.Windows.Forms.GroupBox groupBox1;
            System.Windows.Forms.GroupBox groupBox2;
            System.Windows.Forms.GroupBox groupBox3;
            this.btnFont = new System.Windows.Forms.Button();
            this.btnDocumentFore = new System.Windows.Forms.Button();
            this.btnDocumentBack = new System.Windows.Forms.Button();
            this.lbExample = new System.Windows.Forms.Label();
            this.btnGoodBack = new System.Windows.Forms.Button();
            this.lbGood = new System.Windows.Forms.Label();
            this.btnBadBack = new System.Windows.Forms.Button();
            this.lbBad = new System.Windows.Forms.Label();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.cbAllFile = new System.Windows.Forms.CheckBox();
            this.dlgFont = new System.Windows.Forms.FontDialog();
            this.dlgColor = new System.Windows.Forms.ColorDialog();
            this.btnReset = new System.Windows.Forms.Button();
            groupBox1 = new System.Windows.Forms.GroupBox();
            groupBox2 = new System.Windows.Forms.GroupBox();
            groupBox3 = new System.Windows.Forms.GroupBox();
            groupBox1.SuspendLayout();
            groupBox2.SuspendLayout();
            groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(this.btnFont);
            groupBox1.Controls.Add(this.btnDocumentFore);
            groupBox1.Controls.Add(this.btnDocumentBack);
            groupBox1.Controls.Add(this.lbExample);
            groupBox1.Location = new System.Drawing.Point(12, 12);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(387, 86);
            groupBox1.TabIndex = 0;
            groupBox1.TabStop = false;
            groupBox1.Text = "File Document Style";
            // 
            // btnFont
            // 
            this.btnFont.Location = new System.Drawing.Point(156, 25);
            this.btnFont.Name = "btnFont";
            this.btnFont.Size = new System.Drawing.Size(74, 52);
            this.btnFont.TabIndex = 1;
            this.btnFont.Text = "Change\r\nFont";
            this.btnFont.UseVisualStyleBackColor = true;
            this.btnFont.Click += new System.EventHandler(this.btnFont_Click);
            // 
            // btnDocumentFore
            // 
            this.btnDocumentFore.Location = new System.Drawing.Point(236, 54);
            this.btnDocumentFore.Name = "btnDocumentFore";
            this.btnDocumentFore.Size = new System.Drawing.Size(146, 23);
            this.btnDocumentFore.TabIndex = 3;
            this.btnDocumentFore.Text = "Change foreground";
            this.btnDocumentFore.UseVisualStyleBackColor = true;
            this.btnDocumentFore.Click += new System.EventHandler(this.btnDocumentFore_Click);
            // 
            // btnDocumentBack
            // 
            this.btnDocumentBack.Location = new System.Drawing.Point(236, 25);
            this.btnDocumentBack.Name = "btnDocumentBack";
            this.btnDocumentBack.Size = new System.Drawing.Size(146, 23);
            this.btnDocumentBack.TabIndex = 2;
            this.btnDocumentBack.Text = "Change background";
            this.btnDocumentBack.UseVisualStyleBackColor = true;
            this.btnDocumentBack.Click += new System.EventHandler(this.btnDocumentBack_Click);
            // 
            // lbExample
            // 
            this.lbExample.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lbExample.Location = new System.Drawing.Point(10, 25);
            this.lbExample.Name = "lbExample";
            this.lbExample.Size = new System.Drawing.Size(140, 52);
            this.lbExample.TabIndex = 0;
            this.lbExample.Text = "Gubka Bob! :)";
            this.lbExample.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(this.btnGoodBack);
            groupBox2.Controls.Add(this.lbGood);
            groupBox2.Location = new System.Drawing.Point(13, 115);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new System.Drawing.Size(190, 84);
            groupBox2.TabIndex = 1;
            groupBox2.TabStop = false;
            groupBox2.Text = "Good Block Style";
            // 
            // btnGoodBack
            // 
            this.btnGoodBack.Location = new System.Drawing.Point(9, 55);
            this.btnGoodBack.Name = "btnGoodBack";
            this.btnGoodBack.Size = new System.Drawing.Size(175, 23);
            this.btnGoodBack.TabIndex = 1;
            this.btnGoodBack.Text = "Change background";
            this.btnGoodBack.UseVisualStyleBackColor = true;
            this.btnGoodBack.Click += new System.EventHandler(this.btnGoodBack_Click);
            // 
            // lbGood
            // 
            this.lbGood.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lbGood.Location = new System.Drawing.Point(9, 16);
            this.lbGood.Name = "lbGood";
            this.lbGood.Size = new System.Drawing.Size(175, 30);
            this.lbGood.TabIndex = 0;
            this.lbGood.Text = "Covered";
            this.lbGood.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // groupBox3
            // 
            groupBox3.Controls.Add(this.btnBadBack);
            groupBox3.Controls.Add(this.lbBad);
            groupBox3.Location = new System.Drawing.Point(209, 114);
            groupBox3.Name = "groupBox3";
            groupBox3.Size = new System.Drawing.Size(190, 84);
            groupBox3.TabIndex = 2;
            groupBox3.TabStop = false;
            groupBox3.Text = "Bad Block Style";
            // 
            // btnBadBack
            // 
            this.btnBadBack.Location = new System.Drawing.Point(9, 55);
            this.btnBadBack.Name = "btnBadBack";
            this.btnBadBack.Size = new System.Drawing.Size(175, 23);
            this.btnBadBack.TabIndex = 1;
            this.btnBadBack.Text = "Change background";
            this.btnBadBack.UseVisualStyleBackColor = true;
            this.btnBadBack.Click += new System.EventHandler(this.btnBadBack_Click);
            // 
            // lbBad
            // 
            this.lbBad.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lbBad.Location = new System.Drawing.Point(9, 16);
            this.lbBad.Name = "lbBad";
            this.lbBad.Size = new System.Drawing.Size(175, 30);
            this.lbBad.TabIndex = 0;
            this.lbBad.Text = "Uncovered";
            this.lbBad.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnOk
            // 
            this.btnOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnOk.Location = new System.Drawing.Point(278, 308);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 4;
            this.btnOk.Text = "Apply";
            this.btnOk.UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(364, 308);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // cbAllFile
            // 
            this.cbAllFile.AutoSize = true;
            this.cbAllFile.Location = new System.Drawing.Point(22, 222);
            this.cbAllFile.Name = "cbAllFile";
            this.cbAllFile.Size = new System.Drawing.Size(284, 17);
            this.cbAllFile.TabIndex = 3;
            this.cbAllFile.Text = "Highlight all blocks in file (not only from current method)";
            this.cbAllFile.UseVisualStyleBackColor = true;
            this.cbAllFile.CheckedChanged += new System.EventHandler(this.cbAllFile_CheckedChanged);
            // 
            // btnReset
            // 
            this.btnReset.Location = new System.Drawing.Point(12, 308);
            this.btnReset.Name = "btnReset";
            this.btnReset.Size = new System.Drawing.Size(125, 23);
            this.btnReset.TabIndex = 6;
            this.btnReset.Text = "Reset to defaults";
            this.btnReset.UseVisualStyleBackColor = true;
            this.btnReset.Click += new System.EventHandler(this.btnReset_Click);
            // 
            // SettingsForm
            // 
            this.AcceptButton = this.btnOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(451, 343);
            this.Controls.Add(this.btnReset);
            this.Controls.Add(this.cbAllFile);
            this.Controls.Add(groupBox3);
            this.Controls.Add(groupBox2);
            this.Controls.Add(groupBox1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SettingsForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Settings";
            groupBox1.ResumeLayout(false);
            groupBox2.ResumeLayout(false);
            groupBox3.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Label lbExample;
        private System.Windows.Forms.Button btnDocumentFore;
        private System.Windows.Forms.Button btnDocumentBack;
        private System.Windows.Forms.Button btnFont;
        private System.Windows.Forms.Label lbGood;
        private System.Windows.Forms.Button btnGoodBack;
        private System.Windows.Forms.Button btnBadBack;
        private System.Windows.Forms.Label lbBad;
        private System.Windows.Forms.CheckBox cbAllFile;
        private System.Windows.Forms.FontDialog dlgFont;
        private System.Windows.Forms.ColorDialog dlgColor;
        private System.Windows.Forms.Button btnReset;
    }
}