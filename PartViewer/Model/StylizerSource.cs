namespace PartViewer.Model
{
    public interface StylizerSource
    {
        Document Document { get;}
        DocumentRange Range { get;}

        void setFace(DocumentRange target, string face);
        void setFace(DocumentRange target, StyleFace face);
    }
}
