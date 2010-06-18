namespace PartCover.StressTest.Classes
{
    public static class BigMethodExtensions
    {
        public static void Foo1(this BigMethod method) { method.Foo(0); }
        public static void Foo2(this BigMethod method) { method.Foo(1); }
    }

    public class BigMethod
    {
        public void Foo(int number)
        {
            while (number > 0)
            {
                FooInner(--number);
            }
        }

        public void FooInner(int number)
        {
            switch (number)
            {
            case 0: Foo0(); break;
            case 1: this.Foo1(); break;
            case 2: this.Foo2(); break;
            case 3: Foo3(); break;
            case 4: Foo4(); break;
            case 5: Foo5(); break;
            case 6: Foo6(); break;
            case 7: Foo7(); break;
            case 8: Foo8(); break;
            case 9: Foo9(); break;
            }
        }

        private static void Foo0() { }
        private void Foo3() { Foo(2); }
        private void Foo4() { Foo(3); }
        private void Foo5() { Foo(4); }
        private void Foo6() { Foo(5); }
        private void Foo7() { Foo(6); }
        private void Foo8() { Foo(7); }
        private void Foo9() { Foo(8); }
    }
}