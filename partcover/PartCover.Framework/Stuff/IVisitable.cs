namespace PartCover.Framework.Stuff
{
    public interface IVisitable<T>
    {
        void Visit(T visitor);
    }
}
