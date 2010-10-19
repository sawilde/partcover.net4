using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using PartCover.Browser.Dialogs;
using PartCover.Browser.Features;
using PartCover.Browser.Properties;
using PartCover.Browser.Stuff;
using PartCover.Framework;
using PartCover.Browser.Api;
using PartCover.Browser.Helpers;
using PartCover.Framework.Data;

namespace PartCover.Browser.Forms
{
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public partial class MainForm
        : Form
        , IReportViewValve
        , ITreeItemSelectionHandler
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
            SelectionHandlers = new List<ITreeItemSelectionHandler>();
            InitializeComponent();
            BuildTransformMenu();

            tvItems.TreeItemSelectionHandler = this;

            SelectionHandlers.Add(ctlNodeView);
        }


        private void mmFileOpen_Click(object sender, EventArgs e)
        {
            if (dlgOpen.ShowDialog(this) != DialogResult.OK)
                return;

            this.OpenReport(dlgOpen.FileName);
        }
        
        public void OpenReport(string fileName)
        {
            CloseViews();
            ServiceContainer.getService<IReportService>().LoadFromFile(fileName);
 	}

        private void mmFileExit_Click(object sender, EventArgs e)
        {
            Close();
        }


        private void mmFileSaveAs_Click(object sender, EventArgs e)
        {
            if (dlgSave.ShowDialog(this) != DialogResult.OK)
                return;
            ServiceContainer.getService<IReportService>().SaveToFile(dlgSave.FileName);
        }

        readonly RunTargetForm runTargetForm = new RunTargetForm();

        //private void ShowError(string error)
        //{
        //    MessageBox.Show(this, error, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        //}

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

            runner.Execute(this);

            ServiceContainer.getService<IReportService>().Open(runner.Report);
            var reportService = ServiceContainer.getService<IReportService>() as CoverageReportService;
            if (reportService != null)
            {
                reportService.LastRunLog = runner.RunLog.ToString();
            }

            if (runner.Report.Assemblies.Count == 0)
            {
                ShowEmptyReportDialog(runner.Report);
            }
            else
            {
                ProcessReportReceived(runner.Report);
            }
        }

        private void ShowEmptyReportDialog(Report report)
        {
            DialogResult dlgResult;
            using (var dialog = new RunEmptyReport())
            {
                dlgResult = dialog.ShowDialog(this);
            }

            switch (dlgResult)
            {
            case DialogResult.Cancel:
                return;
            case DialogResult.Retry:
                mmRunTarget.PerformClick();
                return;
            case DialogResult.OK:
                ShowSkippedItems(report);
                return;
            }
        }

        private void ShowSkippedItems(Report report)
        {
            using (var dlg = new SkippedItemsReport())
            {
                dlg.Items = report.SkippedItems;
                dlg.RuleReceiver = runTargetForm.AddIncludeRule;
                dlg.ShowDialog(this);
            }
        }

        private void ProcessReportReceived(Report report)
        {
            if (runTargetForm.OutputToFile)
            {
                using (var writer = new XmlTextWriter(dlgSave.FileName, Encoding.UTF8))
                {
                    writer.Formatting = Formatting.Indented;
                    writer.Indentation = 1;
                    writer.IndentChar = ' ';
                    ReportSerializer.Save(writer, report);
                }
            }
            else
            {
                ServiceContainer.getService<IReportService>().Open(report);
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
            if (ServiceContainer.getService<IReportService>().ReportFileName == null)
            {
                mmFileSaveAs.PerformClick();
            }

            if (ServiceContainer.getService<IReportService>().ReportFileName == null)
                return;

            var asyncProcess = new TinyAsyncUserProcess
            {
                Action = tracker => HtmlPreview.DoTransform(tracker, ServiceContainer.getService<IReportService>().ReportFileName, transform)
            };

            asyncProcess.Execute(this);
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

                if (view is ITreeItemSelectionHandler)
                {
                    SelectionHandlers.Add((ITreeItemSelectionHandler)view);
                }

                var asyncProcess = new TinyAsyncUserProcess
                {
                    Action = tracker => view.attach(serviceContainer, tracker)
                };
                asyncProcess.Execute(this);

                view.FormClosed += delegate
                {
                    if (view is ITreeItemSelectionHandler)
                    {
                        SelectionHandlers.Remove((ITreeItemSelectionHandler)view);
                    }

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
            using (var form = new AboutForm())
                form.ShowDialog(this);
        }

        List<ITreeItemSelectionHandler> SelectionHandlers { get; set; }

        #region Implementation of ITreeItemSelectionHandler

        public void Select(AssemblyEntry assembly)
        {
            SelectionHandlers.ForEach(x => x.Select(assembly));
        }

        public void Select(AssemblyEntry assembly, string namespacePath)
        {
            SelectionHandlers.ForEach(x => x.Select(assembly, namespacePath));
        }

        public void Select(TypedefEntry typedef)
        {
            SelectionHandlers.ForEach(x => x.Select(typedef));
        }

        public void Select(MethodEntry method)
        {
            SelectionHandlers.ForEach(x => x.Select(method));
        }

        public void Deselect()
        {
            SelectionHandlers.ForEach(x => x.Deselect());
        }

        #endregion

        private void mmFileShowSkipped_Click(object sender, EventArgs e)
        {
            var service = ServiceContainer.getService<IReportService>();
            if (service.Report == null || service.Report.SkippedItems.Count == 0)
            {
                ShowInformation("No skipped items are available. Run the report first.");
                return;
            }

            ShowSkippedItems(service.Report);
        }

        private void mmFileShowLog_Click(object sender, EventArgs e)
        {
            var service = ServiceContainer.getService<IReportService>();
            if (service.LastRunLog == null)
            {
                return;
            }
            
            using (var dlg = new RunLogReport()) {
                dlg.AttachLog(service.LastRunLog);
                dlg.ShowDialog(this);
            }
        }

    
    }
}