namespace PartCover.Browser.Api
{
    public interface IFeature : IService
    {
        void Attach(IServiceContainer container);

        void Detach(IServiceContainer container);

        void Build(IServiceContainer container);

        void Destroy(IServiceContainer container);
    }
}
