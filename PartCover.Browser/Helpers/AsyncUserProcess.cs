using System;
using System.Threading;
using System.Windows.Forms;
using PartCover.Framework;

namespace PartCover.Browser.Helpers
{
    internal abstract class AsyncUserProcess<T> where T : AsyncUserProcessForm, IProgressCallback, new()
    {
        private T progressForm;

        private Exception lastException;
        public Exception Exception
        {
            get { return lastException; }
        }

        private IProgressCallback progressCallback;
        protected IProgressCallback Callback
        {
            get { return progressCallback; }
        }

        private IProgressCallback listener;
        public IProgressCallback Listener
        {
            get { return listener; }
            set { listener = null; }
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
                progressCallback = progressForm;

                progressForm.executable = executeThread;
                progressForm.ShowDialog(owner);

                progressCallback = null;
            }

            if (lastException != null)
                throw new ThreadInterruptedException(string.Empty, lastException);
        }
    }
}
