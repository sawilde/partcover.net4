using PartCover.Framework.Data;

namespace PartCover.Browser.Api
{
    public interface ITreeItemSelectionHandler
    {
        void Select(AssemblyEntry assembly);
        void Select(AssemblyEntry assembly, string namespacePath);
        void Select(TypedefEntry typedef);
        void Select(MethodEntry method);
        void Deselect();
    }
}