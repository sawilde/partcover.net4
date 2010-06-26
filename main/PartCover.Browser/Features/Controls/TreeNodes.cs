using System.Collections;
using System.Windows.Forms;
using PartCover.Browser.Resources;
using PartCover.Browser.Stuff;
using PartCover.Framework.Data;
using PartCover.Framework.Stuff;

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
        int GetCodeSize();
        int GetCoveredCodeSize();
        void UpdateCoverageInfo();
    }

    interface INodeVisitor
    {
        void OnAssembly(AssemblyTreeNode node);
        void OnNamespace(NamespaceTreeNode node);
        void OnType(ClassTreeNode node);
        void OnProperty(PropertyTreeNode node);
        void OnMethod(MethodTreeNode node);
    }

    internal abstract class TreeNodeBase : TreeNode, IVisitable<INodeVisitor>
    {
        protected TreeNodeBase(string name, int imageIndex, int selectedImageIndex) : base(name, imageIndex, selectedImageIndex) { }
        protected TreeNodeBase(string name) : base(name) { }

        public abstract void Visit(INodeVisitor visitor);
    }

    internal class AssemblyTreeNode : TreeNodeBase, ICoverageInfo
    {
        public AssemblyEntry Assembly { get; private set; }

        public AssemblyTreeNode(AssemblyEntry assembly)
            : base(assembly.Name)
        {
            Assembly = assembly;
            ImageIndex = VSImage.Current.Assembly;
            SelectedImageIndex = VSImage.Current.Assembly;
        }

        #region ICoverageInfo Members
        private int codeSize;
        private int coveredCodeSize;

        public int GetCodeSize() { return codeSize; }
        public int GetCoveredCodeSize() { return coveredCodeSize; }

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
            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
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
        public string Namespace { get; private set; }
        public string NamespacePath { get; set; }
        public AssemblyEntry Assembly { get; private set; }

        public NamespaceTreeNode(AssemblyEntry assembly, string namespacePath)
            : base(namespacePath)
        {
            Namespace = namespacePath;
            Assembly = assembly;
            ImageIndex = VSImage.Current.Namespace;
            SelectedImageIndex = VSImage.Current.Namespace;
        }

        #region ICoverageInfo Members
        private int codeSize;
        private int coveredCodeSize;

        public int GetCodeSize() { return codeSize; }
        public int GetCoveredCodeSize() { return coveredCodeSize; }

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
            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", Namespace, percent);
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
        public TypedefEntry Typedef { get; private set; }

        public ClassTreeNode(TypedefEntry typedef)
            : base(Types.RemoveNamespacePath(typedef.Name))
        {
            Typedef = typedef;
            ImageIndex = ImageSelector.ForType(Typedef);
            SelectedImageIndex = ImageSelector.ForType(Typedef);
        }

        #region IVisitable Members
        public override void Visit(INodeVisitor visitor)
        {
            visitor.OnType(this);
        }
        #endregion IVisitable Members

        #region ICoverageInfo Members
        private int codeSize;
        private int coveredCodeSize;

        public int GetCodeSize() { return codeSize; }
        public int GetCoveredCodeSize() { return coveredCodeSize; }

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
            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", Types.RemoveNamespacePath(Typedef.Name), percent);
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

        public int GetCodeSize()
        {
            var size = 0;
            if (Setter != null) size += Setter.GetCodeSize();
            if (Getter != null) size += Getter.GetCodeSize();
            return size;
        }

        public int GetCoveredCodeSize()
        {
            var size = 0;
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
        public MethodEntry Method { get; private set; }

        public MethodTreeNode(MethodEntry md)
            : base(md.Name)
        {
            Method = md;
            ImageIndex = ImageSelector.ForMethod(Method);
            SelectedImageIndex = ImageSelector.ForMethod(Method);
        }

        #region ICoverageInfo Members
        private int codeSize;
        private int coveredCodeSize;

        public int GetCodeSize() { return codeSize; }
        public int GetCoveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo()
        {
            foreach (ICoverageInfo iInfo in Nodes)
                iInfo.UpdateCoverageInfo();

            codeSize = 0;
            coveredCodeSize = 0;
            foreach (var block in Method.Blocks)
            {
                codeSize += block.Length;
                coveredCodeSize += block.VisitCount > 0 ? block.Length : 0;
            }

            if (Method.Blocks.Count == 0)
            {
                codeSize = Method.BodySize;
            }


            var percent = codeSize == 0 ? 0 : coveredCodeSize / (float)codeSize * 100;
            Text = string.Format("{0} ({1:#0}%)", Method.Name, percent);
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
}