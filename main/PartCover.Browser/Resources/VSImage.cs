using System.Drawing;

namespace PartCover.Browser.Resources
{
    internal class VSImage : ResourceImageList
    {
        private VSImage()
            : base(System.Reflection.Assembly.GetExecutingAssembly().GetName().Name + ".Resources.VSObject.VSObject_", ".bmp", Color.Magenta)
        {
        }

        private static VSImage current;
        public static VSImage Current
        {
            get
            {
                if (current == null)
                    current = new VSImage();
                return current;
            }
        }

        public int Assembly { get { return indexOf("Assembly"); } }
        public int Namespace { get { return indexOf("Namespace"); } }

        public int Class { get { return indexOf("Class"); } }
        public int ClassPrivate { get { return indexOf("ClassPrivate"); } }
        public int ClassProtected { get { return indexOf("ClassProtected"); } }
        public int ClassInternal { get { return indexOf("ClassSealed"); } }
        public int ClassInternalProtected { get { return indexOf("ClassFriend"); } }

        public int Interface { get { return indexOf("Interface"); } }
        public int InterfacePrivate { get { return indexOf("InterfacePrivate"); } }
        public int InterfaceProtected { get { return indexOf("InterfaceProtected"); } }
        public int InterfaceInternal { get { return indexOf("InterfaceSealed"); } }
        public int InterfaceInternalProtected { get { return indexOf("InterfaceFriend"); } }

        public int ValueType { get { return indexOf("ValueType"); } }
        public int ValueTypePrivate { get { return indexOf("ValueTypePrivate"); } }
        public int ValueTypeProtected { get { return indexOf("ValueTypeProtected"); } }
        public int ValueTypeInternal { get { return indexOf("ValueTypeSealed"); } }
        public int ValueTypeInternalProtected { get { return indexOf("ValueTypeFriend"); } }

        public int Method { get { return indexOf("Method"); } }
        public int MethodPrivate { get { return indexOf("MethodPrivate"); } }
        public int MethodProtected { get { return indexOf("MethodProtected"); } }
        public int MethodInternal { get { return indexOf("MethodSealed"); } }
        public int MethodInternalProtected { get { return indexOf("MethodFriend"); } }

        public int Method_Static { get { return indexOf("Method_S"); } }
        public int MethodPrivate_Static { get { return indexOf("MethodPrivate_S"); } }
        public int MethodProtected_Static { get { return indexOf("MethodProtected_S"); } }
        public int MethodInternal_Static { get { return indexOf("MethodSealed_S"); } }
        public int MethodInternalProtected_Static { get { return indexOf("MethodFriend_S"); } }

        public int PropertySet { get { return indexOf("PropertySet"); } }
        public int PropertyGet { get { return indexOf("PropertyGet"); } }
        public int Properties { get { return indexOf("Properties"); } }

        public int EventAdd { get { return indexOf("EventAdd"); } }
        public int EventRemove { get { return indexOf("EventRemove"); } }
    }
}
