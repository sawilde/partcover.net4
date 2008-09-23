namespace PartViewer.Model
{
    public interface ViewPlugin
    {
        void attach(View target);
        void detach(View target);
    }
}
