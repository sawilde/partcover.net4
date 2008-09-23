using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace PartCover.Browser.Api
{
    public class ReportView : Form
    {
        private IServiceContainer container;

        public IServiceContainer Services { get { return container; } }

        public virtual void attach(IServiceContainer container)
        {
            this.container = container;
        }

        public virtual void detach(IServiceContainer container)
        {
            this.container = null;
        }
    }
}
