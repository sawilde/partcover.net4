using System.Collections.Generic;
using PartCover.Browser.Features.Controls;

namespace PartCover.Browser.Stuff
{
    internal abstract class MethodTreeNodeComparer : IComparer<TreeNodeBase>
    {
        private class NodeTypeDetector : INodeVisitor
        {
            public int Level { get; private set; }

            public void OnAssembly(AssemblyTreeNode node) { Level = 0; }
            public void OnNamespace(NamespaceTreeNode node) { Level = 0; }
            public void OnType(ClassTreeNode node) { Level = 0; }
            public void OnProperty(PropertyTreeNode node) { Level = 1; }
            public void OnMethod(MethodTreeNode node) { Level = 2; }

            public void Reset() { Level = 0; }
        }

        public readonly static MethodTreeNodeComparer Default = new MethodTreeNodeComparerKindName();

        private readonly NodeTypeDetector xDetector = new NodeTypeDetector();
        private readonly NodeTypeDetector yDetector = new NodeTypeDetector();

        public int Compare(TreeNodeBase x, TreeNodeBase y)
        {
            xDetector.Reset();
            yDetector.Reset();

            x.Visit(xDetector);
            y.Visit(yDetector);

            switch (xDetector.Level)
            {
                case 1:
                    return yDetector.Level == 1
                        ? CompareMethods((PropertyTreeNode)x, (PropertyTreeNode)y)
                        : CompareMethods((PropertyTreeNode)x, (MethodTreeNode)y);
                case 2:
                    return yDetector.Level == 2
                        ? CompareMethods((MethodTreeNode)x, (MethodTreeNode)y)
                        : CompareMethods((MethodTreeNode)x, (PropertyTreeNode)y);
                default:
                    return 0;
            }
        }

        protected abstract int CompareMethods(MethodTreeNode x, PropertyTreeNode y);
        protected abstract int CompareMethods(MethodTreeNode x, MethodTreeNode y);
        protected abstract int CompareMethods(PropertyTreeNode x, MethodTreeNode y);
        protected abstract int CompareMethods(PropertyTreeNode x, PropertyTreeNode y);

        protected static int CompareByName(TreeNodeBase x, TreeNodeBase y) { return x.Text.CompareTo(y.Text); }
    }

    internal class MethodTreeNodeComparerName : MethodTreeNodeComparer
    {
        protected override int CompareMethods(MethodTreeNode x, PropertyTreeNode y)
        {
            return CompareByName(x, y);
        }

        protected override int CompareMethods(MethodTreeNode x, MethodTreeNode y)
        {
            return CompareByName(x, y);
        }

        protected override int CompareMethods(PropertyTreeNode x, MethodTreeNode y)
        {
            return CompareByName(x, y);
        }

        protected override int CompareMethods(PropertyTreeNode x, PropertyTreeNode y)
        {
            return CompareByName(x, y);
        }
    }

    internal class MethodTreeNodeComparerKindName : MethodTreeNodeComparer
    {
        protected override int CompareMethods(MethodTreeNode x, PropertyTreeNode y)
        {
            return -1;
        }

        protected override int CompareMethods(MethodTreeNode x, MethodTreeNode y)
        {
            return CompareByName(x, y);
        }

        protected override int CompareMethods(PropertyTreeNode x, MethodTreeNode y)
        {
            return 1;
        }

        protected override int CompareMethods(PropertyTreeNode x, PropertyTreeNode y)
        {
            return CompareByName(x, y);
        }
    }
}
