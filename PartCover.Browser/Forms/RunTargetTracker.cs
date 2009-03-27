using System;
using System.Windows.Forms;
using PartCover.Browser.Helpers;
using PartCover.Browser.Stuff;

namespace PartCover.Browser.Forms
{
    internal partial class RunTargetTracker
        : AsyncUserProcessForm
        , ITargetProgressTracker
    {
        DateTime startedAt;

        public RunTargetTracker()
        {
            InitializeComponent();
            tssMessage.Text = string.Empty;
            tssTime.Text = string.Empty;
        }

        private void putText(string message, bool clearBeforePut)
        {
            if (clearBeforePut)
            {
                tssMessage.Text = message;
            }
            else
            {
                tssMessage.Text = tssMessage.Text + message;
            }
        }

        private delegate void PutStringDelegate(string message);

        public void ShowStatus(string message)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(ShowStatus), message);
                return;
            }

            putText(message, true);
        }

        public void ShowLogMessage(string data)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(ShowLogMessage), data);
                return;
            }

            tbLog.AppendText(data + Environment.NewLine);
        }

        protected override void BeforeStart()
        {
            startedAt = DateTime.Now;
            timer.Enabled = true;
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(putTime));

            }
            else
            {
                putTime();
            }
        }

        private void putTime()
        {
            var span = DateTime.Now - startedAt;

            var messsage = string.Empty;
            if (span.Minutes > 0)
            {
                messsage += string.Format("{0,2} min ", span.Minutes);
            }
            if (span.Seconds > 0)
            {
                messsage += string.Format("{0,2} sec", span.Seconds);
            }

            if (tssTime.Text != messsage)
            {
                tssTime.Text = messsage;
            }
        }

    }
}