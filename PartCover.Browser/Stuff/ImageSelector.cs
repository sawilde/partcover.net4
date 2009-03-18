using System.Collections.Generic;
using System.Reflection;

using PartCover.Browser.Resources;
using PartCover.Framework.Stuff;
using PartCover.Browser.Api.ReportItems;

namespace PartCover.Browser.Stuff
{
    internal static class ImageSelector
    {
        private readonly static Dictionary<TypeAttributes, int> typeImages = new Dictionary<TypeAttributes, int>();
        private readonly static Dictionary<MethodAttributes, int> methodImages = new Dictionary<MethodAttributes, int>();

        static ImageSelector()
        {
            typeImages[TypeAttributes.NotPublic | TypeAttributes.Class] = VSImage.Current.ClassPrivate;
            typeImages[TypeAttributes.Public | TypeAttributes.Class] = VSImage.Current.Class;
            typeImages[TypeAttributes.NestedPublic | TypeAttributes.Class] = VSImage.Current.Class;
            typeImages[TypeAttributes.NestedPrivate | TypeAttributes.Class] = VSImage.Current.ClassPrivate;
            typeImages[TypeAttributes.NestedFamily | TypeAttributes.Class] = VSImage.Current.ClassProtected;
            typeImages[TypeAttributes.NestedAssembly | TypeAttributes.Class] = VSImage.Current.ClassInternal;
            typeImages[TypeAttributes.NestedFamANDAssem | TypeAttributes.Class] = VSImage.Current.ClassInternalProtected;
            typeImages[TypeAttributes.NestedFamORAssem | TypeAttributes.Class] = VSImage.Current.ClassInternalProtected;

            typeImages[TypeAttributes.NotPublic | TypeAttributes.Interface] = VSImage.Current.InterfacePrivate;
            typeImages[TypeAttributes.Public | TypeAttributes.Interface] = VSImage.Current.Interface;
            typeImages[TypeAttributes.NestedPublic | TypeAttributes.Interface] = VSImage.Current.Interface;
            typeImages[TypeAttributes.NestedPrivate | TypeAttributes.Interface] = VSImage.Current.InterfacePrivate;
            typeImages[TypeAttributes.NestedFamily | TypeAttributes.Interface] = VSImage.Current.InterfaceProtected;
            typeImages[TypeAttributes.NestedAssembly | TypeAttributes.Interface] = VSImage.Current.InterfaceInternal;
            typeImages[TypeAttributes.NestedFamANDAssem | TypeAttributes.Interface] = VSImage.Current.InterfaceInternalProtected;
            typeImages[TypeAttributes.NestedFamORAssem | TypeAttributes.Interface] = VSImage.Current.InterfaceInternalProtected;

            typeImages[TypeAttributes.NotPublic | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypePrivate;
            typeImages[TypeAttributes.Public | TypeAttributes.LayoutMask] = VSImage.Current.ValueType;
            typeImages[TypeAttributes.NestedPublic | TypeAttributes.LayoutMask] = VSImage.Current.ValueType;
            typeImages[TypeAttributes.NestedPrivate | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypePrivate;
            typeImages[TypeAttributes.NestedFamily | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypeProtected;
            typeImages[TypeAttributes.NestedAssembly | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypeInternal;
            typeImages[TypeAttributes.NestedFamANDAssem | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypeInternalProtected;
            typeImages[TypeAttributes.NestedFamORAssem | TypeAttributes.LayoutMask] = VSImage.Current.ValueTypeInternalProtected;

            methodImages[MethodAttributes.Public] = VSImage.Current.Method;
            methodImages[MethodAttributes.Private] = VSImage.Current.MethodPrivate;
            methodImages[MethodAttributes.PrivateScope] = VSImage.Current.MethodPrivate;
            methodImages[MethodAttributes.Family] = VSImage.Current.MethodProtected;
            methodImages[MethodAttributes.Assembly] = VSImage.Current.MethodInternal;
            methodImages[MethodAttributes.FamANDAssem] = VSImage.Current.MethodInternalProtected;
            methodImages[MethodAttributes.FamORAssem] = VSImage.Current.MethodInternalProtected;

            methodImages[MethodAttributes.Public | MethodAttributes.Static] = VSImage.Current.Method_Static;
            methodImages[MethodAttributes.Private | MethodAttributes.Static] = VSImage.Current.MethodPrivate_Static;
            methodImages[MethodAttributes.PrivateScope | MethodAttributes.Static] = VSImage.Current.MethodPrivate_Static;
            methodImages[MethodAttributes.Family | MethodAttributes.Static] = VSImage.Current.MethodProtected_Static;
            methodImages[MethodAttributes.Assembly | MethodAttributes.Static] = VSImage.Current.MethodInternal_Static;
            methodImages[MethodAttributes.FamANDAssem | MethodAttributes.Static] = VSImage.Current.MethodInternalProtected_Static;
            methodImages[MethodAttributes.FamORAssem | MethodAttributes.Static] = VSImage.Current.MethodInternalProtected_Static;

        }

        public static int ForPropertyGet(IMethod md)
        {
            return VSImage.Current.PropertyGet;
        }

        public static int ForPropertySet(IMethod md)
        {
            return VSImage.Current.PropertySet;
        }

        public static int ForEventAdd(IMethod md)
        {
            return VSImage.Current.EventAdd;
        }

        public static int ForEventRemove(IMethod md)
        {
            return VSImage.Current.EventRemove;
        }

        public static int ForType(IClass type)
        {
            var visibilityAndSemantic = Types.GetAccessAndSemantic(type.Flags);
            if (Types.IsPrivate(type.Flags))
            {
                var nested = type.Name.IndexOf('+');
                var generic = type.Name.IndexOf('<');
                if (nested == -1 || (generic != -1 && nested > generic))
                    visibilityAndSemantic |= TypeAttributes.NestedAssembly;
            }

            if (Types.IsValueType(type.Flags))
            {
                visibilityAndSemantic |= TypeAttributes.LayoutMask;
            }

            int index;
            if (typeImages.TryGetValue(visibilityAndSemantic, out index))
                return index;

            return -1;
        }

        public static int ForMethod(IMethod md)
        {
            var access = Methods.getAccess(md.Flags);
            if (Methods.isStatic(md.Flags))
            {
                access |= MethodAttributes.Static;
            }

            int index;
            if (methodImages.TryGetValue(access, out index))
                return index;

            return -1;
        }
    }
}
