using PartCover.Browser.Api;

namespace PartCover.Browser.Features.Controls
{
    partial class ItemDetails 
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
            this.rtbNodeProps = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // rtbNodeProps
            // 
            this.rtbNodeProps.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.rtbNodeProps.Dock = System.Windows.Forms.DockStyle.Fill;
            this.rtbNodeProps.Location = new System.Drawing.Point(0, 0);
            this.rtbNodeProps.Name = "rtbNodeProps";
            this.rtbNodeProps.ReadOnly = true;
            this.rtbNodeProps.Size = new System.Drawing.Size(243, 200);
            this.rtbNodeProps.TabIndex = 5;
            this.rtbNodeProps.Text = "";
            this.rtbNodeProps.WordWrap = false;
            // 
            // ItemDetails
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.rtbNodeProps);
            this.Name = "ItemDetails";
            this.Size = new System.Drawing.Size(243, 200);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.RichTextBox rtbNodeProps;
    }
}
