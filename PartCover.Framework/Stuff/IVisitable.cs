namespace PartCover.Framework.Stuff
{
    public interface IVisitable<T>
    {
        void visit(T visitor);
    }
}
