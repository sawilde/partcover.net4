namespace PartViewer.Model
{
    public interface IStylizerSource
    {
        Document Document { get;}
        DocumentRange Range { get;}

        void AssignFace(DocumentRange target, string face);
        void AssignFace(DocumentRange target, IStyleFace face);
    }
}
