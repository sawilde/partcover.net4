using System;
using System.Windows.Forms;

namespace PartCover.Browser.Helpers
{
    internal class AsyncUserProcessForm : Form
    {
        internal MethodInvoker executable;

        private IAsyncResult ar;

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            if (DesignMode || executable == null)
                return;

            ar = executable.BeginInvoke(onEnd, null);
            if (ar.IsCompleted && ar.CompletedSynchronously)
                onEnd(ar);
        }

        private void onEnd(IAsyncResult value)
        {
            if (InvokeRequired)
            {
                Invoke(new AsyncCallback(onEnd), value);
                return;
            }

            executable.EndInvoke(value);
            Close();
        }

        private void InitializeComponent()
        {
            this.SuspendLayout();
            // 
            // AsyncUserProcessForm
            // 
            this.ClientSize = new System.Drawing.Size(406, 163);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "AsyncUserProcessForm";
            this.Text = "Tracker";
            this.ResumeLayout(false);

        }
    }
}
