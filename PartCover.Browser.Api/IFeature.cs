using System;
using System.Collections.Generic;
using System.Text;

namespace PartCover.Browser.Api
{
    public interface IFeature : IService
    {
        void attach(IServiceContainer container);

        void detach(IServiceContainer container);

        void build(IServiceContainer container);

        void destroy(IServiceContainer container);
    }
}
