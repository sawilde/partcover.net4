using System;
using PartCover.Browser.Helpers;
using PartCover.Framework;
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

        public void putDate() {
            if (InvokeRequired)
            {
                Invoke(new MethodInvoker(putDate));
                return;
            }

            tbText.AppendText(DateTime.Now.ToString() + " ");
        }

        public void putText(string message) {
            if (InvokeRequired)
            {
                Invoke(new StringSetter(putText), message);
                return;
            }

            tbText.AppendText(message);
        }

        public void setMessage(string value)
        {
            putDate();
            putText(value + Environment.NewLine);
        }

        public void setPercent(float value)
        {
        }

        public float getPercent()
        {
            return 0;
        }

        public void queueBegin(string message)
        {
            putDate();
            putText(message);
        }

        public void queuePush(string message)
        {
            putText(message);
        }

        public void queueEnd(string message)
        {
            putText(message + Environment.NewLine);
        }
    }
}