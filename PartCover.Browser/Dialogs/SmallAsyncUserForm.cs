using System;
using PartCover.Browser.Helpers;
using PartCover.Framework;

namespace PartCover.Browser.Dialogs
{
    internal partial class SmallAsyncUserForm : AsyncUserProcessForm, IProgressCallback {
        public SmallAsyncUserForm()
        {
            InitializeComponent();
        }

        private delegate void StringSetter(string value);

        public void writeStatus(string value)
        {
            if (InvokeRequired)
            {
                Invoke(new StringSetter(writeStatus), value);
                return;
            }

            tbText.AppendText(string.Format("{0}: {1}{2}", DateTime.Now, value, Environment.NewLine));
        }
    }
}