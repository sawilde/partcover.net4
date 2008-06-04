using System;
using System.Collections;
using System.Globalization;
using System.IO;
using System.Xml;

namespace PartCover.Framework.Walkers
{
	public sealed class CoverageReport
	{
        public struct FileDescriptor {
            public UInt32 fileId;
            public string fileUrl;
        }

        public class TypeDescriptor {
            public string assemblyName;
            public string typeName;
            public UInt32 flags;

            public MethodDescriptor[] methods = new MethodDescriptor[0];
        }

        public class MethodDescriptor {
            public string methodName;
            public string methodSig;
            public UInt32 flags;
            public UInt32 implFlags;

            public InnerBlockData[] insBlocks;

            public UInt32 GetCodeSize(int blockIndex) {
                UInt32 res = 0;
                foreach(InnerBlock inner in insBlocks[blockIndex].blocks) 
                    res += inner.blockLen;
                return res;
            }

            public UInt32 GetCoveredCodeSize(int blockIndex) {
                UInt32 res = 0;
                foreach(InnerBlock inner in insBlocks[blockIndex].blocks) 
                    if (inner.visitCount > 0) res += inner.blockLen;
                return res;
            }

            public MethodDescriptor(int initialBlockSize) { SetBlockDataSize(initialBlockSize); }
            public MethodDescriptor() { SetBlockDataSize(0); }

            private void SetBlockDataSize(int initialBlockSize) {
                insBlocks = new InnerBlockData[initialBlockSize];
                while(initialBlockSize-- > 0)
                    insBlocks[initialBlockSize] = new InnerBlockData();
            }
        }

        public class InnerBlock {
            public UInt32 position;
            public UInt32 blockLen;
            public UInt32 visitCount;
            public UInt32 fileId;
            public UInt32 startLine;
            public UInt32 startColumn;
            public UInt32 endLine;
            public UInt32 endColumn;
        }

        public class InnerBlockData {
            public InnerBlock[] blocks = new InnerBlock[0];
        }

        public TypeDescriptor[] types = new TypeDescriptor[0];
        public FileDescriptor[] files = new FileDescriptor[0];

        #region Save 
        /*

        public void Save(TextWriter writer) {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.AppendChild(xmlDoc.CreateElement("Report"));

            foreach(FileDescriptor f in files) {
                XmlNode fileNode = xmlDoc.DocumentElement.AppendChild(xmlDoc.CreateElement("File"));
                fileNode.Attributes.Append(xmlDoc.CreateAttribute("id")).Value = f.fileId.ToString();
                fileNode.Attributes.Append(xmlDoc.CreateAttribute("url")).Value = f.fileUrl;
            }

            foreach(MethodDescriptor m in methods) {
                XmlNode asmNode = GetAssemblyNode(xmlDoc.DocumentElement, m.assemblyName);
                XmlNode classNode = GetClassNode(asmNode, m.className);
                XmlNode methodNode = GetMethodNode(classNode, m);
                foreach(InnerBlockData inner in m.insBlocks) {
                    XmlNode mblockNode = GetMethodBlockNode(methodNode, inner.module);
                    foreach(InnerBlock block in inner.blocks) {
                        AddMethodBlockNode(mblockNode, block);
                    }
                }
                methodNode.Attributes.Append(methodNode.OwnerDocument.CreateAttribute("cover")).Value = m.coveragePercent.ToString("#0.##", CultureInfo.InvariantCulture);
            }

            xmlDoc.Save(writer);
        }

        private XmlNode GetAssemblyNode(XmlNode parent, string assemblyName) {
            XmlNode res = parent.SelectSingleNode("Assembly[@name='" + assemblyName + "']");
            if (res == null) {
                res = parent.AppendChild(parent.OwnerDocument.CreateElement("Assembly"));
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("name")).Value = assemblyName;
            }
            return res;
        }

        private XmlNode GetClassNode(XmlNode parent, string className) {
            XmlNode res = parent.SelectSingleNode("Class[@name='" + className + "']");
            if (res == null) {
                res = parent.AppendChild(parent.OwnerDocument.CreateElement("Class"));
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("name")).Value = className;
            }
            return res;
        }

        private XmlNode GetMethodNode(XmlNode parent, MethodDescriptor md) {
            XmlNode res = parent.SelectSingleNode("Method[@name='" + md.methodName + "' and @sig='" + md.methodSig + "']");
            if (res == null) {
                res = parent.AppendChild(parent.OwnerDocument.CreateElement("Method"));
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("name")).Value = md.methodName;
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("sig")).Value = md.methodSig;
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("fl")).Value = md.flags.ToString();
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("ifl")).Value = md.implFlags.ToString();
            }
            return res;
        }

        private XmlNode GetMethodBlockNode(XmlNode parent, string moduleName) {
            int moduleId = GetModuleId( parent.OwnerDocument.DocumentElement, moduleName );
            XmlNode res = parent.SelectSingleNode("Variant[@module='" + moduleId + "']");
            if (res == null) {
                res = parent.AppendChild(parent.OwnerDocument.CreateElement("Variant"));
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("module")).Value = moduleId.ToString();
            }
            return res;
        }

        private int GetModuleId(XmlNode doc, string moduleName) {
            int result;
            XmlNode res = doc.SelectSingleNode("Module[@path='" + moduleName + "']");
            if (res != null) {
                result = int.Parse(res.Attributes["id"].Value);
            } else {
                result = doc.SelectNodes("Module").Count + 1;
                res = doc.AppendChild(doc.OwnerDocument.CreateElement("Module"));
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("path")).Value = moduleName;
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("id")).Value = result.ToString();
            }
            return result;
        }

        private void AddMethodBlockNode(XmlNode parent, InnerBlock block) {
            XmlNode res = parent.AppendChild(parent.OwnerDocument.CreateElement("Pt"));
            res.Attributes.Append(res.OwnerDocument.CreateAttribute("visit")).Value = block.visitCount.ToString();
            res.Attributes.Append(res.OwnerDocument.CreateAttribute("offset")).Value = block.position.ToString();
            res.Attributes.Append(res.OwnerDocument.CreateAttribute("length")).Value = block.blockLen.ToString();

            if (block.fileId != 0) {
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("fid")).Value = block.fileId.ToString();
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("sl")).Value = block.startLine.ToString();
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("sc")).Value = block.startColumn.ToString();
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("el")).Value = block.endLine.ToString();
                res.Attributes.Append(res.OwnerDocument.CreateAttribute("ec")).Value = block.endColumn.ToString();
            }
        }
*/

        #endregion

        #region Load
/*
        public void Load(string fileName) {
            XmlDocument xml = new XmlDocument();
            xml.Load(fileName);

            foreach(XmlNode fileNode in xml.SelectNodes("/Report/File")) {
                FileDescriptor file = new FileDescriptor();
                file.fileId = UInt32.Parse(fileNode.Attributes["id"].Value);
                file.fileUrl = fileNode.Attributes["url"].Value;
                files.Add(file);
            }

            foreach(XmlNode mNode in xml.SelectNodes("/Report/Assembly/Class/Method")) {
                MethodDescriptor metDescr = new MethodDescriptor();
                metDescr.assemblyName = mNode.ParentNode.ParentNode.Attributes["name"].Value;
                metDescr.className = mNode.ParentNode.Attributes["name"].Value;
                metDescr.methodName = mNode.Attributes["name"].Value;
                metDescr.methodSig = mNode.Attributes["sig"].Value;
                metDescr.flags = UInt32.Parse(mNode.Attributes["fl"].Value);
                metDescr.implFlags = UInt32.Parse(mNode.Attributes["ifl"].Value);
                metDescr.coveragePercent = float.Parse(mNode.Attributes["cover"].Value, CultureInfo.InvariantCulture);

                ArrayList blocks = new ArrayList();
                foreach(XmlNode bNode in mNode.SelectNodes("./Variant")) {
                    InnerBlockData bData = new InnerBlockData();
                    bData.module = xml.SelectSingleNode(string.Format("/Report/Module[@id={0}]", bNode.Attributes["module"].Value)).Attributes["path"].Value;

                    ArrayList innerBlocks = new ArrayList();
                    foreach(XmlNode ibNode in bNode.SelectNodes("./Pt")) {
                        InnerBlock iBlock = new InnerBlock();
                        if (ibNode.Attributes["fid"] != null) {
                            iBlock.fileId = UInt32.Parse(ibNode.Attributes["fid"].Value);
                            iBlock.startLine = UInt32.Parse(ibNode.Attributes["sl"].Value);
                            iBlock.endLine = UInt32.Parse(ibNode.Attributes["el"].Value);
                            iBlock.startColumn = UInt32.Parse(ibNode.Attributes["sc"].Value);
                            iBlock.endColumn = UInt32.Parse(ibNode.Attributes["ec"].Value);
                        }

                        iBlock.position = UInt32.Parse(ibNode.Attributes["offset"].Value);
                        iBlock.blockLen = UInt32.Parse(ibNode.Attributes["length"].Value);
                        iBlock.visitCount = UInt32.Parse(ibNode.Attributes["visit"].Value);

                        innerBlocks.Add(iBlock);
                    }

                    bData.blocks = (InnerBlock[]) innerBlocks.ToArray(typeof(InnerBlock));

                    blocks.Add(bData);
                }

                metDescr.insBlocks = (InnerBlockData[]) blocks.ToArray(typeof(InnerBlockData));

                methods.Add(metDescr);
            }
        }
*/
        #endregion
    }
}
