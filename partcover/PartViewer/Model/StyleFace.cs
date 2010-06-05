namespace PartViewer.Model
{
    public interface IStyleFace
    {
        string Name { get;}
        IStylizer Owner { get;}
        Style FaceStyle { get;}
    }
}
