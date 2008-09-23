using System;
using System.Collections.Generic;
using System.Text;

using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Stuff.ReportItems
{
    class NamespaceItem : INamespace
    {
        private string name;
        private NamespaceItem parent;
        private AssemblyItem assembly;

        public NamespaceItem(string name, AssemblyItem assembly)
        {
            this.name = name;
            this.assembly = assembly;
        }

        public string Name
        {
            get { return name; }
        }

        public NamespaceItem Parent
        {
            get { return parent; }
            set { parent = value; }
        }

        public AssemblyItem Assembly
        {
            get { return assembly ?? (Parent != null ? Parent.Assembly : null); }
            set { assembly = value; }
        }

        INamespace INamespace.Parent
        {
            get { return Parent; }
        }

        IAssembly INamespace.Assembly
        {
            get { return Assembly; }
        }
    }
}
