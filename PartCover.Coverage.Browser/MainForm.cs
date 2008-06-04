using System;
using System.IO;
using System.Diagnostics;
using System.Xml;
using System.Globalization;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using PartCover.Framework.Walkers;

namespace PartCover.Coverage.Browser
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
    public class MainForm : System.Windows.Forms.Form {
        private System.Windows.Forms.MainMenu mm;
        private System.Windows.Forms.MenuItem mmFile;
        private System.Windows.Forms.MenuItem mmFileOpen;
        private System.Windows.Forms.OpenFileDialog dlgOpen;
        private System.Windows.Forms.TreeView tvItems;
        private System.Windows.Forms.Splitter splTree;
        private System.Windows.Forms.MenuItem mmFileExit;
        private System.Windows.Forms.ImageList treeImg;
        private System.Windows.Forms.Panel pnBlockInfo;
        private System.Windows.Forms.MenuItem mmFileSaveAs;
        private System.Windows.Forms.MenuItem mmRunTarget;
        private System.Windows.Forms.MenuItem mmSep1;
        private System.Windows.Forms.MenuItem mmSep2;
        private System.Windows.Forms.SaveFileDialog dlgSave;
        private System.ComponentModel.IContainer components;

        public MainForm() {
            InitializeComponent();
        }

        protected override void Dispose( bool disposing ) {
            if( disposing ) {
                if (components != null) {
                    components.Dispose();
                }
            }
            base.Dispose( disposing );
        }

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MainForm));
            this.mm = new System.Windows.Forms.MainMenu();
            this.mmFile = new System.Windows.Forms.MenuItem();
            this.mmRunTarget = new System.Windows.Forms.MenuItem();
            this.mmSep2 = new System.Windows.Forms.MenuItem();
            this.mmFileOpen = new System.Windows.Forms.MenuItem();
            this.mmFileSaveAs = new System.Windows.Forms.MenuItem();
            this.mmSep1 = new System.Windows.Forms.MenuItem();
            this.mmFileExit = new System.Windows.Forms.MenuItem();
            this.dlgOpen = new System.Windows.Forms.OpenFileDialog();
            this.tvItems = new System.Windows.Forms.TreeView();
            this.treeImg = new System.Windows.Forms.ImageList(this.components);
            this.splTree = new System.Windows.Forms.Splitter();
            this.pnBlockInfo = new System.Windows.Forms.Panel();
            this.dlgSave = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // mm
            // 
            this.mm.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                               this.mmFile});
            // 
            // mmFile
            // 
            this.mmFile.Index = 0;
            this.mmFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                   this.mmRunTarget,
                                                                                   this.mmSep2,
                                                                                   this.mmFileOpen,
                                                                                   this.mmFileSaveAs,
                                                                                   this.mmSep1,
                                                                                   this.mmFileExit});
            this.mmFile.Text = "&File";
            // 
            // mmRunTarget
            // 
            this.mmRunTarget.Index = 0;
            this.mmRunTarget.Text = "&Run Target";
            this.mmRunTarget.Click += new System.EventHandler(this.mmRunTarget_Click);
            // 
            // mmSep2
            // 
            this.mmSep2.Index = 1;
            this.mmSep2.Text = "-";
            // 
            // mmFileOpen
            // 
            this.mmFileOpen.Index = 2;
            this.mmFileOpen.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.mmFileOpen.Text = "&Open Report";
            this.mmFileOpen.Click += new System.EventHandler(this.mmFileOpen_Click);
            // 
            // mmFileSaveAs
            // 
            this.mmFileSaveAs.Index = 3;
            this.mmFileSaveAs.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.mmFileSaveAs.Text = "&Save Report As ...";
            this.mmFileSaveAs.Click += new System.EventHandler(this.mmFileSaveAs_Click);
            // 
            // mmSep1
            // 
            this.mmSep1.Index = 4;
            this.mmSep1.Text = "-";
            // 
            // mmFileExit
            // 
            this.mmFileExit.Index = 5;
            this.mmFileExit.Text = "E&xit";
            this.mmFileExit.Click += new System.EventHandler(this.mmFileExit_Click);
            // 
            // dlgOpen
            // 
            this.dlgOpen.Filter = "PartCover report|*.xml";
            // 
            // tvItems
            // 
            this.tvItems.Dock = System.Windows.Forms.DockStyle.Left;
            this.tvItems.ImageList = this.treeImg;
            this.tvItems.Location = new System.Drawing.Point(0, 0);
            this.tvItems.Name = "tvItems";
            this.tvItems.Size = new System.Drawing.Size(208, 485);
            this.tvItems.Sorted = true;
            this.tvItems.TabIndex = 0;
            this.tvItems.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.tvItems_AfterSelect);
            // 
            // treeImg
            // 
            this.treeImg.ImageSize = new System.Drawing.Size(16, 16);
            this.treeImg.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("treeImg.ImageStream")));
            this.treeImg.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // splTree
            // 
            this.splTree.Location = new System.Drawing.Point(208, 0);
            this.splTree.Name = "splTree";
            this.splTree.Size = new System.Drawing.Size(3, 485);
            this.splTree.TabIndex = 1;
            this.splTree.TabStop = false;
            // 
            // pnBlockInfo
            // 
            this.pnBlockInfo.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pnBlockInfo.Location = new System.Drawing.Point(211, 0);
            this.pnBlockInfo.Name = "pnBlockInfo";
            this.pnBlockInfo.Size = new System.Drawing.Size(469, 485);
            this.pnBlockInfo.TabIndex = 2;
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(680, 485);
            this.Controls.Add(this.pnBlockInfo);
            this.Controls.Add(this.splTree);
            this.Controls.Add(this.tvItems);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mm;
            this.Name = "MainForm";
            this.Text = "PartCover coverage browser";
            this.ResumeLayout(false);

        }
        #endregion

        [STAThread]
        static void Main() {
            Application.Run(new MainForm());
        }

        CoverageReport report;

        private void mmFileOpen_Click(object sender, System.EventArgs e) {
            if(dlgOpen.ShowDialog(this) == DialogResult.OK) {
                HideVariant();

                report = new CoverageReport();

                StreamReader reader = new StreamReader(dlgOpen.FileName);
                CoverageReportHelper.ReadReport(report, reader);
                reader.Close();

                ShowReport();
            }
        }

        private void mmFileExit_Click(object sender, System.EventArgs e) {
            this.Close();
        }

        private void ShowReport() {
            tvItems.BeginUpdate();
            tvItems.Nodes.Clear();

            if (report == null)
                return;

            foreach(string assemblyName in CoverageReportHelper.GetAssemblies(report)) {
                AssemblyTreeNode asmNode = new AssemblyTreeNode(assemblyName);
                tvItems.Nodes.Add(asmNode);

                foreach(CoverageReport.TypeDescriptor dType in CoverageReportHelper.GetTypes(report, assemblyName)) {
                    TreeNode namespaceNode = GetNamespaceNode(asmNode, dType.typeName);
                    ClassTreeNode classNode = new ClassTreeNode(dType);

                    namespaceNode.Nodes.Add(classNode);

                    foreach(CoverageReport.MethodDescriptor md in dType.methods) {
                        MethodTreeNode mNode = new MethodTreeNode(md);
                        classNode.Nodes.Add(mNode);

                        if (md.insBlocks.Length > 1) {
                            foreach(CoverageReport.InnerBlockData bData in md.insBlocks)
                                mNode.Nodes.Add(new BlockVariantTreeNode(bData));
                        }
                    }
                }

                asmNode.UpdateCoverageInfo();
            }

            tvItems.EndUpdate();
        }

        private TreeNode GetNamespaceNode(TreeNode asmNode, string typedefName) {
            string[] names = CoverageReportHelper.SplitNamespaces(typedefName);
            TreeNode parentNode = asmNode;
            for(int i = 0; i < names.Length - 1; ++i) {
                NamespaceTreeNode nextNode = FindNamespaceNode(parentNode.Nodes, names[i]);
                if (nextNode == null) {
                    nextNode = new NamespaceTreeNode(names[i]);
                    parentNode.Nodes.Add(nextNode);
                }
                parentNode = nextNode;
            }
            return parentNode;
        }

        private NamespaceTreeNode FindNamespaceNode(TreeNodeCollection nodes, string namespaceName) {
            foreach(TreeNode tn in nodes) {
                NamespaceTreeNode ntn = tn as NamespaceTreeNode;
                if (ntn == null)
                    continue;
                if (ntn.Namespace == namespaceName) 
                    return ntn;
            }
            return null;
        }

        private void tvItems_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e) {
            if (e.Node == null) {
                ShowMethod(null);
                return;
            } 
            
            if (e.Node is MethodTreeNode) {
                MethodTreeNode tn = (MethodTreeNode) e.Node;
                ShowMethod(tn);
            } else if (e.Node is BlockVariantTreeNode) {
                BlockVariantTreeNode tn = (BlockVariantTreeNode) e.Node;
                ShowVariant(tn.BlockData);
            } else {
                ShowMethod(null);
            }

            ((Control)sender).Focus();
        }

        private void ShowMethod(MethodTreeNode tn) {
            if (tn == null || tn.Descriptor.insBlocks.Length != 1)
                ShowVariant(null);
            else
                ShowVariant(tn.Descriptor.insBlocks[0]);
        }

        private void HideVariant() {
            pnBlockInfo.Controls.Clear();
        }

        private void ShowVariant(CoverageReport.InnerBlockData bData) {
            HideVariant();
            if (bData == null || bData.blocks.Length == 0)
                return;

            BlockInfoControl infoControl = new BlockInfoControl();
            infoControl.Dock = DockStyle.Fill;

            pnBlockInfo.Controls.Add(infoControl);

            infoControl.SetBlockData(report, bData);
        }

        private void mmFileSaveAs_Click(object sender, System.EventArgs e) {
            if (report != null) {
                if (dlgSave.ShowDialog(this) == DialogResult.OK) {
                    StreamWriter writer = new StreamWriter(dlgSave.FileName);
                    CoverageReportHelper.WriteReport(report, writer);
                    writer.Close();
                }
            }
        }

        RunTargetForm runTargetForm = new RunTargetForm();

        private void ShowError(string error) {
            MessageBox.Show(this, error, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void ShowInformation(string error) {
            MessageBox.Show(this, error, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void mmRunTarget_Click(object sender, System.EventArgs e) {
            if (runTargetForm.ShowDialog() == DialogResult.OK) {

                try {
                    using(GenerationForm genForm = new GenerationForm()) {
                        genForm.TargetPath = runTargetForm.TargetPath;
                        genForm.TargetDir = runTargetForm.TargetWorkingDir;
                        genForm.TargetArgs = runTargetForm.TargetArgs;
                        genForm.IncludeRules = runTargetForm.IncludeItems;
                        genForm.ExcludeRules = runTargetForm.ExcludeItems;
                        genForm.ShowDialog(this);

                        if (genForm.Success) {
                            if (genForm.Report.types.Length == 0) {
                                ShowInformation("Report is empty. Check settings and run target again.");
                                return;
                            }

                            report = genForm.Report;
                            if (runTargetForm.OutputToFile) {
                                StreamWriter writer = new StreamWriter(dlgSave.FileName);
                                CoverageReportHelper.WriteReport(report, writer);
                                writer.Close();
                            } else {
                                HideVariant();
                                ShowReport();
                            }
                        }
                    }
                } catch (Exception ex) {
                    ShowError("Cannot get report! (" + ex.Message + ")");
                    return;
                }
                
            }
        }
    }
}
