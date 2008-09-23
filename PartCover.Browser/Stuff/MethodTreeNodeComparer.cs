using System.Collections.Generic;
using PartCover.Browser.Controls;

namespace PartCover.Browser.Stuff
{
    internal abstract class MethodTreeNodeComparer : IComparer<TreeNodeBase>
    {
        private class NodeTypeDetector : INodeVisitor
        {
            int level;
            public int Level { get { return level; } }

            public void forAssembly(AssemblyTreeNode node) { level = 0; }
            public void forNamespace(NamespaceTreeNode node) { level = 0; }
            public void forType(ClassTreeNode node) { level = 0; }
            public void forProperty(PropertyTreeNode node) { level = 1; }
            public void forMethod(MethodTreeNode node) { level = 2; }
            public void forBlockVariant(BlockVariantTreeNode node) { level = 0; }

            public void reset() { level = 0; }
        }

        public readonly static MethodTreeNodeComparer Default = new MethodTreeNodeComparerKindName();

        private readonly NodeTypeDetector xDetector = new NodeTypeDetector();
        private readonly NodeTypeDetector yDetector = new NodeTypeDetector();

        public int Compare(TreeNodeBase x, TreeNodeBase y)
        {
            xDetector.reset();
            yDetector.reset();

            x.visit(xDetector);
            y.visit(yDetector);

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
