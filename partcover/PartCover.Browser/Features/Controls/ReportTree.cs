using System;
using System.Collections.Generic;
using System.Windows.Forms;

using PartCover.Browser.Api;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Data;
using PartCover.Framework.Stuff;

namespace PartCover.Browser.Features.Controls
{
    public partial class ReportTree : TreeView
    {
        public ReportTree()
        {
            TreeViewNodeSorter = new TreeNodeSorter();
            ImageList = VSImage.Current.GetImageList();
        }

        internal ITreeItemSelectionHandler TreeItemSelectionHandler { get; set; }

        private IServiceContainer serviceContainer;
        public IServiceContainer ServiceContainer
        {
            get { return serviceContainer; }
            set
            {
                if (serviceContainer != null) UnadviseEvents();
                serviceContainer = value;
                if (serviceContainer != null) AdviseEvents();
            }
        }

        private void AdviseEvents()
        {
            serviceContainer.getService<IReportService>().ReportClosing += OnReportClosing;
            serviceContainer.getService<IReportService>().ReportOpened += OnReportOpened;
        }

        private void UnadviseEvents()
        {
            serviceContainer.getService<IReportService>().ReportClosing -= OnReportClosing;
            serviceContainer.getService<IReportService>().ReportOpened -= OnReportOpened;
        }

        void OnReportOpened(object sender, EventArgs e)
        {
            BeginUpdate();

            var report = serviceContainer.getService<IReportService>().Report;

            foreach (var assembly in report.Assemblies)
            {
                var asmNode = new AssemblyTreeNode(assembly);
                Nodes.Add(asmNode);

                foreach (var dType in assembly.Types)
                {
                    var namespaceNode = GetNamespaceNode(asmNode, dType);
                    var classNode = new ClassTreeNode(dType);
                    namespaceNode.Nodes.Add(classNode);

                    var props = new Dictionary<string, PropertyTreeNode>();
                    foreach (var md in dType.Methods)
                    {
                        if (!Methods.IsSpecial(md.Flags))
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        //has special meaning
                        var mdSpecial = Methods.GetMdSpecial(md.Name);
                        if (mdSpecial == MdSpecial.Unknown)
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        var propName = Methods.GetMdSpecialName(md.Name);

                        PropertyTreeNode propertyNode;
                        if (!props.TryGetValue(propName, out propertyNode))
                        {
                            propertyNode = new PropertyTreeNode(propName);
                            props[propName] = propertyNode;
                            classNode.Nodes.Add(propertyNode);
                        }

                        var mdNode = new MethodTreeNode(md)
                        {
                            //MethodName = mdSpecial.ToString().ToLowerInvariant()
                        };

                        switch (mdSpecial)
                        {
                        case MdSpecial.Get:
                            mdNode.ImageIndex = ImageSelector.ForPropertyGet(md);
                            mdNode.SelectedImageIndex = ImageSelector.ForPropertyGet(md);
                            propertyNode.Getter = mdNode;
                            break;
                        case MdSpecial.Remove:
                            mdNode.ImageIndex = ImageSelector.ForEventRemove(md);
                            mdNode.SelectedImageIndex = ImageSelector.ForEventRemove(md);
                            propertyNode.Getter = mdNode;
                            break;
                        case MdSpecial.Set:
                            mdNode.ImageIndex = ImageSelector.ForPropertySet(md);
                            mdNode.SelectedImageIndex = ImageSelector.ForPropertySet(md);
                            propertyNode.Setter = mdNode;
                            break;
                        case MdSpecial.Add:
                            mdNode.ImageIndex = ImageSelector.ForEventAdd(md);
                            mdNode.SelectedImageIndex = ImageSelector.ForEventAdd(md);
                            propertyNode.Setter = mdNode;
                            break;
                        }
                    }

                    foreach (var kv in props)
                    {
                        if (kv.Value.Getter != null) kv.Value.Nodes.Add(kv.Value.Getter);
                        if (kv.Value.Setter != null) kv.Value.Nodes.Add(kv.Value.Setter);
                    }
                }

                asmNode.UpdateCoverageInfo();
            }

            EndUpdate();
        }

        void OnReportClosing(object sender, EventArgs e)
        {
            BeginUpdate();
            Nodes.Clear();
            EndUpdate();
        }

        private static NamespaceTreeNode FindNamespaceNode(TreeNodeCollection nodes, string namespaceName)
        {
            foreach (TreeNode tn in nodes)
            {
                var ntn = tn as NamespaceTreeNode;
                if (ntn != null && ntn.Namespace == namespaceName)
                    return ntn;
            }
            return null;
        }

        private static void AddMethodTreeNode(TreeNode classNode, MethodEntry md)
        {
            classNode.Nodes.Add(new MethodTreeNode(md));
        }

        private static TreeNode GetNamespaceNode(AssemblyTreeNode asmNode, TypedefEntry iClass)
        {
            var names = Types.GetNamespaceChain(iClass.Name);
            TreeNode parentNode = asmNode;
            for (var i = 0; i < names.Length - 1; ++i)
            {
                var nextNode = FindNamespaceNode(parentNode.Nodes, names[i]);
                if (nextNode == null)
                {
                    nextNode = new NamespaceTreeNode(asmNode.Assembly, names[i])
                    {
                        NamespacePath = string.Join(".", names, 0, i + 1)
                    };
                    parentNode.Nodes.Add(nextNode);
                }
                parentNode = nextNode;
            }
            return parentNode;
        }

        protected override void OnAfterSelect(TreeViewEventArgs e)
        {
            base.OnAfterSelect(e);

            if (e.Node is AssemblyTreeNode)
            {
                var node = (AssemblyTreeNode)e.Node;
                TreeItemSelectionHandler.Select(node.Assembly);
            }
            else if (e.Node is NamespaceTreeNode)
            {
                var node = (NamespaceTreeNode)e.Node;
                TreeItemSelectionHandler.Select(node.Assembly, node.NamespacePath);
            }
            else if (e.Node is ClassTreeNode)
            {
                var node = (ClassTreeNode)e.Node;
                TreeItemSelectionHandler.Select(node.Typedef);
            }
            else if (e.Node is MethodTreeNode)
            {
                var node = (MethodTreeNode)e.Node;
                TreeItemSelectionHandler.Select(node.Method);
            }
            else
            {
                TreeItemSelectionHandler.Deselect();
            }
        }
    }
}