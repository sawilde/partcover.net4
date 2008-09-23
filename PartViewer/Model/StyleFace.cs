using PartViewer.Styles;

namespace PartViewer.Model
{
    public interface StyleFace
    {
        string Name { get;}
        Stylizer Owner { get;}
        Style FaceStyle { get;}
    }
}
