using System.Reflection;

namespace PartCover.Framework.Stuff
{
    public enum MdSpecial
    {
        Unknown,
        Get,
        Set,
        Add,
        Remove
    }

    public static class Methods
    {
        public static MethodAttributes getAccess(uint flags)
        {
            MethodAttributes attrs = (MethodAttributes) flags;
            return attrs & MethodAttributes.MemberAccessMask;
        }

        public static bool isStatic(uint flags)
        {
            MethodAttributes attrs = (MethodAttributes)flags;
            return (attrs & MethodAttributes.Static) == MethodAttributes.Static;
        }

        public static bool isSpecial(uint flags)
        { 
            MethodAttributes attrs = (MethodAttributes)flags;
            return (attrs & MethodAttributes.SpecialName) == MethodAttributes.SpecialName;
        }

        public static MdSpecial getMdSpecial(string name)
        {
            if (name.StartsWith("set_")) return MdSpecial.Set;
            if (name.StartsWith("get_")) return MdSpecial.Get;
            if (name.StartsWith("add_")) return MdSpecial.Add;
            if (name.StartsWith("remove_")) return MdSpecial.Remove;
            return MdSpecial.Unknown;
        }

        public static string getMdSpecialName(string name)
        {
            switch (getMdSpecial(name))
            {
                case MdSpecial.Add:
                case MdSpecial.Get:
                case MdSpecial.Set:
                    return name.Substring(4);
                case MdSpecial.Remove:
                    return name.Substring(7);
                default:
                    return name;
            }
        }
    }
}