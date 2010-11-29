using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;

namespace PartCover.StressTest.Classes
{
    public class Superclass
    {
        public virtual void Method(string a)
        {
            // ...
        }
    }

    public class Subclass : Superclass
    {
        public override void Method(string a)
        {
            base.Method(a);
            // ...
        }
    }

    public class SuperclassGeneric<T>
    {
        public virtual void Method(T a)
        {
            // ...
        }
    }

    public class SubclassGeneric<T> : SuperclassGeneric<T>
    {
        public override void Method(T a)
        {
            base.Method(a);
            
        }
    }

}
