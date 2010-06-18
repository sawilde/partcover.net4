using System.Reflection;

namespace PartCover.Framework.Stuff
{
    public static class Types
    {
        public static string[] GetNamespaceChain(string fullName)
        {
            return fullName.Split('.');
        }

        public static string RemoveNamespacePath(string typedefName)
        {
            var index = typedefName.LastIndexOf('.');
            return index == -1 ? typedefName : typedefName.Remove(0, index).TrimStart('.');
        }

        public static TypeAttributes GetAccessAndSemantic(TypeAttributes flags)
        {
            return flags & (TypeAttributes.VisibilityMask | TypeAttributes.ClassSemanticsMask);
        }

        public static bool IsPrivate(TypeAttributes flags)
        {
            return (flags & TypeAttributes.VisibilityMask) == TypeAttributes.NotPublic;
        }

        public static bool IsInterface(uint flags)
        {
            return ((TypeAttributes)flags & TypeAttributes.Interface) == TypeAttributes.Interface;
        }

        public static bool IsValueType(TypeAttributes flags)
        {
            return (flags & TypeAttributes.LayoutMask) != TypeAttributes.AutoLayout;
        }
    }
}