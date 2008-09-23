using System;
using System.Collections.Generic;
using System.Text;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Stuff.ReportItems
{
    class AssemblyItem : IAssembly
    {
        private readonly string name;
        private readonly List<ClassItem> types = new List<ClassItem>();
        private readonly List<NamespaceItem> namespaces = new List<NamespaceItem>();

        public AssemblyItem(string name)
        {
            this.name = name;
        }

        public void addType(ClassItem item)
        {
            types.Add(item);
        }

        public IClass[] getTypes()
        {
            return types.ToArray();
        }

        public string Name
        {
            get { return name; }
        }

        public NamespaceItem findNamespace(string name, NamespaceItem parentNamespace)
        {
            return namespaces.Find(delegate(NamespaceItem actual) {
                return actual.Parent == parentNamespace && actual.Name == name;
            });
        }

        public void addNamespace(NamespaceItem namespaceItem)
        {
            namespaces.Add(namespaceItem);
        }
    }
}
