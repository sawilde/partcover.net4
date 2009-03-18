using System;
using System.Reflection;

namespace PartCover.Framework.Stuff
{
    public static class Types
    {
        public static TypeAttributes GetAccessAndSemantic(UInt32 flags)
        {
            return (TypeAttributes)flags & (TypeAttributes.VisibilityMask | TypeAttributes.ClassSemanticsMask);
        }

        public static bool IsPrivate(UInt32 flags)
        {
            return ((TypeAttributes)flags & TypeAttributes.VisibilityMask) == TypeAttributes.NotPublic;
        }

        public static bool IsInterface(uint flags)
        {
            return ((TypeAttributes)flags & TypeAttributes.Interface) == TypeAttributes.Interface;
        }

        public static bool IsValueType(UInt32 flags)
        {
            return ((TypeAttributes)flags & TypeAttributes.LayoutMask) != TypeAttributes.AutoLayout;
        }
    }
}