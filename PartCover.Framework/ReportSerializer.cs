using System.Collections.Generic;
using System.Globalization;
using System.Xml;
using PartCover.Framework.Data;

namespace PartCover.Framework
{
    public class ReportSerializer
    {
        public static void Load(XmlReader reader, Report report) { }

        public static void Save(XmlWriter writer, Report report)
        {
            writer.WriteStartElement("PartCoverReport");
            writer.WriteAttributeString("data", report.Date.ToString("O", CultureInfo.InvariantCulture));

            foreach (var dFile in report.Files)
            {
                writer.WriteStartElement("File");
                writer.WriteAttributeString("id", dFile.Id.ToString());
                writer.WriteAttributeString("url", dFile.PathUri);
                writer.WriteEndElement();
            }

            var typeList = new List<TypedefEntry>();

            new List<AssemblyEntry>(report.Assemblies).ForEach(x =>
            {
                writer.WriteStartElement("Assembly");
                writer.WriteAttributeString("name", x.Name);
                writer.WriteAttributeString("module", x.Module);
                writer.WriteEndElement();

                typeList.AddRange(x.Types);
            });

            typeList.ForEach(x =>
            {
                writer.WriteStartElement("Type");
                writer.WriteAttributeString("asm", x.Assembly.Name);
                writer.WriteAttributeString("flags", ((long)x.Attributes).ToString());

                new List<MethodEntry>(x.Methods).ForEach(m =>
                {
                    writer.WriteStartElement("Method");
                    writer.WriteAttributeString("name", m.Name);
                    writer.WriteAttributeString("sig", m.Signature);
                    writer.WriteAttributeString("flags", ((long)m.Flags).ToString());
                    writer.WriteAttributeString("iflags", ((long)m.ImplFlags).ToString());

                    new List<MethodBlock>(m.Blocks).ForEach(b =>
                    {
                        writer.WriteStartElement("pt");
                        writer.WriteAttributeString("visit", b.VisitCount.ToString(CultureInfo.InvariantCulture));
                        writer.WriteAttributeString("pos", b.Offset.ToString(CultureInfo.InvariantCulture));
                        writer.WriteAttributeString("len", b.Length.ToString(CultureInfo.InvariantCulture));

                        if (b.File != 0)
                        {
                            writer.WriteAttributeString("fid", b.File.ToString(CultureInfo.InvariantCulture));
                            writer.WriteAttributeString("sl", b.Start.Line.ToString(CultureInfo.InvariantCulture));
                            writer.WriteAttributeString("sc", b.Start.Column.ToString(CultureInfo.InvariantCulture));
                            writer.WriteAttributeString("el", b.End.Line.ToString(CultureInfo.InvariantCulture));
                            writer.WriteAttributeString("ec", b.End.Column.ToString(CultureInfo.InvariantCulture));
                        }

                        writer.WriteEndElement();
                    });

                    writer.WriteEndElement();
                });

                writer.WriteEndElement();
            });

            writer.WriteEndElement();
        }
    }
}