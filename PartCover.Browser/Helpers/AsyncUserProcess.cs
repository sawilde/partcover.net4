using System;
using System.Threading;
using System.Windows.Forms;
using PartCover.Framework;
using PartCover.Browser.Api;
using PartCover.Browser.Dialogs;

namespace PartCover.Browser.Helpers
{
    internal class TinyAsyncUserProcess : AsyncUserProcess<SmallAsyncUserForm, IProgressTracker>
    {
        Action<IProgressTracker> method;
        public Action<IProgressTracker> Action {
            get { return method; }
            set { method = value; }
        }

        protected override void doWork()
        {
            Action(Tracker);
        }
    }

    internal abstract class AsyncUserProcess<T, D> where T : AsyncUserProcessForm, D, new()
    {
        private T progressForm;

        private Exception lastException;
        public Exception Exception
        {
            get { return lastException; }
        }

        private D progressTracker;
        protected D Tracker
        {
            get { return progressTracker; }
        }

        protected abstract void doWork();


        private void executeThread()
        {
            try
            {
                doWork();
            }
            catch (Exception e)
            {
                lastException = e;
            }
        }

        public void execute(IWin32Window owner)
        {
            using (progressForm = new T())
            {
                progressTracker = progressForm;

                progressForm.executable = executeThread;
                progressForm.ShowDialog(owner);

                progressTracker = default(D);
            }

            if (lastException != null)
                throw new ThreadInterruptedException(string.Empty, lastException);
        }
    }
}
