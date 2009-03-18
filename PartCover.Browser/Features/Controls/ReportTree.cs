using System;
using System.Collections.Generic;
using System.Windows.Forms;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
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

        private IServiceContainer serviceContainer;
        public IServiceContainer ServiceContainer
        {
            get { return serviceContainer; }
            set
            {
                if (serviceContainer != null) unadviseEvents();
                serviceContainer = value;
                if (serviceContainer != null) adviseEvents();
            }
        }

        private void adviseEvents()
        {
            serviceContainer.getService<ICoverageReportService>().ReportClosing += onReportClosing;
            serviceContainer.getService<ICoverageReportService>().ReportOpened += onReportOpened;
            serviceContainer.getService<IReportItemSelectionService>().SelectionChanged += onSelectionChanged;
        }

        private void unadviseEvents()
        {
            serviceContainer.getService<ICoverageReportService>().ReportClosing -= onReportClosing;
            serviceContainer.getService<ICoverageReportService>().ReportOpened -= onReportOpened;
            serviceContainer.getService<IReportItemSelectionService>().SelectionChanged -= onSelectionChanged;
        }

        void onSelectionChanged(object sender, EventArgs e)
        {
            if (selecting) return;

            var item = serviceContainer.getService<IReportItemSelectionService>().SelectedItem;

            TreeNode selectedNode = null;
            if (item is INamespace)
            {
                selectedNode = FindNode((INamespace)item);
            }
            else if (item is IAssembly)
            {
                selectedNode = FindNode((IAssembly)item);
            }
            else if (item is IClass)
            {
                selectedNode = FindNode((IClass)item);
            }
            else if (item is IMethod)
            {
                selectedNode = FindNode((IMethod)item);
            }

            SelectedNode = selectedNode;
        }

        void onReportOpened(object sender, EventArgs e)
        {
            BeginUpdate();

            var report = serviceContainer.getService<ICoverageReportService>().Report;

            foreach (var assembly in report.Assemblies)
            {
                var asmNode = new AssemblyTreeNode(assembly);
                Nodes.Add(asmNode);

                foreach (var dType in assembly.getTypes())
                {
                    var namespaceNode = GetNamespaceNode(asmNode, dType);
                    var classNode = new ClassTreeNode(dType);
                    namespaceNode.Nodes.Add(classNode);

                    var props = new Dictionary<string, PropertyTreeNode>();
                    foreach (var md in dType.getMethods())
                    {
                        if (!Methods.isSpecial(md.Flags))
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        //has special meaning
                        var mdSpecial = Methods.getMdSpecial(md.Name);
                        if (mdSpecial == MdSpecial.Unknown)
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        var propName = Methods.getMdSpecialName(md.Name);

                        PropertyTreeNode propertyNode;
                        if (!props.TryGetValue(propName, out propertyNode))
                        {
                            propertyNode = new PropertyTreeNode(propName);
                            props[propName] = propertyNode;
                            classNode.Nodes.Add(propertyNode);
                        }

                        var mdNode = new MethodTreeNode(md)
                        {
                            MethodName = mdSpecial.ToString().ToLowerInvariant()
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

        void onReportClosing(object sender, EventArgs e)
        {
            BeginUpdate();
            Nodes.Clear();
            EndUpdate();
        }

        private static NamespaceTreeNode FindNamespaceNode(TreeNodeCollection nodes, INamespace iNamespace)
        {
            foreach (TreeNode tn in nodes)
            {
                var ntn = tn as NamespaceTreeNode;
                if (ntn != null && ntn.Namespace == iNamespace)
                    return ntn;
            }
            return null;
        }

        private static ClassTreeNode FindClassNode(TreeNodeCollection nodes, IClass iClass)
        {
            foreach (TreeNode nd in nodes)
            {
                var csNode = nd as ClassTreeNode;
                if (csNode != null && csNode.Class == iClass)
                    return csNode;
            }
            return null;
        }

        private static MethodTreeNode FindMethodNode(TreeNodeCollection nodes, IMethod iMethod)
        {
            MethodTreeNode mdNode = null;
            foreach (TreeNode nd in nodes)
            {
                if (mdNode != null)
                    break;

                mdNode = nd as MethodTreeNode;

                if (mdNode != null && mdNode.Method == iMethod)
                    return mdNode;

                if (mdNode == null && nd.Nodes.Count > 0)
                    mdNode = FindMethodNode(nd.Nodes, iMethod);
            }
            return mdNode;
        }

        private static void AddMethodTreeNode(TreeNode classNode, IMethod md)
        {
            var mNode = new MethodTreeNode(md);
            classNode.Nodes.Add(mNode);

            if (md.CoveredVariants.Length <= 1)
            {
                return;
            }

            foreach (var bData in md.CoveredVariants)
                mNode.Nodes.Add(new BlockVariantTreeNode(bData));
        }

        private static TreeNode GetNamespaceNode(TreeNode asmNode, IClass iClass)
        {
            var names = iClass.getNamespaceChain();
            var parentNode = asmNode;
            for (var i = 0; i < names.Length; ++i)
            {
                var nextNode = FindNamespaceNode(parentNode.Nodes, names[i]);
                if (nextNode == null)
                {
                    nextNode = new NamespaceTreeNode(names[i]);
                    parentNode.Nodes.Add(nextNode);
                }
                parentNode = nextNode;
            }
            return parentNode;
        }


        private bool selecting;
        protected override void OnAfterSelect(TreeViewEventArgs e)
        {
            base.OnAfterSelect(e);

            if (selecting) return;
            selecting = true;

            var service = serviceContainer.getService<IReportItemSelectionService>();
            if (e.Node is MethodTreeNode)
            {
                service.Select(((MethodTreeNode)e.Node).Method);
            }
            else if (e.Node is BlockVariantTreeNode)
            {
                service.Select(((BlockVariantTreeNode)e.Node).VariantData);
            }
            else if (e.Node is ClassTreeNode)
            {
                service.Select(((ClassTreeNode)e.Node).Class);
            }
            else if (e.Node is AssemblyTreeNode)
            {
                service.Select(((AssemblyTreeNode)e.Node).Assembly);
            }
            else if (e.Node is NamespaceTreeNode)
            {
                service.Select(((NamespaceTreeNode)e.Node).Namespace);
            }
            else
            {
                service.SelectNone();
            }

            selecting = false;
            Focus();
        }

        private AssemblyTreeNode FindNode(IAssembly iAssembly)
        {
            foreach (AssemblyTreeNode nd in Nodes)
                if (nd.Assembly == iAssembly)
                    return nd;
            return null;
        }

        private TreeNode FindNode(INamespace iNamespace)
        {
            TreeNode nmNode = FindNode(iNamespace.Assembly);

            var namespaces = new Queue<INamespace>();
            while (iNamespace != null)
            {
                namespaces.Enqueue(iNamespace);
                iNamespace = iNamespace.Parent;
            }

            while (null != (iNamespace = namespaces.Dequeue()))
            {
                var treeNode = FindNamespaceNode(nmNode.Nodes, iNamespace);
                if (treeNode == null)
                    return null;
                nmNode = treeNode;
            }

            return nmNode;
        }

        private ClassTreeNode FindNode(IClass iClass)
        {
            var namespaces = iClass.getNamespaceChain();
            var nodes = namespaces.Length > 0
                ? FindNode(namespaces[namespaces.Length - 1]).Nodes
                : FindNode(iClass.Assembly).Nodes;
            return FindClassNode(nodes, iClass);
        }

        private MethodTreeNode FindNode(IMethod iMethod)
        {
            return FindMethodNode(FindNode(iMethod.Class).Nodes, iMethod);
        }
    }
}