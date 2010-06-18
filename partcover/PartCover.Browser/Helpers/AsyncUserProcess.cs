using System;
using System.Threading;
using System.Windows.Forms;
using PartCover.Browser.Api;
using PartCover.Browser.Dialogs;

namespace PartCover.Browser.Helpers
{
    internal class TinyAsyncUserProcess : AsyncUserProcess<SmallAsyncUserForm, IProgressTracker>
    {
        public Action<IProgressTracker> Action { get; set; }

        protected override void doWork()
        {
            Action(Tracker);
        }
    }

    internal abstract class AsyncUserProcess<T, D> where T : AsyncUserProcessForm, D, new()
    {
        private T progressForm;

        public Exception Exception { get; private set; }

        protected D Tracker { get; private set; }

        protected abstract void doWork();

        private void executeThread()
        {
            try
            {
                doWork();
            }
            catch (Exception e)
            {
                Exception = e;
            }
        }

        public void Execute(IWin32Window owner)
        {
            using (progressForm = new T())
            {
                Tracker = progressForm;

                progressForm.executable = executeThread;
                progressForm.ShowDialog(owner);

                Tracker = default(D);
            }

            if (Exception != null)
                throw new ThreadInterruptedException(string.Empty, Exception);
        }
    }
}
