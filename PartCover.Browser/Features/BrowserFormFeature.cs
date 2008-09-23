using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api;

namespace PartCover.Browser.Features
{
    internal class BrowserFormFeature : IFeature
    {
        private MainForm mainForm;

        public MainForm MainForm
        {
            get { return mainForm; }
        }

        public void attach(IServiceContainer container)
        {
            mainForm = new MainForm();
            mainForm.ServiceContainer = container;

            container.registerService(mainForm);
        }

        public void detach(IServiceContainer container)
        {
            container.unregisterService(mainForm);

            mainForm.ServiceContainer = null;
            mainForm = null;
        }

        public void build(IServiceContainer container) { }

        public void destroy(IServiceContainer container) { }
    }
}
