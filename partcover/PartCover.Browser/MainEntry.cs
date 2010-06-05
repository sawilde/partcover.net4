using System;
using System.Windows.Forms;
using System.Reflection;
using log4net;
using PartCover.Browser.Forms;

namespace PartCover.Browser
{
    public class MainEntry
    {
        private static readonly ILog log = LogManager.GetLogger(MethodBase.GetCurrentMethod().DeclaringType);

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            var host = new ApplicationHost();

            log.Info("search for features");
            foreach (var feature in FeatureSeeker.seek(Assembly.GetExecutingAssembly()))
            {
                log.Info("register feature: " + feature.GetType());
                host.registerService(feature);
            }

            host.build();

            Application.Run(host.getService<MainForm>());

            host.destroy();
        }
    }
}
