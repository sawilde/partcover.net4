using PartCover.Browser.Api;
using PartCover.Browser.Forms;

namespace PartCover.Browser.Features
{
    internal class BrowserFormFeature : IFeature
    {
        public MainForm MainForm { get; private set; }

        public void Attach(IServiceContainer container)
        {
            MainForm = new MainForm {
                ServiceContainer = container
            };

            container.registerService(MainForm);
        }

        public void Detach(IServiceContainer container)
        {
            container.unregisterService(MainForm);

            MainForm.ServiceContainer = null;
            MainForm = null;
        }

        public void Build(IServiceContainer container) { }

        public void Destroy(IServiceContainer container) { }
    }
}
