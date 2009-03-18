using System;
using System.Windows.Forms;
using PartCover.Browser.Stuff;
using PartCover.Browser.Helpers;
using PartCover.Framework.Walkers;
using System.Globalization;

namespace PartCover.Browser.Forms
{
    internal partial class RunTargetTracker : AsyncUserProcessForm, IRunTargetProgressTracker
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
        public void AppendMessage(string message)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(AppendMessage), message);
                return;
            }

            putText(message, true);
        }

        public void QueueBegin(string message)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(QueueBegin), message);
                return;
            }

            putText(message, true);
        }

        public void QueuePush(string message)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(QueuePush), message);
                return;
            }
            putText(message, false);
        }

        public void QueueEnd(string message)
        {
            if (!IsHandleCreated) return;

            if (InvokeRequired)
            {
                Invoke(new PutStringDelegate(QueueEnd), message);
                return;
            }
            putText(message, false);
        }

        public float Percent
        {
            set { }
            get { return 0; }
        }

        protected override void BeforeStart()
        {
            startedAt = DateTime.Now;
            timer.Enabled = true;
        }

        public void add(CoverageReport.RunLogMessage runLogMessage)
        {
            putLogEntry(runLogMessage);
        }

        delegate void PutLogEntryDelegate(CoverageReport.RunLogMessage item);
        private void putLogEntry(CoverageReport.RunLogMessage item)
        {
            if (InvokeRequired)
            {
                Invoke(new PutLogEntryDelegate(putLogEntry), item);
                return;
            }

            tbLog.AppendText(string.Format(CultureInfo.CurrentCulture,
                "[{0,6}][{1,6}]{2}{3}",
                item.ThreadId, item.MsOffset, item.Message, Environment.NewLine));

        }

        public void add(CoverageReport.RunHistoryMessage runHistoryMessage)
        {
            AppendMessage(runHistoryMessage.Message);
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