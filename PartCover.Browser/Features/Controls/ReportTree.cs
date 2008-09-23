using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using PartCover.Browser.Api;
using PartCover.Browser.Api.ReportItems;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Stuff;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Controls
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

            IReportItem item = serviceContainer.getService<IReportItemSelectionService>().SelectedItem;

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

            ICoverageReport report = serviceContainer.getService<ICoverageReportService>().Report;

            foreach (IAssembly assembly in report.getAssemblies())
            {
                AssemblyTreeNode asmNode = new AssemblyTreeNode(assembly);
                Nodes.Add(asmNode);

                foreach (IClass dType in assembly.getTypes())
                {
                    TreeNode namespaceNode = GetNamespaceNode(asmNode, dType);
                    ClassTreeNode classNode = new ClassTreeNode(dType);
                    namespaceNode.Nodes.Add(classNode);

                    Dictionary<string, PropertyTreeNode> props = new Dictionary<string, PropertyTreeNode>();
                    foreach (IMethod md in dType.getMethods())
                    {
                        if (!Methods.isSpecial(md.Flags))
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        //has special meaning
                        MdSpecial mdSpecial = Methods.getMdSpecial(md.Name);
                        if (mdSpecial == MdSpecial.Unknown)
                        {
                            AddMethodTreeNode(classNode, md);
                            continue;
                        }

                        string propName = Methods.getMdSpecialName(md.Name);

                        PropertyTreeNode propertyNode;
                        if (!props.TryGetValue(propName, out propertyNode))
                        {
                            propertyNode = new PropertyTreeNode(propName);
                            props[propName] = propertyNode;
                            classNode.Nodes.Add(propertyNode);
                        }

                        MethodTreeNode mdNode = new MethodTreeNode(md);
                        mdNode.MethodName = mdSpecial.ToString().ToLowerInvariant();

                        switch (mdSpecial)
                        {
                            case MdSpecial.Get:
                                mdNode.ImageIndex = ImageSelector.forPropertyGet(md);
                                mdNode.SelectedImageIndex = ImageSelector.forPropertyGet(md);
                                propertyNode.Getter = mdNode;
                                break;
                            case MdSpecial.Remove:
                                mdNode.ImageIndex = ImageSelector.forEventRemove(md);
                                mdNode.SelectedImageIndex = ImageSelector.forEventRemove(md);
                                propertyNode.Getter = mdNode;
                                break;
                            case MdSpecial.Set:
                                mdNode.ImageIndex = ImageSelector.forPropertySet(md);
                                mdNode.SelectedImageIndex = ImageSelector.forPropertySet(md);
                                propertyNode.Setter = mdNode;
                                break;
                            case MdSpecial.Add:
                                mdNode.ImageIndex = ImageSelector.forEventAdd(md);
                                mdNode.SelectedImageIndex = ImageSelector.forEventAdd(md);
                                propertyNode.Setter = mdNode;
                                break;
                        }
                    }

                    foreach (KeyValuePair<string, PropertyTreeNode> kv in props)
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
                NamespaceTreeNode ntn = tn as NamespaceTreeNode;
                if (ntn != null && ntn.Namespace == iNamespace)
                    return ntn;
            }
            return null;
        }

        private static ClassTreeNode FindClassNode(TreeNodeCollection nodes, IClass iClass)
        {
            foreach (TreeNode nd in nodes)
            {
                ClassTreeNode csNode = nd as ClassTreeNode;
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
            MethodTreeNode mNode = new MethodTreeNode(md);
            classNode.Nodes.Add(mNode);

            if (md.CoveredVariants.Length > 1)
            {
                foreach (ICoveredVariant bData in md.CoveredVariants)
                    mNode.Nodes.Add(new BlockVariantTreeNode(bData));
            }
        }

        private static TreeNode GetNamespaceNode(TreeNode asmNode, IClass iClass)
        {
            INamespace[] names = iClass.getNamespaceChain();
            TreeNode parentNode = asmNode;
            for (int i = 0; i < names.Length; ++i)
            {
                NamespaceTreeNode nextNode = FindNamespaceNode(parentNode.Nodes, names[i]);
                if (nextNode == null)
                {
                    nextNode = new NamespaceTreeNode(names[i]);
                    parentNode.Nodes.Add(nextNode);
                }
                parentNode = nextNode;
            }
            return parentNode;
        }


        private bool selecting = false;
        protected override void OnAfterSelect(TreeViewEventArgs e)
        {
            base.OnAfterSelect(e);

            if (selecting) return;
            selecting = true;

            IReportItemSelectionService service = serviceContainer.getService<IReportItemSelectionService>();
            if (e.Node is MethodTreeNode)
            {
                service.select(((MethodTreeNode)e.Node).Method);
            }
            else if (e.Node is BlockVariantTreeNode)
            {
                service.select(((BlockVariantTreeNode)e.Node).VariantData);
            }
            else if (e.Node is ClassTreeNode)
            {
                service.select(((ClassTreeNode)e.Node).Class);
            }
            else if (e.Node is AssemblyTreeNode)
            {
                service.select(((AssemblyTreeNode)e.Node).Assembly);
            }
            else if (e.Node is NamespaceTreeNode)
            {
                service.select(((NamespaceTreeNode)e.Node).Namespace);
            }
            else
            {
                service.selectNone();
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

            Queue<INamespace> namespaces = new Queue<INamespace>();
            while (iNamespace != null)
                namespaces.Enqueue(iNamespace);

            while (null != (iNamespace = namespaces.Dequeue()))
            {
                NamespaceTreeNode treeNode = FindNamespaceNode(nmNode.Nodes, iNamespace);
                if (treeNode == null)
                    return null;
                nmNode = treeNode;
            }

            return nmNode;
        }

        private ClassTreeNode FindNode(IClass iClass)
        {
            TreeNodeCollection nodes;

            INamespace[] namespaces = iClass.getNamespaceChain();
            if (namespaces.Length > 0)
            {
                nodes = FindNode(namespaces[namespaces.Length - 1]).Nodes;
            }
            else
            {
                nodes = FindNode(iClass.Assembly).Nodes;
            }

            return FindClassNode(nodes, iClass);
        }

        private MethodTreeNode FindNode(IMethod iMethod)
        {
            return FindMethodNode(FindNode(iMethod.Class).Nodes, iMethod);
        }
    }
}

class asdasd { }
