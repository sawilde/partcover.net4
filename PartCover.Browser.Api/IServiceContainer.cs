namespace PartCover.Browser.Api
{
    public interface IServiceContainer
    {
        T getService<T>() where T : class;

        bool registerService<T>(T service) where T : class;

        bool unregisterService<T>(T service) where T : class;
    }
}
