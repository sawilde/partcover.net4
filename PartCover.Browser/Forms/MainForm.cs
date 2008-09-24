using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

using PartCover.Browser.Dialogs;
using PartCover.Browser.Properties;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Stuff;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api;
using PartCover.Browser.Features;
using PartCover.Browser.Helpers;

namespace PartCover.Browser
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public partial class MainForm
        : Form
        , IReportViewValve
    {
        private Dictionary<IReportViewFactory, ReportView> viewFactories = new Dictionary<IReportViewFactory, ReportView>();

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
            ServiceContainer.getService<ICoverageReportService>().loadFromFile(dlgOpen.FileName);
        }

        private void mmFileExit_Click(object sender, EventArgs e)
        {
            Close();
        }


        private void mmFileSaveAs_Click(object sender, EventArgs e)
        {
            if (dlgSave.ShowDialog(this) != DialogResult.OK)
                return;
            ServiceContainer.getService<ICoverageReportService>().saveReport(dlgSave.FileName);
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

            TargetRunner runner = new TargetRunner();
            runner.RunTargetForm = runTargetForm;
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
                StreamWriter writer = new StreamWriter(dlgSave.FileName);
                CoverageReportHelper.WriteReport(runner.Report, writer);
                writer.Close();
            }
            else
            {
                ServiceContainer.getService<ICoverageReportService>().load(runner.Report);
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
            using (SettingsForm form = new SettingsForm())
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
            Menu.MenuItemCollection items = miHtml.MenuItems;
            foreach (string s in HtmlPreview.enumTransforms())
            {
                MenuItem item = new MenuItem();

                item.Text = s;
                item.Click += delegate(object sender, EventArgs e)
                {
                    MakeHtmlPreview(s);
                };

                items.Add(item);
            }
        }

        private void MakeHtmlPreview(string s) { }

        public void add(IReportViewFactory factory)
        {
            viewFactories.Add(factory, null);
            miViews.MenuItems.Add(factory.ViewName, delegate
            {
                showView(factory);
            });
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

            ReportView view = viewFactories[factory];
            if (view == null)
            {
                viewFactories[factory] = view = factory.create();

                view.WindowState = FormWindowState.Maximized;
                view.MdiParent = this;
                view.Text = factory.ViewName;

                TinyAsyncUserProcess asyncProcess = new TinyAsyncUserProcess();
                asyncProcess.Action = delegate(IProgressTracker tracker)
                {
                    view.attach(serviceContainer, tracker);
                };
                asyncProcess.execute(this);

                view.FormClosed += delegate {
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

