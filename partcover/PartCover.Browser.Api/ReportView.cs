using System.Windows.Forms;

namespace PartCover.Browser.Api
{
    public class ReportView : Form
    {
        public IServiceContainer Services { get; private set; }

        public virtual void attach(IServiceContainer container, IProgressTracker tracker)
        {
            Services = container;
        }

        public virtual void detach(IServiceContainer container, IProgressTracker tracker)
        {
            Services = null;
        }
    }
}
