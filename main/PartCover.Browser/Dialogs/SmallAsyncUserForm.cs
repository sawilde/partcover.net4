using System;
using PartCover.Browser.Helpers;
using PartCover.Browser.Api;
using System.Windows.Forms;

namespace PartCover.Browser.Dialogs
{
    internal partial class SmallAsyncUserForm : AsyncUserProcessForm, IProgressTracker
    {
        public SmallAsyncUserForm()
        {
            InitializeComponent();
        }

        private delegate void StringSetter(string value);

        public void putDate()
        {
            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(putDate));
                return;
            }

            tbText.AppendText(DateTime.Now + " ");
        }

        public void putText(string message)
        {
            if (InvokeRequired)
            {
                Invoke(new StringSetter(putText), message);
                return;
            }

            tbText.AppendText(message);
        }

        public void AppendMessage(string value)
        {
            putDate();
            putText(value + Environment.NewLine);
        }

        public float Percent
        {
            set { }
            get { return 0; }
        }

        public void QueueBegin(string message)
        {
            putDate();
            putText(message);
        }

        public void QueuePush(string message)
        {
            putText(message);
        }

        public void QueueEnd(string message)
        {
            putText(message + Environment.NewLine);
        }
    }
}