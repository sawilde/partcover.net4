using System;
using System.Globalization;
using System.IO;
using System.Diagnostics;
using System.Xml;
using System.Reflection;
using System.Collections.Generic;


namespace PartCover.Framework.Walkers
{
    /*
    public static class CoverageReportHelper
    {
        public static void AddFile(CoverageReport report, UInt32 id, String url)
        {
            var file = new CoverageReport.FileDescriptor
            {
                fileId = id,
                fileUrl = url
            };
            report.files.Add(file);
        }

        private static CoverageReport.TypeDescriptor FindExistingType(CoverageReport report, CoverageReport.TypeDescriptor dType)
        {
            foreach (var td in report.types)
                if (td.assemblyName == dType.assemblyName && td.typeName == dType.typeName && td.flags == dType.flags)
                    return td;
            return null;
        }

        private static CoverageReport.MethodDescriptor FindExistingMethod(CoverageReport.TypeDescriptor dType, CoverageReport.MethodDescriptor dMethod)
        {
            foreach (var md in dType.methods)
                if (md.methodName == dMethod.methodName && md.methodSig == dMethod.methodSig && md.flags == dMethod.flags && md.implFlags == dMethod.implFlags)
                    return md;
            return null;
        }

        private static CoverageReport.InnerBlockData FindExistingBlockData(IEnumerable<CoverageReport.InnerBlockData> datas, CoverageReport.InnerBlockData bData)
        {
            foreach (var dataBlock in datas)
            {
                if (dataBlock.blocks.Length != bData.blocks.Length)
                    continue;

                var validBlock = true;
                for (var i = 0; validBlock && i < dataBlock.blocks.Length; ++i)
                {
                    var existingBlock = dataBlock.blocks[i];
                    var newBlock = FindBlock(bData.blocks, existingBlock);
                    validBlock = newBlock != null;
                }
                if (validBlock)
                    return dataBlock;
            }
            return null;
        }

        private static CoverageReport.InnerBlock FindBlock(IEnumerable<CoverageReport.InnerBlock> blocks, CoverageReport.InnerBlock block)
        {
            foreach (var dataBlock in blocks)
            {
                if (dataBlock.position != block.position || dataBlock.blockLen != block.blockLen)
                    continue;
                if (dataBlock.fileId == block.fileId &&
                    dataBlock.startLine == block.startLine && dataBlock.startColumn == block.startColumn &&
                    dataBlock.endLine == block.endLine && dataBlock.endColumn == block.endColumn)
                    return dataBlock;
            }
            return null;
        }

        public static void AddType(CoverageReport report, CoverageReport.TypeDescriptor dType)
        {
            var existingType = FindExistingType(report, dType);
            if (existingType == null)
            {
                report.types.Add(dType);
                return;
            }

            foreach (var md in dType.methods)
            {
                var existingMethod = FindExistingMethod(existingType, md);
                if (existingMethod == null)
                {
                    var newMethods = new CoverageReport.MethodDescriptor[existingType.methods.Length + 1];
                    existingType.methods.CopyTo(newMethods, 1);
                    newMethods[0] = md;
                    existingType.methods = newMethods;
                    continue;
                }

                Debug.Assert(md.insBlocks.Length == 1);

                var existingBlockData = FindExistingBlockData(existingMethod.insBlocks, md.insBlocks[0]);

                if (existingBlockData == null)
                {
                    var newBlocks = new CoverageReport.InnerBlockData[existingMethod.insBlocks.Length + 1];
                    existingMethod.insBlocks.CopyTo(newBlocks, 1);
                    newBlocks[0] = md.insBlocks[0];
                    existingMethod.insBlocks = newBlocks;
                }
                else
                {
                    for (var i = 0; i < existingBlockData.blocks.Length; ++i)
                    {
                        var existingBlock = existingBlockData.blocks[i];
                        var newBlock = FindBlock(md.insBlocks[0].blocks, existingBlock);
                        Debug.Assert(newBlock != null);
                        existingBlock.visitCount += newBlock.visitCount;
                    }
                }
            }
        }

        public static void AddMethod(CoverageReport.TypeDescriptor dType, CoverageReport.MethodDescriptor dMethod)
        {
            var newMethods = new CoverageReport.MethodDescriptor[dType.methods.Length + 1];
            dType.methods.CopyTo(newMethods, 1);

            newMethods[0] = dMethod;

            dType.methods = newMethods;
        }

        public static void AddMethodBlock(CoverageReport.MethodDescriptor dMethod, CoverageReport.InnerBlock inner)
        {
            var bData = dMethod.insBlocks[0];

            var newBlocks = new CoverageReport.InnerBlock[bData.blocks.Length + 1];
            bData.blocks.CopyTo(newBlocks, 1);

            newBlocks[0] = inner;

            bData.blocks = newBlocks;
        }

        public static void AddBlock(CoverageReport.InnerBlockData bData, CoverageReport.InnerBlock inner)
        {
            var newBlocks = new CoverageReport.InnerBlock[bData.blocks.Length + 1];
            bData.blocks.CopyTo(newBlocks, 1);
            newBlocks[0] = inner;
            bData.blocks = newBlocks;
        }

        public static void AddBlockData(CoverageReport.MethodDescriptor dMethod, CoverageReport.InnerBlockData bData)
        {
            var newBlocks = new CoverageReport.InnerBlockData[dMethod.insBlocks.Length + 1];
            dMethod.insBlocks.CopyTo(newBlocks, 1);
            newBlocks[0] = bData;
            dMethod.insBlocks = newBlocks;
        }

        public static string[] GetAssemblies(CoverageReport report)
        {
            var list = new Dictionary<string, bool>();
            foreach (var dType in report.types)
                list[dType.assemblyName] = true;

            var listInst = new List<string>(list.Keys);
            listInst.Sort();
            return listInst.ToArray();
        }

        public static ICollection<CoverageReport.TypeDescriptor> GetTypes(CoverageReport report, string assembly)
        {
            var res = new List<CoverageReport.TypeDescriptor>();
            foreach (var dType in report.types)
                if (dType.assemblyName == assembly)
                    res.Add(dType);
            return res;
        }

        public static string[] SplitNamespaces(string typedefName)
        {
            return typedefName.Split('.');
        }

        public static string GetTypeDefName(string typedefName)
        {
            string[] names = SplitNamespaces(typedefName);
            return names[names.Length - 1];
        }

        public static string GetFileUrl(CoverageReport report, UInt32 fileId)
        {
            foreach (CoverageReport.FileDescriptor fd in report.files)
                if (fd.fileId == fileId) return fd.fileUrl;
            return null;
        }

        public static UInt32 GetBlockCodeSize(CoverageReport.InnerBlockData bData)
        {
            return GetBlockCodeSize(bData.blocks);
        }

        public static UInt32 GetBlockCodeSize(IEnumerable<CoverageReport.InnerBlock> blocks)
        {
            UInt32 codeSize = 0;
            foreach (CoverageReport.InnerBlock ib in blocks)
                codeSize += ib.blockLen;
            return codeSize;
        }

        public static UInt32 GetBlockCoveredCodeSize(CoverageReport.InnerBlockData bData)
        {
            return GetBlockCoveredCodeSize(bData.blocks);
        }

        public static UInt32 GetBlockCoveredCodeSize(IEnumerable<CoverageReport.InnerBlock> blocks)
        {
            UInt32 codeSize = 0;
            foreach (CoverageReport.InnerBlock ib in blocks)
                if (ib.visitCount > 0) codeSize += ib.blockLen;
            return codeSize;
        }

        public static float GetBlockCoverage(CoverageReport.InnerBlock ib)
        {
            return ib.visitCount > 0 ? 100 : 0;
        }

        #region Save/Load

        private static Assembly GetHelperAssembly()
        {
            return Assembly.GetAssembly(typeof(CoverageReportHelper));
        }

        public static string VersionString(Version version)
        {
            return string.Format("{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision);
        }

        public static bool IsEqual(Version version, string versionString)
        {
            return versionString != null && VersionString(version) == versionString;
        }

        public static void WriteReport(CoverageReport report, TextWriter writer)
        {
            var xml = new XmlDocument();

            var root = xml.AppendChild(xml.CreateElement("PartCoverReport"));
            root.Attributes.Append(xml.CreateAttribute("ver")).Value = VersionString(GetHelperAssembly().GetName().Version);

            if (report.ExitCode.HasValue)
                root.Attributes.Append(xml.CreateAttribute("exitCode")).Value = report.ExitCode.Value.ToString(CultureInfo.InvariantCulture);

            foreach (var dFile in report.files)
            {
                var fileNode = root.AppendChild(xml.CreateElement("file"));
                fileNode.Attributes.Append(xml.CreateAttribute("id")).Value = dFile.fileId.ToString(CultureInfo.InvariantCulture);
                fileNode.Attributes.Append(xml.CreateAttribute("url")).Value = dFile.fileUrl;
            }

            foreach (var dType in report.types)
            {
                var typeNode = root.AppendChild(xml.CreateElement("type"));
                typeNode.Attributes.Append(xml.CreateAttribute("asm")).Value = dType.assemblyName;
                typeNode.Attributes.Append(xml.CreateAttribute("name")).Value = dType.typeName;
                typeNode.Attributes.Append(xml.CreateAttribute("flags")).Value = dType.flags.ToString(CultureInfo.InvariantCulture);

                foreach (CoverageReport.MethodDescriptor dMethod in dType.methods)
                {
                    XmlNode methodNode = typeNode.AppendChild(xml.CreateElement("method"));
                    methodNode.Attributes.Append(xml.CreateAttribute("name")).Value = dMethod.methodName;
                    methodNode.Attributes.Append(xml.CreateAttribute("sig")).Value = dMethod.methodSig;
                    methodNode.Attributes.Append(xml.CreateAttribute("flags")).Value = dMethod.flags.ToString(CultureInfo.InvariantCulture);
                    methodNode.Attributes.Append(xml.CreateAttribute("iflags")).Value = dMethod.implFlags.ToString(CultureInfo.InvariantCulture);

                    foreach (CoverageReport.InnerBlockData bData in dMethod.insBlocks)
                    {
                        XmlNode codeNode = methodNode.AppendChild(xml.CreateElement("code"));
                        foreach (CoverageReport.InnerBlock inner in bData.blocks)
                        {
                            XmlNode point = codeNode.AppendChild(xml.CreateElement("pt"));
                            point.Attributes.Append(xml.CreateAttribute("visit")).Value = inner.visitCount.ToString(CultureInfo.InvariantCulture);
                            point.Attributes.Append(xml.CreateAttribute("pos")).Value = inner.position.ToString(CultureInfo.InvariantCulture);
                            point.Attributes.Append(xml.CreateAttribute("len")).Value = inner.blockLen.ToString(CultureInfo.InvariantCulture);
                            if (inner.fileId != 0)
                            {
                                point.Attributes.Append(xml.CreateAttribute("fid")).Value = inner.fileId.ToString(CultureInfo.InvariantCulture);
                                point.Attributes.Append(xml.CreateAttribute("sl")).Value = inner.startLine.ToString(CultureInfo.InvariantCulture);
                                point.Attributes.Append(xml.CreateAttribute("sc")).Value = inner.startColumn.ToString(CultureInfo.InvariantCulture);
                                point.Attributes.Append(xml.CreateAttribute("el")).Value = inner.endLine.ToString(CultureInfo.InvariantCulture);
                                point.Attributes.Append(xml.CreateAttribute("ec")).Value = inner.endColumn.ToString(CultureInfo.InvariantCulture);
                            }
                        }
                    }
                }
            }

            XmlNode runNode = root.AppendChild(xml.CreateElement("run"));

            XmlNode runTrackerNode = runNode.AppendChild(xml.CreateElement("tracker"));
            foreach (CoverageReport.RunHistoryMessage rhm in report.runHistory)
            {
                XmlNode messageNode = runTrackerNode.AppendChild(xml.CreateElement("message"));
                messageNode.Attributes.Append(xml.CreateAttribute("tm")).Value = rhm.Time.ToUniversalTime().Ticks.ToString(CultureInfo.InvariantCulture);
                messageNode.InnerText = rhm.Message;
            }

            XmlNode runLogNode = runNode.AppendChild(xml.CreateElement("log"));
            foreach (CoverageReport.RunLogMessage rlm in report.runLog)
            {
                XmlNode messageNode = runLogNode.AppendChild(xml.CreateElement("message"));
                messageNode.Attributes.Append(xml.CreateAttribute("tr")).Value = rlm.ThreadId.ToString(CultureInfo.InvariantCulture);
                messageNode.Attributes.Append(xml.CreateAttribute("ms")).Value = rlm.MsOffset.ToString(CultureInfo.InvariantCulture);
                messageNode.InnerText = rlm.Message;
            }

            xml.Save(writer);
        }

        private static UInt32 GetUInt32Attribute(XmlNode node, string attr)
        {
            string strAttr = GetStringAttribute(node, attr);
            try
            {
                return UInt32.Parse(strAttr);
            }
            catch { throw new ReportException("Wrong report format, UInt32 type expected at " + node.Name + "[@" + attr + "]"); }
        }

        private static long GetLongAttribute(XmlNode node, string attr)
        {
            string strAttr = GetStringAttribute(node, attr);
            try
            {
                return long.Parse(strAttr);
            }
            catch { throw new ReportException("Wrong report format, long type expected at " + node.Name + "[@" + attr + "]"); }
        }

        private static int GetIntAttribute(XmlNode node, string attr)
        {
            string strAttr = GetStringAttribute(node, attr);
            try
            {
                return int.Parse(strAttr);
            }
            catch { throw new ReportException("Wrong report format, int type expected at " + node.Name + "[@" + attr + "]"); }
        }

        private static string GetStringAttribute(XmlNode node, string attr)
        {
            XmlAttribute attrNode = node.Attributes[attr];
            if (attrNode == null || attrNode.Value == null) throw new ReportException("Wrong report format");
            return attrNode.Value;
        }

        public static void ReadReport(CoverageReport report, TextReader reader)
        {
            var xml = new XmlDocument();
            xml.Load(reader);

            var root = xml.SelectSingleNode("/PartCoverReport");
            if (root == null) throw new ReportException("Wrong report format");
            var verAttribute = root.Attributes["ver"];
            if (verAttribute == null) throw new ReportException("Wrong report format");
            var exitCodeAttribute = root.Attributes["exitCode"];
            if (exitCodeAttribute != null) report.ExitCode = GetIntAttribute(root, exitCodeAttribute.Name);

            foreach (XmlNode fileNode in xml.SelectNodes("/PartCoverReport/file"))
                AddFile(report, GetUInt32Attribute(fileNode, "id"), GetStringAttribute(fileNode, "url"));

            foreach (XmlNode typeNode in xml.SelectNodes("/PartCoverReport/type"))
            {
                var dType = new CoverageReport.TypeDescriptor
                {
                    assemblyName = GetStringAttribute(typeNode, "asm"),
                    typeName = GetStringAttribute(typeNode, "name"),
                    flags = GetUInt32Attribute(typeNode, "flags")
                };

                foreach (XmlNode methodNode in typeNode.SelectNodes("method"))
                {
                    var dMethod = new CoverageReport.MethodDescriptor(0)
                    {
                        methodName = GetStringAttribute(methodNode, "name"),
                        methodSig = GetStringAttribute(methodNode, "sig"),
                        flags = GetUInt32Attribute(methodNode, "flags"),
                        implFlags = GetUInt32Attribute(methodNode, "iflags")
                    };

                    foreach (XmlNode blockNode in methodNode.SelectNodes("code"))
                    {
                        var dBlock = new CoverageReport.InnerBlockData();
                        foreach (XmlNode pointNode in blockNode.SelectNodes("pt"))
                        {
                            var dPoint = new CoverageReport.InnerBlock
                            {
                                visitCount = GetUInt32Attribute(pointNode, "visit"),
                                position = GetUInt32Attribute(pointNode, "pos"),
                                blockLen = GetUInt32Attribute(pointNode, "len")
                            };

                            if (pointNode.Attributes["fid"] != null)
                            {
                                dPoint.fileId = GetUInt32Attribute(pointNode, "fid");
                                dPoint.startLine = GetUInt32Attribute(pointNode, "sl");
                                dPoint.startColumn = GetUInt32Attribute(pointNode, "sc");
                                dPoint.endLine = GetUInt32Attribute(pointNode, "el");
                                dPoint.endColumn = GetUInt32Attribute(pointNode, "ec");
                            }
                            AddBlock(dBlock, dPoint);
                        }
                        AddBlockData(dMethod, dBlock);
                    }
                    AddMethod(dType, dMethod);
                }
                AddType(report, dType);
            }

            foreach (XmlNode messageNode in xml.SelectNodes("/PartCoverReport/run/tracker/message"))
                AddTrackerMessage(report, messageNode);

            foreach (XmlNode messageNode in xml.SelectNodes("/PartCoverReport/run/log/message"))
                AddLogFileMessage(report, messageNode);
        }

        private static void AddLogFileMessage(CoverageReport report, XmlNode messageNode)
        {
            var message = new CoverageReport.RunLogMessage
            {
                ThreadId = GetIntAttribute(messageNode, "tr"),
                MsOffset = GetIntAttribute(messageNode, "ms"),
                Message = messageNode.InnerText
            };
            report.runLog.Add(message);
        }

        private static void AddTrackerMessage(CoverageReport report, XmlNode messageNode)
        {
            var message = new CoverageReport.RunHistoryMessage
            {
                Time = new DateTime(GetLongAttribute(messageNode, "tm"), DateTimeKind.Utc),
                Message = messageNode.InnerText
            };
            report.runHistory.Add(message);
        }

        #endregion //Save/Load
    }
    */
}
