using System;
using System.Collections;
using System.Windows.Forms;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Stuff;
using PartCover.Framework.Walkers;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Features.Controls
{
    internal class TreeNodeSorter : IComparer
    {
        private class NodeTypeDetector : INodeVisitor
        {
            public int Level { get; private set; }

            public void OnAssembly(AssemblyTreeNode node) { Level = 0; }
            public void OnNamespace(NamespaceTreeNode node) { Level = 1; }
            public void OnType(ClassTreeNode node) { Level = 2; }
            public void OnProperty(PropertyTreeNode node) { Level = 3; }
            public void OnMethod(MethodTreeNode node) { Level = 3; }
            public void OnBlockVariant(BlockVariantTreeNode node) { Level = 4; }
            public void Reset() { Level = 0; }
        }

        private readonly MethodTreeNodeComparer methodComparer = MethodTreeNodeComparer.Default;
        private readonly NodeTypeDetector xDetector = new NodeTypeDetector();
        private readonly NodeTypeDetector yDetector = new NodeTypeDetector();

        public int Compare(object x, object y)
        {
            var nodeX = (TreeNodeBase)x;
            var nodeY = (TreeNodeBase)y;

            if (nodeX == null && nodeY == null) return 0;
            if (nodeY == null) return 1;
            if (nodeX == null) return -1;

            if (ReferenceEquals(nodeX, nodeY)) return 0;

            xDetector.Reset();
            nodeX.Visit(xDetector);
            yDetector.Reset();
            nodeY.Visit(yDetector);

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
        void OnAssembly(AssemblyTreeNode node);
        void OnNamespace(NamespaceTreeNode node);
        void OnType(ClassTreeNode node);
        void OnProperty(PropertyTreeNode node);
        void OnMethod(MethodTreeNode node);
        void OnBlockVariant(BlockVariantTreeNode node);
    }

    internal abstract class TreeNodeBase : TreeNode, IVisitable<INodeVisitor>
    {
        protected TreeNodeBase(string name, int imageIndex, int selectedImageIndex) : base(name, imageIndex, selectedImageIndex) { }
        protected TreeNodeBase(string name) : base(name) { }

        public abstract void Visit(INodeVisitor visitor);
    }

    internal class AssemblyTreeNode : TreeNodeBase, ICoverageInfo
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
        private UInt32 codeSize;
        private UInt32 coveredCodeSize;

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
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnAssembly(this);
        }
        #endregion IVisitable Members
    }

    internal class NamespaceTreeNode : TreeNodeBase, ICoverageInfo
    {
        public INamespace Namespace { get; private set; }

        public NamespaceTreeNode(INamespace iNamespace)
            : base(iNamespace.Name)
        {
            Namespace = iNamespace;
            ImageIndex = VSImage.Current.Namespace;
            SelectedImageIndex = VSImage.Current.Namespace;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize;
        private UInt32 coveredCodeSize;

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
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnNamespace(this);
        }
        #endregion IVisitable Members
    }

    internal class ClassTreeNode : TreeNodeBase, ICoverageInfo
    {
        public IClass Class { get; private set; }

        public ClassTreeNode(IClass dType)
            : base(CoverageReportHelper.GetTypeDefName(dType.Name))
        {
            Class = dType;
            ImageIndex = ImageSelector.ForType(dType);
            SelectedImageIndex = ImageSelector.ForType(dType);
        }

        #region IVisitable Members
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnType(this);
        }
        #endregion IVisitable Members

        #region ICoverageInfo Members
        private UInt32 codeSize;
        private UInt32 coveredCodeSize;

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
            Text = string.Format("{0} ({1:#0}%)", CoverageReportHelper.GetTypeDefName(Class.Name), percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class PropertyTreeNode : TreeNodeBase, ICoverageInfo
    {
        readonly string _property;

        public PropertyTreeNode(string text)
            : base(text)
        {
            ImageIndex = VSImage.Current.Properties;
            SelectedImageIndex = VSImage.Current.Properties;
            _property = text;
        }

        public MethodTreeNode Setter { get; set; }
        public MethodTreeNode Getter { get; set; }

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

            var percent = GetCodeSize() == 0 ? 0 : GetCoveredCodeSize() / (float)GetCodeSize() * 100;

            Text = string.Format("{0} ({1:#0}%)", _property, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #region IVisitable Members
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnProperty(this);
        }
        #endregion IVisitable Members
    }

    internal class MethodTreeNode : TreeNodeBase, ICoverageInfo
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
            ImageIndex = ImageSelector.ForMethod(md);
            SelectedImageIndex = ImageSelector.ForMethod(md);
        }

        #region ICoverageInfo Members
        private UInt32 codeSize;
        private UInt32 coveredCodeSize;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            foreach (ICoverageInfo iInfo in Nodes)
                iInfo.UpdateCoverageInfo();

            codeSize = 0;
            coveredCodeSize = 0;
            foreach (var bData in md.CoveredVariants)
            {
                codeSize = CoverageReportHelper.GetBlockCodeSize(bData.Blocks);
                coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(bData.Blocks);
            }
            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", MethodName, percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }
        #endregion

        #region IVisitable Members
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnMethod(this);
        }
        #endregion IVisitable Members
    }

    internal class BlockVariantTreeNode : TreeNodeBase, ICoverageInfo
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
        private UInt32 codeSize;
        private UInt32 coveredCodeSize;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            codeSize = CoverageReportHelper.GetBlockCodeSize(bData.Blocks);
            coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(bData.Blocks);

            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("Block Data ({0:#0}%)", percent);
            ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }
        #endregion

        #region IVisitable Members
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnBlockVariant(this);
        }
        #endregion IVisitable Members
    }
}