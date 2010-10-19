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

            var mainForm = host.getService<MainForm>();

            var reportName = FindReportArgument();
            if (!String.IsNullOrEmpty(reportName))
            {
                if (System.IO.File.Exists(reportName))
                {
                    log.Debug("open form with report argument: " + reportName);
                    mainForm.OpenReport(reportName);
                }
                else
                {
                    // todo: should we report this to the user?
                    log.Debug("report argument file does not exist: " + reportName);
                }
            }
            
            Application.Run(mainForm);

            host.destroy();
        }

        private static string FindReportArgument()
        {
            var reportNameIsNextArg = false;

            foreach (string arg in Environment.GetCommandLineArgs())
            {
                if (reportNameIsNextArg)
                {
                    log.Debug("Found report argument: " + arg);
                    return arg;
                }
                reportNameIsNextArg = (arg.ToLower().Equals("--report"));
            }

            return null;
        }
    }
}
