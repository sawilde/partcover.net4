using System;
using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using System.Xml;
using PartCover.Framework.Data;

namespace PartCover.Framework
{

    public class ReportSerializer
    {
        
        public static void Save(XmlTextWriter writer, Report report)
        {
            var xmlDoc = new XmlDocument();
            var el = xmlDoc.CreateElement("PartCoverReport");
            var at = xmlDoc.CreateAttribute("version");
            at.Value = "4.0";
            el.Attributes.Append(at);
            var doc = (XmlElement)xmlDoc.AppendChild(el);
            Save(doc, report);
            doc.WriteTo(writer);
        }

        public static bool Load(XmlTextReader reader, Report report)
        {
            var xmlDoc = new XmlDocument();
            XmlNode el;
            while ((el = xmlDoc.ReadNode(reader)) != null)
            {
                if (el.NodeType != XmlNodeType.Element)
                    continue;
                Load((XmlElement)el, report);
                return true;
            }
            return false;
        }

        public static void Save(XmlElement node, Report report)
        {
            AddAttribute(node, "date").Value = report.Date.ToString("O", CultureInfo.InvariantCulture);

            foreach (var dFile in report.Files)
            {
                var fileNode = AddElement(node, "File");
                AddAttribute(fileNode, "id").Value = dFile.Id.ToString(CultureInfo.InvariantCulture);
                AddAttribute(fileNode, "url").Value = dFile.PathUri;
            }

            var typeList = new List<TypedefEntry>();

            new List<AssemblyEntry>(report.Assemblies).ForEach(x =>
            {
                var asmNode = AddElement(node, "Assembly");
                AddAttribute(asmNode, "id").Value = x.AssemblyRef.ToString(CultureInfo.InvariantCulture);
                AddAttribute(asmNode, "name").Value = x.Name;
                AddAttribute(asmNode, "module").Value = x.Module;
                AddAttribute(asmNode, "domain").Value = x.Domain;
                AddAttribute(asmNode, "domainIdx").Value = x.DomainIndex.ToString(CultureInfo.InvariantCulture);

                typeList.AddRange(x.Types);
            });

            typeList.ForEach(x =>
            {
                var typeNode = AddElement(node, "Type");
                AddAttribute(typeNode, "asmref").Value = x.Assembly.AssemblyRef.ToString(CultureInfo.InvariantCulture);
                AddAttribute(typeNode, "name").Value = x.Name;
                AddAttribute(typeNode, "flags").Value = ((long)x.Attributes).ToString(CultureInfo.InvariantCulture);

                new List<MethodEntry>(x.Methods).ForEach(m =>
                {
                    var metNode = AddElement(typeNode, "Method");
                    AddAttribute(metNode, "methoddef").Value = m.MethodDef.ToString(CultureInfo.InvariantCulture);
                    AddAttribute(metNode, "name").Value = m.Name;
                    AddAttribute(metNode, "sig").Value = m.Signature;
                    AddAttribute(metNode, "bodysize").Value = m.BodySize.ToString(CultureInfo.InvariantCulture);
                    AddAttribute(metNode, "flags").Value = ((long)m.Flags).ToString(CultureInfo.InvariantCulture);
                    AddAttribute(metNode, "iflags").Value = ((long)m.ImplFlags).ToString(CultureInfo.InvariantCulture);

                    new List<MethodBlock>(m.Blocks).ForEach(b =>
                    {
                        var blockNode = AddElement(metNode, "pt");
                        AddAttribute(blockNode, "visit").Value = b.VisitCount.ToString(CultureInfo.InvariantCulture);
                        AddAttribute(blockNode, "pos").Value = b.Offset.ToString(CultureInfo.InvariantCulture);
                        AddAttribute(blockNode, "len").Value = b.Length.ToString(CultureInfo.InvariantCulture);

                        if (b.File > 0)
                        {
                            AddAttribute(blockNode, "fid").Value = b.File.ToString(CultureInfo.InvariantCulture);
                            AddAttribute(blockNode, "sl").Value = b.Start.Line.ToString(CultureInfo.InvariantCulture);
                            AddAttribute(blockNode, "sc").Value = b.Start.Column.ToString(CultureInfo.InvariantCulture);
                            AddAttribute(blockNode, "el").Value = b.End.Line.ToString(CultureInfo.InvariantCulture);
                            AddAttribute(blockNode, "ec").Value = b.End.Column.ToString(CultureInfo.InvariantCulture);
                        }
                    });
                });
            });
        }

        public static void Load(XmlElement node, Report report)
        {
            report.Date = ReadAttributeDate(node, "date");

            foreach (var dFile in SelectChildNodes(node, "file"))
            {
                report.Files.Add(new FileEntry
                {
                    Id = ReadAttributeInt(dFile, "id"),
                    PathUri = ReadAttribute(dFile, "url")
                });
            }

            foreach (var asmNode in SelectChildNodes(node, "assembly"))
            {
                report.Assemblies.Add(new AssemblyEntry
                {
                    AssemblyRef = ReadAttributeInt(asmNode, "id"),
                    Name = ReadAttribute(asmNode, "name"),
                    Module = ReadAttribute(asmNode, "module"),
                    Domain = ReadAttribute(asmNode, "domain"),
                    DomainIndex = ReadAttributeInt(asmNode, "domainIdx")
                });
            }

            foreach (var typeNode in SelectChildNodes(node, "type"))
            {
                var assmRef = ReadAttributeInt(typeNode, "asmref");
                var assemblyEntry = report.Assemblies.Find(x => x.AssemblyRef == assmRef);
                if (assemblyEntry == null)
                {
                    continue;
                }

                var typedefEntry = new TypedefEntry
                {
                    Name = ReadAttribute(typeNode, "name"),
                    Attributes = (TypeAttributes)ReadAttributeLong(typeNode, "flags"),
                    Assembly = assemblyEntry
                };
                assemblyEntry.Types.Add(typedefEntry);

                foreach (var methodNode in SelectChildNodes(typeNode, "method"))
                {
                    var methodEntry = new MethodEntry
                    {
                        Name = ReadAttribute(methodNode, "name"),
                        Signature = ReadAttribute(methodNode, "sig"),
                        BodySize = ReadAttributeInt(methodNode, "bodysize"),
                        Flags = (MethodAttributes)ReadAttributeLong(methodNode, "flags"),
                        ImplFlags = (MethodImplAttributes)ReadAttributeLong(methodNode, "iflags")
                    };
                    typedefEntry.Methods.Add(methodEntry);


                    foreach (var pointNode in SelectChildNodes(methodNode, "pt"))
                    {
                        methodEntry.Blocks.Add(new MethodBlock
                        {
                            VisitCount = ReadAttributeInt(pointNode, "visit"),
                            Offset = ReadAttributeInt(pointNode, "pos"),
                            Length = ReadAttributeInt(pointNode, "len"),
                            File = ReadAttributeInt(pointNode, "fid"),
                            Start = new Position
                            {
                                Column = ReadAttributeInt(pointNode, "sc"),
                                Line = ReadAttributeInt(pointNode, "sl")
                            },
                            End = new Position
                            {
                                Column = ReadAttributeInt(pointNode, "ec"),
                                Line = ReadAttributeInt(pointNode, "el")
                            }
                        });
                    }
                }
            }
        }

        private static IEnumerable<XmlElement> SelectChildNodes(XmlNode node, string childName)
        {
            foreach (XmlNode child in node.ChildNodes)
            {
                if (child.NodeType != XmlNodeType.Element)
                    continue;

                if (child.LocalName.ToLowerInvariant() == childName.ToLowerInvariant())
                    yield return (XmlElement)child;
            }
        }

        private static XmlAttribute AddAttribute(XmlNode node, string attrName)
        {
            return node.Attributes.Append(node.OwnerDocument.CreateAttribute(attrName));
        }

        private static XmlNode AddElement(XmlNode node, string nodeName)
        {
            return node.AppendChild(node.OwnerDocument.CreateElement(nodeName));
        }

        private static long ReadAttributeLong(XmlNode node, string attrName)
        {
            var val = ReadAttribute(node, attrName);
            try
            {
                return string.IsNullOrEmpty(val) ? 0 : long.Parse(val);
            }
            catch (FormatException)
            {
                return 0;
            }
            catch (ArgumentNullException)
            {
                return 0;
            }
        }

        private static int ReadAttributeInt(XmlNode node, string attrName)
        {
            var val = ReadAttribute(node, attrName);

            try
            {
                return string.IsNullOrEmpty(val) ? 0 : int.Parse(val);
            }
            catch (FormatException)
            {
                return 0;
            }
            catch (ArgumentNullException)
            {
                return 0;
            }
        }

        private static string ReadAttribute(XmlNode node, string attrName)
        {
            var attr = node.Attributes[attrName];
            return attr != null ? attr.Value : null;
        }

        private static DateTime ReadAttributeDate(XmlNode element, string attributeName)
        {
            var attr = element.Attributes[attributeName];
            if (attr == null || attr.Value == null)
            {
                return default(DateTime);
            }

            try
            {
                return DateTime.Parse(attr.Value);
            }
            catch (FormatException)
            {
                return default(DateTime);
            }
        }
    }
}