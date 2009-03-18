using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

using PartCover.Browser.Dialogs;
using PartCover.Browser.Properties;
using PartCover.Browser.Stuff;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api;
using PartCover.Browser.Helpers;

namespace PartCover.Browser.Forms
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public partial class MainForm : Form, IReportViewValve
    {
        private readonly Dictionary<IReportViewFactory, ReportView> viewFactories = new Dictionary<IReportViewFactory, ReportView>();

        private IServiceContainer serviceContainer;
        public IServiceContainer ServiceContainer
        {
            get { return serviceContainer; }
            set
            {
                tvItems.ServiceContainer = value;
                serviceContainer = value;
            }
        }

        public MainForm()
        {
            InitializeComponent();
            BuildTransformMenu();
        }


        private void mmFileOpen_Click(object sender, EventArgs e)
        {
            if (dlgOpen.ShowDialog(this) != DialogResult.OK)
                return;

            CloseViews();
            ServiceContainer.getService<ICoverageReportService>().LoadFromFile(dlgOpen.FileName);
        }

        private void mmFileExit_Click(object sender, EventArgs e)
        {
            Close();
        }


        private void mmFileSaveAs_Click(object sender, EventArgs e)
        {
            if (dlgSave.ShowDialog(this) != DialogResult.OK)
                return;
            ServiceContainer.getService<ICoverageReportService>().SaveToFile(dlgSave.FileName);
        }

        readonly RunTargetForm runTargetForm = new RunTargetForm();

        private void ShowError(string error)
        {
            MessageBox.Show(this, error, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void ShowInformation(string error)
        {
            MessageBox.Show(this, error, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void mmRunTarget_Click(object sender, EventArgs e)
        {
            if (runTargetForm.ShowDialog() != DialogResult.OK)
                return;

            CloseViews();

            var runner = new TargetRunner
            {
                RunTargetForm = runTargetForm
            };
            runner.execute(this);

            try
            {
                if (runner.Report.types.Count == 0)
                {
                    ShowInformation("Report is empty. Check settings and run target again.");
                    return;
                }

            }
            catch (Exception ex)
            {
                ShowError("Cannot get report! (" + ex.Message + ")");
                return;
            }

            if (runTargetForm.OutputToFile)
            {
                using (var writer = new StreamWriter(dlgSave.FileName))
                {
                    CoverageReportHelper.WriteReport(runner.Report, writer);
                }
            }
            else
            {
                ServiceContainer.getService<ICoverageReportService>().Load(runner.Report);
            }
        }

        private void CloseViews()
        {
            while (MdiChildren.Length > 0)
            {
                MdiChildren[0].Close();
            }
        }

        private void miSettings_Click(object sender, EventArgs e)
        {
            using (var form = new SettingsForm())
            {
                if (DialogResult.OK != form.ShowDialog(this))
                {
                    Settings.Default.Reset();
                }
                else
                {
                    Settings.Default.Save();
                }
            }
        }

        private void BuildTransformMenu()
        {
            var items = miHtml.MenuItems;
            foreach (var transform in HtmlPreview.enumTransforms())
            {
                var item = new MenuItem
                {
                    Text = transform
                };

                var transformName = transform;
                item.Click += delegate { MakeHtmlPreview(transformName); };

                items.Add(item);
            }
        }

        private void MakeHtmlPreview(string transform)
        {
            if (ServiceContainer.getService<ICoverageReportService>().ReportFileName == null)
            {
                mmFileSaveAs.PerformClick();
            }

            if (ServiceContainer.getService<ICoverageReportService>().ReportFileName == null)
                return;

            var asyncProcess = new TinyAsyncUserProcess
            {
                Action = tracker => HtmlPreview.DoTransform(tracker, ServiceContainer.getService<ICoverageReportService>().ReportFileName, transform)
            };

            asyncProcess.execute(this);

        }

        public void add(IReportViewFactory factory)
        {
            viewFactories.Add(factory, null);
            miViews.MenuItems.Add(factory.ViewName,
                delegate { showView(factory); });
        }

        public void remove(IReportViewFactory factory)
        {
            viewFactories.Remove(factory);
        }

        private delegate void ShowViewDelegate(IReportViewFactory factory);
        private void showView(IReportViewFactory factory)
        {
            if (InvokeRequired)
            {
                Invoke(new ShowViewDelegate(showView), factory);
                return;
            }

            var view = viewFactories[factory];
            if (view == null)
            {
                viewFactories[factory] = view = factory.Create();

                view.WindowState = FormWindowState.Maximized;
                view.MdiParent = this;
                view.Text = factory.ViewName;

                var asyncProcess = new TinyAsyncUserProcess
                {
                    Action = tracker => view.attach(serviceContainer, tracker)
                };
                asyncProcess.execute(this);

                view.FormClosed += delegate
                {
                    view.detach(serviceContainer, new DummyProgressTracker());
                    viewFactories[factory] = null;
                };
            }

            view.Show();
            view.Activate();
            view.Focus();
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            showView(ServiceContainer.getService<Features.FeatureViewCoverage>());
        }

        private void miAbout_Click(object sender, EventArgs e)
        {
            using (AboutForm form = new AboutForm())
                form.ShowDialog(this);
        }
    }
}