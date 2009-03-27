using System.Reflection;

namespace PartCover.Framework.Stuff
{
    public static class Types
    {
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