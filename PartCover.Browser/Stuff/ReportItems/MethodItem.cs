using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api.ReportItems;
using System.Reflection;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Stuff.ReportItems
{
    class MethodItem : IMethod
    {
        private uint flags, implFlags;
        private string name, sig;

        private ClassItem classItem;
        private List<CoveredVariantItem> variants = new List<CoveredVariantItem>();

        public MethodItem(CoverageReport.MethodDescriptor md, ClassItem classItem)
        {
            name = md.methodName;
            sig = md.methodSig;
            flags = md.flags;
            implFlags = md.implFlags;
            this.classItem = classItem;
        }

        public uint Flags { get { return flags; } }
        public uint ImplFlags { get { return implFlags; } }

        public string Name { get { return name; } }
        public string Signature { get { return sig; } }

        public ICoveredVariant[] CoveredVariants
        {
            get { return variants == null ? new ICoveredVariant[0] : variants.ToArray(); }
        }

        public void addBlock(CoveredVariantItem block)
        {
            if (variants == null)
                variants = new List<CoveredVariantItem>();
            variants.Add(block);
        }


        public ClassItem Class
        {
            get { return classItem; }
        }

        IClass IMethod.Class
        {
            get { return Class; }
        }
    }
}
