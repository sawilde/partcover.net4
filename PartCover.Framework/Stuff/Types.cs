using System;
using System.Reflection;

namespace PartCover.Framework.Stuff
{
    public static class Types
    {
        public static TypeAttributes getAccessAndSemantic(UInt32 flags)
        {
            TypeAttributes attrs = (TypeAttributes)flags;
            return attrs & (TypeAttributes.VisibilityMask | TypeAttributes.ClassSemanticsMask);
        }

        public static bool isPrivate(UInt32 flags)
        {
            TypeAttributes attrs = (TypeAttributes)flags;
            return (attrs & TypeAttributes.VisibilityMask) == TypeAttributes.NotPublic;
        }

        public static bool isInterface(UInt32 flags)
        {
            TypeAttributes attrs = (TypeAttributes)flags;
            return (attrs & TypeAttributes.Interface) == TypeAttributes.Interface;
        }

        public static bool isValueType(UInt32 flags)
        {
            TypeAttributes attrs = (TypeAttributes)flags;
            return (attrs & TypeAttributes.LayoutMask) != TypeAttributes.AutoLayout;
        }
    }
}