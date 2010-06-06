using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PartCover.StressTest.Classes
{
    public class GenericClass<T> 
    {
        T _data;
        
        public GenericClass(T data)
        {
            _data = data;
        }

        public T DoAction<S>(S thing)
        {
            return default(T);
        }

    }
}
