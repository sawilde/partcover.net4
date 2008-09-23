using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api.ReportItems;
using PartCover.Framework.Walkers;

namespace PartCover.Browser.Stuff.ReportItems
{
    class ClassItem : IClass
    {
        private uint flags;
        private string fullName;
        private NamespaceItem iNamespace;
        private readonly AssemblyItem iAssembly;
        private readonly List<MethodItem> methods = new List<MethodItem>();

        public ClassItem(string fullName, AssemblyItem iAssembly)
        {
            this.fullName = fullName;
            this.iAssembly = iAssembly;
        }

        public void addMethod(MethodItem item)
        {
            methods.Add(item);
        }

        public IMethod[] getMethods()
        {
            return methods.ToArray();
        }

        public uint Flags
        {
            get { return flags; }
            internal set { flags = value; }
        }

        public string Name
        {
            get { return CoverageReportHelper.GetTypeDefName(QName); }
        }

        public string QName
        {
            get { return fullName; }
        }

        public NamespaceItem Namespace
        {
            get { return iNamespace; }
            set { iNamespace = value; }
        }

        public AssemblyItem Assembly
        {
            get { return iAssembly; }
        }

        public INamespace[] getNamespaceChain()
        {
            List<NamespaceItem> items = new List<NamespaceItem>();

            NamespaceItem currentNamespace = Namespace;
            while (currentNamespace != null)
            {
                items.Add(currentNamespace);
                currentNamespace = currentNamespace.Parent;
            }

            items.Reverse();
            return items.ToArray();
        }

        IAssembly IClass.Assembly
        {
            get { return Assembly; }
        }
    }
}
