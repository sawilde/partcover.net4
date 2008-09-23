using System;
using System.Collections;
using System.Windows.Forms;

using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Stuff;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Controls
{
    internal class TreeNodeSorter : IComparer
    {
        private class NodeTypeDetector : INodeVisitor
        {
            int level;
            public int Level { get { return level; } }
            public void forAssembly(AssemblyTreeNode node) { level = 0; }
            public void forNamespace(NamespaceTreeNode node) { level = 1; }
            public void forType(ClassTreeNode node) { level = 2; }
            public void forProperty(PropertyTreeNode node) { level = 3; }
            public void forMethod(MethodTreeNode node) { level = 3; }
            public void forBlockVariant(BlockVariantTreeNode node) { level = 4; }
            public void reset() { level = 0; }
        }

        private readonly MethodTreeNodeComparer methodComparer = MethodTreeNodeComparer.Default;
        private readonly NodeTypeDetector xDetector = new NodeTypeDetector();
        private readonly NodeTypeDetector yDetector = new NodeTypeDetector();

        public int Compare(object x, object y)
        {
            TreeNodeBase nodeX = (TreeNodeBase)x;
            TreeNodeBase nodeY = (TreeNodeBase)y;

            if (nodeX == null && nodeY == null) return 0;
            if (nodeY == null) return 1;
            if (nodeX == null) return -1;

            if (ReferenceEquals(nodeX, nodeY)) return 0;

            xDetector.reset();
            nodeX.visit(xDetector);
            yDetector.reset();
            nodeY.visit(yDetector);

            if (0 != xDetector.Level.CompareTo(yDetector.Level))
                return xDetector.Level.CompareTo(yDetector.Level);

            switch (xDetector.Level)
            {
                case 3: return methodComparer.Compare(nodeX, nodeY);
                default: return nodeX.Text.CompareTo(nodeY.Text);
            }
        }
    }

    interface ICoverageInfo
    {
        UInt32 GetCodeSize();
        UInt32 GetCoveredCodeSize();
        void UpdateCoverageInfo();
    }

    interface INodeVisitor
    {
        void forAssembly(AssemblyTreeNode node);
        void forNamespace(NamespaceTreeNode node);
        void forType(ClassTreeNode node);
        void forProperty(PropertyTreeNode node);
        void forMethod(MethodTreeNode node);
        void forBlockVariant(BlockVariantTreeNode node);
    }

    internal abstract class TreeNodeBase : TreeNode, IVisitable<INodeVisitor>
    {
        public TreeNodeBase(string name, int imageIndex, int selectedImageIndex) : base(name, imageIndex, selectedImageIndex) { }
        public TreeNodeBase(string name) : base(name) { }

        public abstract void visit(INodeVisitor visitor);
    }

    internal class AssemblyTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        private readonly IAssembly assembly;
        public IAssembly Assembly
        {
            get { return assembly; }
        }

        public AssemblyTreeNode(IAssembly assembly)
            : base(assembly.Name)
        {
            this.assembly = assembly;
            ImageIndex = VSImage.Current.Assembly;
            SelectedImageIndex = VSImage.Current.Assembly;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach (ICoverageInfo iInfo in Nodes)
            {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCoveredCodeSize();
            }
            float percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", Assembly.Name, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forAssembly(this);
        }
        #endregion IVisitable Members
    }

    internal class NamespaceTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        readonly INamespace iNamespace;
        public INamespace Namespace
        {
            get { return iNamespace; }
        }

        public NamespaceTreeNode(INamespace iNamespace)
            : base(iNamespace.Name)
        {
            this.iNamespace = iNamespace;
            ImageIndex = VSImage.Current.Namespace;
            SelectedImageIndex = VSImage.Current.Namespace;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach (ICoverageInfo iInfo in Nodes)
            {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCoveredCodeSize();
            }
            float percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", Namespace.Name, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forNamespace(this);
        }
        #endregion IVisitable Members
    }

    internal class ClassTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        readonly IClass dType;
        public IClass Class
        {
            get { return dType; }
        }

        public ClassTreeNode(IClass dType)
            : base(CoverageReportHelper.GetTypeDefName(dType.Name))
        {
            this.dType = dType;
            ImageIndex = ImageSelector.forType(dType);
            SelectedImageIndex = ImageSelector.forType(dType);
        }

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forType(this);
        }
        #endregion IVisitable Members

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach (ICoverageInfo iInfo in Nodes)
            {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCoveredCodeSize();
            }
            float percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", CoverageReportHelper.GetTypeDefName(dType.Name), percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class PropertyTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        readonly string _property;
        MethodTreeNode _setter;
        MethodTreeNode _getter;

        public PropertyTreeNode(string text)
            : base(text)
        {
            ImageIndex = VSImage.Current.Properties;
            SelectedImageIndex = VSImage.Current.Properties;
            _property = text;
        }

        public MethodTreeNode Setter
        {
            get { return _setter; }
            set { _setter = value; }
        }
        public MethodTreeNode Getter
        {
            get { return _getter; }
            set { _getter = value; }
        }

        public uint GetCodeSize()
        {
            uint size = 0;
            if (Setter != null) size += Setter.GetCodeSize();
            if (Getter != null) size += Getter.GetCodeSize();
            return size;
        }

        public uint GetCoveredCodeSize()
        {
            uint size = 0;
            if (Setter != null) size += Setter.GetCoveredCodeSize();
            if (Getter != null) size += Getter.GetCoveredCodeSize();
            return size;
        }

        public void UpdateCoverageInfo()
        {
            if (Setter != null) Setter.UpdateCoverageInfo();
            if (Getter != null) Getter.UpdateCoverageInfo();

            float percent = GetCodeSize() == 0 ? 0 : GetCoveredCodeSize() / (float)GetCodeSize() * 100;

            Text = string.Format("{0} ({1:#0}%)", _property, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forProperty(this);
        }
        #endregion IVisitable Members
    }

    internal class MethodTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        readonly IMethod md;
        public IMethod Method
        {
            get { return md; }
        }

        private string methodName;
        public string MethodName
        {
            get { return methodName; }
            set
            {
                methodName = value;
                Text = value;
            }
        }

        public MethodTreeNode(IMethod md)
            : base(md.Name)
        {
            this.md = md;
            MethodName = md.Name;
            ImageIndex = ImageSelector.forMethod(md);
            SelectedImageIndex = ImageSelector.forMethod(md);
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            foreach (ICoverageInfo iInfo in Nodes)
                iInfo.UpdateCoverageInfo();

            codeSize = 0;
            coveredCodeSize = 0;
            foreach (ICoveredVariant bData in md.CoveredVariants)
            {
                codeSize = CoverageReportHelper.GetBlockCodeSize(bData.Blocks);
                coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(bData.Blocks);
            }
            float percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", MethodName, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }
        #endregion

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forMethod(this);
        }
        #endregion IVisitable Members
    }

    internal class BlockVariantTreeNode : TreeNodeBase
        , ICoverageInfo
        , IVisitable<INodeVisitor>
    {
        readonly ICoveredVariant bData;
        public ICoveredVariant VariantData
        {
            get { return bData; }
        }

        public BlockVariantTreeNode(ICoveredVariant bData)
            : base("Block Data")
        {
            this.bData = bData;
            ImageIndex = VSImage.Current.Namespace;
            SelectedImageIndex = VSImage.Current.Namespace;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            codeSize = CoverageReportHelper.GetBlockCodeSize(bData.Blocks);
            coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(bData.Blocks);

            float percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("Block Data ({0:#0}%)", percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }
        #endregion

        #region IVisitable Members
        public override void visit(INodeVisitor visitor)
        {
            visitor.forBlockVariant(this);
        }
        #endregion IVisitable Members
    }
}
