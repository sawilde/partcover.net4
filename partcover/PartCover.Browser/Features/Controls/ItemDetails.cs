using System.Windows.Forms;
using PartCover.Browser.Api;
using PartCover.Framework.Data;

namespace PartCover.Browser.Features.Controls
{
    public partial class ItemDetails 
        : UserControl
        , ITreeItemSelectionHandler
    {
        public ItemDetails()
        {
            InitializeComponent();
        }

        const string ASSEMBLY_INFO = @"{{\rtf1\ansi\b {0}\b0\line\b Domain:\b0 [{1}] {2}\line\b Types:\b0 {3}}}";

        const string NAMESPACE_INFO = @"{{\rtf1\ansi\b {0}\b0}}";

        const string TYPEDEF_INFO = @"{{\rtf1\ansi\b {0}\b0\line\b Flags:\b0 {1}}}";

        const string METHOD_INFO = @"{{\rtf1\ansi\b {0}\b0\line\b Sig:\b0 {1}\line\b Flags:\b0 {2}\line\b ImplFlags:\b0 {3}\line\b Body size:\b0 {4}}}";

        public void Select(AssemblyEntry assembly)
        {
            rtbNodeProps.Rtf = string.Format(ASSEMBLY_INFO, assembly.Name, assembly.DomainIndex, assembly.Domain, assembly.Types.Count);
        }

        public void Select(AssemblyEntry assembly, string namespacePath)
        {
            rtbNodeProps.Rtf = string.Format(NAMESPACE_INFO, namespacePath);
        }

        public void Select(TypedefEntry typedef)
        {
            rtbNodeProps.Rtf = string.Format(TYPEDEF_INFO, typedef.Name, typedef.Attributes);
        }

        public void Select(MethodEntry method)
        {
            rtbNodeProps.Rtf = string.Format(METHOD_INFO, method.Name, method.Signature, method.Flags, method.ImplFlags, method.BodySize);
        }

        public void Deselect()
        {
            rtbNodeProps.Rtf = "";
        }
    }
}
