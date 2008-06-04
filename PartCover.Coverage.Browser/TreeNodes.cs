using System;
using System.Windows.Forms;
using System.Drawing;
using System.Reflection;

using PartCover.Framework.Walkers;

namespace PartCover.Coverage.Browser
{
    struct NodeImageIndex {
        public const int InternalClass = 0;
        public const int InternalConstant = 1;
        public const int InternalDelegate = 2;
        public const int InternalEnumeration = 3;
        public const int InternalEvent = 4;
        public const int InternalField = 5;
        public const int InternalInterface = 6;
        public const int InternalMethod = 7;
        public const int InternalProperty = 8;
        public const int InternalStructure = 9;
        public const int Namespace = 10;
        public const int Operator = 11;
        public const int PrivateConstant = 12;
        public const int PrivateEvent = 13;
        public const int PrivateField = 14;
        public const int PrivateMethod = 15;
        public const int PrivateProperty = 16;
        public const int ProtectedConstant = 17;
        public const int ProtectedEvent = 18;
        public const int ProtectedField = 19;
        public const int ProtectedMethod = 20;
        public const int ProtectedProperty = 21;
        public const int PublicClass = 22;
        public const int PublicConstant = 23;
        public const int PublicDelegate = 24;
        public const int PublicEnumeration = 25;
        public const int PublicEvent = 26;
        public const int PublicField = 27;
        public const int PublicInterface = 28;
        public const int PublicMethod = 29;
        public const int PublicProperty = 30;
        public const int PublicStructure = 31;
        public const int AssemblyImage = 32;
    }

    interface ICoverageInfo {
        UInt32 GetCodeSize();
        UInt32 GetCOveredCodeSize();
        void UpdateCoverageInfo();
    }

    internal class TreeNodeBase : TreeNode {
        public TreeNodeBase(string name, int imageIndex, int selectedImageIndex) : base(name, imageIndex, selectedImageIndex) {}
        public TreeNodeBase(string name) : base(name) {}
    }

    internal class AssemblyTreeNode : TreeNodeBase, ICoverageInfo
	{
        private string assemblyName;

		public AssemblyTreeNode(string assemblyName) : base(assemblyName, NodeImageIndex.AssemblyImage, NodeImageIndex.AssemblyImage) {
            this.assemblyName = assemblyName;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCOveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo() {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach(ICoverageInfo iInfo in Nodes) {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCOveredCodeSize();
            }
            float percent = codeSize == 0? 0 : coveredCodeSize / (float) codeSize * 100;
            this.Text = string.Format("{0} ({1:#0}%)", assemblyName, percent);
            this.ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class NamespaceTreeNode : TreeNodeBase, ICoverageInfo {
        string namespaceName;
        public string Namespace {
            get { return namespaceName; }
        }

        public NamespaceTreeNode(string namespaceName) : base(namespaceName, NodeImageIndex.Namespace, NodeImageIndex.Namespace) {
            this.namespaceName = namespaceName;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCOveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo() {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach(ICoverageInfo iInfo in Nodes) {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCOveredCodeSize();
            }
            float percent = codeSize == 0? 0 : coveredCodeSize / (float) codeSize * 100;
            this.Text = string.Format("{0} ({1:#0}%)", Namespace, percent);
            this.ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class ClassTreeNode : TreeNodeBase, ICoverageInfo {
        CoverageReport.TypeDescriptor dType;
        public CoverageReport.TypeDescriptor Descriptor {
            get { return dType; }
        }

        public ClassTreeNode(CoverageReport.TypeDescriptor dType) : base(CoverageReportHelper.GetTypeDefName(dType.typeName)) {
            ImageIndex = NodeImageIndex.PublicStructure;
            SelectedImageIndex = ImageIndex;
            this.dType = dType;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCOveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo() {
            codeSize = 0;
            coveredCodeSize = 0;

            foreach(ICoverageInfo iInfo in Nodes) {
                iInfo.UpdateCoverageInfo();
                codeSize += iInfo.GetCodeSize();
                coveredCodeSize += iInfo.GetCOveredCodeSize();
            }
            float percent = codeSize == 0? 0 : coveredCodeSize / (float) codeSize * 100;
            this.Text = string.Format("{0} ({1:#0}%)", CoverageReportHelper.GetTypeDefName(dType.typeName), percent);
            this.ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class MethodTreeNode : TreeNodeBase, ICoverageInfo {
        CoverageReport.MethodDescriptor md;
        public CoverageReport.MethodDescriptor Descriptor {
            get { return md; }
        }

        public MethodTreeNode(CoverageReport.MethodDescriptor md) : base(md.methodName) {
            this.md = md;
            ImageIndex = NodeImageIndex.PublicMethod;
            SelectedImageIndex = ImageIndex;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCOveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo() {
            foreach(ICoverageInfo iInfo in Nodes)
                iInfo.UpdateCoverageInfo();

            codeSize = 0;
            coveredCodeSize = 0;
            foreach(CoverageReport.InnerBlockData bData in md.insBlocks) {
                codeSize = CoverageReportHelper.GetBlockCodeSize(bData);
                coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(bData);
            }
            float percent = codeSize == 0? 0 : coveredCodeSize / (float) codeSize * 100;
            this.Text = string.Format("{0} ({1:#0}%)", md.methodName, percent);
            this.ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }

    internal class BlockVariantTreeNode : TreeNodeBase {
        CoverageReport.InnerBlockData bData;
        public CoverageReport.InnerBlockData BlockData {
            get { return bData; }
        }

        public BlockVariantTreeNode(CoverageReport.InnerBlockData bData) : base("Block Data") {
            this.bData = bData;
            ImageIndex = NodeImageIndex.Namespace;
            SelectedImageIndex = ImageIndex;
        }

        #region ICoverageInfo Members
        private UInt32 codeSize = 0;
        private UInt32 coveredCodeSize = 0;

        public UInt32 GetCodeSize() { return codeSize; }
        public UInt32 GetCOveredCodeSize() { return coveredCodeSize; }

        public void UpdateCoverageInfo() {
            codeSize = CoverageReportHelper.GetBlockCodeSize(BlockData);
            coveredCodeSize = CoverageReportHelper.GetBlockCoveredCodeSize(BlockData);

            float percent = codeSize == 0? 0 : coveredCodeSize / (float) codeSize * 100;
            this.Text = string.Format("Block Data ({1:#0}%)", percent);
            this.ForeColor = Helpers.ColorProvider.GetForeColorForPercent(percent);
        }

        #endregion
    }
}
