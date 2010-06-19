namespace PartViewer.Model
{
    public interface IViewPlugin
    {
        void Attach(IView target);
        void Detach(IView target);
    }
}
