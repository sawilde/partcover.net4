
// disable ADL warning for vc71
#ifdef _MSC_VER
#pragma warning( disable : 4675 ) // warning C4675: '...' : resolved overload was found by argument-dependent lookup
#endif

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <RCF/test/TestMinimal.hpp>

#include <boost/config.hpp>
#include <boost/test/floating_point_comparison.hpp>
#define RCF_CHECK_CLOSE(d1, d2, tol) RCF_CHECK( boost::test_tools::check_is_close(d1, d2, tol) )

#include <boost/any.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#ifndef __BORLANDC__
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/variant.hpp>
#endif

// streams
#include <SF/IBinaryStream.hpp>
#include <SF/OBinaryStream.hpp>
#include <SF/INativeBinaryStream.hpp>
#include <SF/ONativeBinaryStream.hpp>
#include <SF/ITextStream.hpp>
#include <SF/OTextStream.hpp>

// construction and registration
#include <SF/SfNew.hpp>
#include <SF/Registry.hpp>
#include <SF/SerializeParent.hpp>

// serializers
#include <SF/any.hpp>
#include <SF/list.hpp>
#include <SF/map.hpp>
#include <SF/memory.hpp>
#include <SF/scoped_ptr.hpp>
#include <SF/set.hpp>
#include <SF/shared_ptr.hpp>
#include <SF/string.hpp>

#include <SF/SerializeDynamicArray.hpp>
#include <SF/SerializeStaticArray.hpp>

#ifndef __BORLANDC__
#include <SF/tuple.hpp>
#include <SF/variant.hpp>
#endif

#include <SF/vector.hpp>

#include <SF/AdlWorkaround.hpp>

#include <RCF/test/PrintTestHeader.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/ByteBuffer.hpp>

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if (0) {} else for
#endif

namespace Test_Serialization {

    double fpt_tolerance = .1;

    bool fpt_compare(const double *pd1, const double *pd2, unsigned int count)
    {
        for (unsigned int i=0; i<count; i++)
            if ((pd1[i]-pd2[i])*(pd1[i]-pd2[i]) > fpt_tolerance)
                return false;
        return true;
    }

    bool fpt_compare(const float *pf1, const float *pf2, unsigned int count)
    {
        for (unsigned int i=0; i<count; i++)
            if ((pf1[i]-pf2[i])*(pf1[i]-pf2[i]) > fpt_tolerance)
                return false;
        return true;
    }

    namespace Test {

        struct GeomObject
        {
            GeomObject() : x(0), y(0) {}
            GeomObject(int x, int y) : x(x), y(y) {}
            virtual ~GeomObject() {}
            bool operator==(const GeomObject &rhs) { return x == rhs.x && y == rhs.y; }
            int x;
            int y;
        };

        struct Rect : public GeomObject
        {
            Rect() : GeomObject(), w(0), h(0) {}
            Rect(int x,int y, int w, int h) : GeomObject(x,y), w(w), h(h) {}
            bool operator==(const Rect &rhs) { return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h; }
            int w;
            int h;
        };

        class Empty {
        public:
            Empty() {}
        };

        struct D { virtual ~D() {} };
        struct A;
        struct B;
        struct C;

        struct A : public D { B *pb; };
        struct B : public D { C *pc; };
        struct C : public D { A *pa; };


        // E, F, G check that we can use serializeParent() without registering
        // anything, if we only serialize most-derived instances.
        struct E
        {
            virtual ~E() {}

            std::string mE;

            void serialize(SF::Archive & ar)
            {
                ar & mE;
            }
        };

        struct F : public E
        {
            std::string mF;

            void serialize(SF::Archive & ar)
            {
                serializeParent( (E *) 0, ar, *this);
                ar & mF;
            }

        };

        struct G : public F
        {
            std::string mG;

            void serialize(SF::Archive & ar)
            {
                serializeParent( (F *) 0, ar, *this);
                ar & mG;
            }
        };



        template<typename T>
        struct TestOne
        {
            bool operator==(const TestOne &rhs) { return n == rhs.n && t == rhs.t; }
            int n;
            T t;
        };

        template<typename T, typename U>
        struct TestTwo
        {
            bool operator==(const TestTwo &rhs) { return t == rhs.t && u == rhs.u; }
            T t;
            U u;
        };

        enum TestEnum { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY };

        class ArrayContainer {
        public:
            ArrayContainer()
            {
                memset(this, 0, sizeof(ArrayContainer));
            }
            ArrayContainer(int n) {
                memset(this, 0, sizeof(ArrayContainer));
                if (n) {

                    srand(1); // Reinitialize random number generator so we get the same sequence each time

                    for (int i=0; i<NSIZE; i++)
                        pn[i] = rand();

                    for (int i=0; i<CHSIZE; i++)
                        for (int j=0; j<CHSIZE; j++)
                            for (int k=0; k<CHSIZE; k++)
                                ch[i][j][k] = 'A' + i + j + k;

                    pd = new double[DSIZE];
                    pd_length = DSIZE;
                    double pi = 3.141592654E-10;
                    for (int i=0; i<DSIZE; i++)
                        pd[i] = rand()*pi;

                    for (int i=0; i<PIISIZE; i++)
                        for (int j=0; j<PIISIZE; j++)
                            pii[i][j] = std::pair<int,int>(i,j);

                    pcc = new std::pair<char,char>[PCCSIZE];
                    pcc_length = PCCSIZE;
                    for (int i=0; i<PIISIZE; i++)
                        pcc[i] = std::pair<char,char>('A'+i,'A'+i);

                }
            }
            ~ArrayContainer()
            {
                if (pd) delete [] pd;
                if (pcc) delete [] pcc;
            }

            friend bool operator==( const ArrayContainer &lhs, const ArrayContainer &rhs )
            {
                bool bOk = true;
                bOk = bOk && (lhs.pn && rhs.pn && 0 == memcmp(&lhs.pn, &rhs.pn, ArrayContainer::NSIZE*sizeof(int)));
                bOk = bOk && (lhs.ch && rhs.ch && 0 == memcmp(&lhs.ch, &rhs.ch, ArrayContainer::CHSIZE*ArrayContainer::CHSIZE*sizeof(char)));
                bOk = bOk && (lhs.pd_length == rhs.pd_length) && (fpt_compare(lhs.pd, rhs.pd, lhs.pd_length));
                bOk = bOk && (lhs.pii && rhs.pii && 0 == memcmp(&lhs.pii, &rhs.pii, ArrayContainer::PIISIZE*ArrayContainer::PIISIZE*sizeof(std::pair<int,int>)));
                bOk = bOk && (lhs.pcc_length == rhs.pcc_length) && (0 == memcmp(lhs.pcc, rhs.pcc, sizeof(std::pair<char,char>)*lhs.pcc_length) );
                return bOk;
            }

        public:

            enum { NSIZE = 100 };
            enum { CHSIZE = 3 };
            enum { DSIZE = 100 };
            enum { PIISIZE = 4 };
            enum { PCCSIZE = 4 };

            //static const int NSIZE = 100;
            //static const int CHSIZE = 3;
            //static const int DSIZE = 100;
            //static const int PIISIZE = 4;
            //static const int PCCSIZE = 4;

            int pn[NSIZE];
            char ch[CHSIZE][CHSIZE][CHSIZE];
            double *pd;
            unsigned int pd_length;

            std::pair<int,int> pii[PIISIZE][PIISIZE];
            std::pair<char,char> *pcc;
            unsigned int pcc_length;
        };

        class Props {
        public:
            Props() : width_(0) {}
            int get_Width() { return width_; }
            void set_Width(int width) { width_ = width; }
            friend bool operator==( const Props &lhs, const Props &rhs ) { return lhs.width_ == rhs.width_; }
        private:
            int width_;
        };

        namespace K1 {

            class A {
            public:
                A() : a(0) {}
                A(int a) : a(a) {}
                friend bool operator==(const A &lhs, const A &rhs) { return lhs.a == rhs.a; }

                void serialize(SF::Archive &ar)
                {
                    ar & a;
                }
            private:
                int a;
            };

            namespace K2 {

                class B {
                public:
                    B() : b(0) {}
                    B(int b) : b(b) {}
                    friend bool operator==(const B &lhs, const B &rhs) { return lhs.b == rhs.b; }

                    void serialize(SF::Archive &ar)
                    {
                        ar & b;
                    }
                private:
                    int b;
                };


            } //namespace K2

        } //namespace K1

    } // namespace Test


    namespace Test {

        void serialize(SF::Archive &ar, D &d)
        {}

        void serialize(SF::Archive &ar, A &a)
        {
            ar & a.pb;
        }

        void serialize(SF::Archive &ar, B &b)
        {
            ar & b.pc;
        }

        void serialize(SF::Archive &ar, C &c)
        {
            ar & c.pa;
        }

        void serialize(SF::Archive &ar, GeomObject &geomObject)
        {
            ar & geomObject.x & geomObject.y;
        }

        void serialize(SF::Archive &ar, Rect &rect)
        {
            serializeParent( (GeomObject*) 0, ar, rect);
            ar & rect.w & rect.h;
        }

        void serialize(SF::Archive &ar, Empty &e)
        {}

        template<typename T>
        void serialize(SF::Archive &ar, TestOne<T> &t)
        {
            ar & t.t & t.n;
        }

        template<typename T, typename U>
        void serialize(SF::Archive &ar, TestTwo<T,U> &t)
        {
            ar & t.t & t.u;
        }

#if !defined(_MSC_VER) || _MSC_VER > 1200

        void serialize(SF::Archive &ar, TestOne<int> &t)
        {
            ar & t.t & t.n;
        }

#endif

        void serialize(SF::Archive &ar, Props &p)
        {
            if (ar.isRead())
            {
                int width;
                ar & width;
                p.set_Width(width);
            }
            else // if (ar.isWrite())
            {
                ar & p.get_Width();
            }
        }


    #if defined(__BORLANDC__) || ( defined(_MSC_VER) && _MSC_VER == 1200 )
        // TODO: sort out array serialization with borland
        // TODO: sort out array serialization with vc6
        /*
        // Implicit type detection of inline static arrays is broken in BCC 5.5, so need to specify the types explicitly
        typedef std::pair<int,int> pair_int_int_t;
        SF_BEGIN( ArrayContainer, "" )
            SF_TYPED_MEMBER( pn_, "pn", int [ArrayContainer::NSIZE] );
            SF_TYPED_MEMBER( ch_, "ch", char [ArrayContainer::CHSIZE][ArrayContainer::CHSIZE][ArrayContainer::CHSIZE] );
            SF_DYNAMIC_ARRAY(pd, SF_THIS->pd_length, "pd");
            SF_TYPED_MEMBER( pii_, "pii", pair_int_int_t [ArrayContainer::PIISIZE][ArrayContainer::PIISIZE] );
            SF_DYNAMIC_ARRAY(pcc, SF_THIS->pcc_length, "pcc");
        SF_END( ArrayContainer );
        */

    #else

        void serialize(SF::Archive &ar, ArrayContainer &a)
        {
            ar & a.pn;
            ar & a.ch;
            ar & SF::dynamicArray(a.pd, a.pd_length);
            ar & a.pii;
            ar & SF::dynamicArray(a.pcc, a.pcc_length);
        }

    #endif


    } // namespace Test



    //********************************************************************
    template<typename IStream, typename OStream>
    void testAtomic()
    {
        using namespace Test;

        bool b1 = true, b2 = false;
        int n1 = 1, n2 = 0;
        char ch1 = 'A', ch2 = 'B';
        float f1 = 2.71828f, f2 = 0.0f;
        double d1 = -3.141592654E-8, d2 = 0.0;

        std::ostringstream os;
        OStream ostr(os);
        ostr << b1 << n1 << ch1 << f1 << d1;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> b2 >> n2 >> ch2 >> f2 >> d2;

        RCF_CHECK( b2 == b1 );
        RCF_CHECK( n2 == n1 );
        RCF_CHECK( ch2 == ch1 );
        
        //RCF_CHECK_CLOSE(f2, f1, 1.1 );
        //RCF_CHECK_CLOSE(d2, d1, 1.1 );

        RCF_CHECK( (f2-f1)*(f2-f1) < 0.1 );
        RCF_CHECK( (d2-d1)*(d2-d1) < 0.1 );

    #ifdef BOOST_WINDOWS
        {
            __int64 m1 = -1, m2 = 0;

            std::ostringstream os;
            OStream ostr(os);
            ostr << m1;

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> m2;

            RCF_CHECK( m2 == m1 );
        }
    #endif

    }

    template<typename IStream, typename OStream>
    void testComposite()
    {
        using namespace Test;

        GeomObject geom1(1,2), geom2(0,0);
        Rect rect1(1,2,3,4), rect2(0,0,0,0);

        TestOne<bool> to1, to2;
        to1.t = true; to1.n = 1;
        to2.t = false; to2.n = 0;

        TestTwo<std::string, double> tt1, tt2;
        tt1.t = "pi"; tt1.u = 3.14;
        tt2.t = ""; tt2.u = 0;

        std::pair<char, int> p1('a', 1), p2('\0', 0);

        Props prop1, prop2;
        prop1.set_Width(1);
        prop2.set_Width(0);

        std::ostringstream os;
        OStream ostr(os);
        ostr << geom1 << rect1 << to1 << tt1 << p1 << prop1;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> geom2 >> rect2 >> to2 >> tt2 >> p2 >> prop2;

        RCF_CHECK( geom2 == geom1 );
        RCF_CHECK( rect2 == rect1 );
        RCF_CHECK( to2 == to1 );
        RCF_CHECK( tt2 == tt1 );
        RCF_CHECK( p2 == p1 );
        RCF_CHECK( prop2 == prop1 );

    }

    template<typename IStream, typename OStream>
    void testEmptyComposite()
    {

        using namespace Test;

        Empty e1, e2;
        Empty *pe1 = NULL, *pe2 = &e1, *pe3 = NULL, *pe4 = NULL;
        std::vector<int> v1, v2;
        std::vector<int> *pv1 = NULL, *pv2 = &v1, *pv3 = NULL, *pv4 = NULL;

        std::ostringstream os;
        OStream ostr(os);
        ostr << e1 << pe1 << pe2;
        ostr << v1 << pv1 << pv2;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> e2 >> pe3 >> pe4;
        istr >> v2 >> pv3 >> pv4;

        RCF_CHECK( pe3 == NULL );
        RCF_CHECK( pe4 != NULL );
        RCF_CHECK( v2.empty() );
        RCF_CHECK( pv3 == NULL );
        RCF_CHECK( pv4 != NULL );

        //if (pe3) delete pe3;
        //if (pe4) delete pe4;
        //if (pv3) delete pv3;
        //if (pv4) delete pv4;
    }


    template<typename IStream, typename OStream>
    void testArrays()
    {
        using namespace Test;

        const SF::UInt32 SIZE = 40;

        // Single dimensional static array
        int pn1[SIZE];
        for (int i=0;i<SIZE;i++)
            pn1[i] = i-20;

        int pn2[SIZE];
        memset(pn2, 0, SIZE*sizeof(int));

        // Single dimensional dynamic array
        int *pn3 = new int[SIZE];
        memcpy(pn3, pn1, SIZE*sizeof(int));

        const int *pn4 = NULL;
        SF::UInt32 n4 = 0;

        // Multidimensional static array
        int pn5[5][5][5];
        for (int i=0; i<5; i++)
            for (int j=0; j<5; j++)
                for (int k=0;k<5; k++)
                    pn5[i][j][k] = i*j*k;

        int pn6[5][5][5];
        memset(pn6, 0, 5*5*5*sizeof(int));

        const int pn7[2][2] = { {1,2}, {3,4} };
        const int pn8[2][2] = { {0,0}, {0,0} };

        // Object containing arrays
        ArrayContainer a1(1), a2(0);

        // serialization of fixed and dynamic size arrays not implemented yet for vc6
#if !(defined(_MSC_VER) && _MSC_VER < 1310)

#if !defined(__BORLANDC__)

        {
            // Serialization of multidimensional arrays doesnt work with BCB, except as members of a class,
            // due to compiler bugs.
            std::ostringstream os;
            OStream ostr(os);
            ostr << pn5;
            ostr << pn7;

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> pn6;
            istr >> pn8;

            RCF_CHECK( pn5 && pn6 && 0 == memcmp( pn5, pn6, 5*5*5*sizeof(int) ) );
            RCF_CHECK( pn7 && pn8 && 0 == memcmp( pn7, pn8, 2*2*sizeof(int) ) );
        }

#endif

        {
            std::ostringstream os;
            OStream ostr(os);
            ostr << pn1;
            ostr << a1;

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> pn2;
            istr >> a2;

            RCF_CHECK( pn1 && pn2 && 0 == memcmp( pn1, pn2, SIZE*sizeof(int) ) );
            RCF_CHECK( a1 == a2 );
        }

        {
            std::ostringstream os;
            OStream ostr(os);
            ostr << SF::dynamicArray(pn3,SIZE);

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> SF::dynamicArray(pn4, n4);

            RCF_CHECK( pn3 && pn4 && 0 == memcmp( pn3, pn4, SIZE*sizeof(int) ) );

            delete [] pn3;
            delete [] pn4;
        }

#endif

    }


    template<typename IStream, typename OStream>
    void testEnum()
    {
        using namespace Test;

        TestEnum x1,y1,z1, x2,y2,z2;
        x1 = MONDAY;
        y1 = TUESDAY;
        z1 = SUNDAY;
        x2 = SUNDAY;
        y2 = SUNDAY;
        z2 = SUNDAY;

        std::ostringstream os;
        OStream ostr(os);
        ostr << x1 << y1 << z1;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> x2 >> y2 >> z2;

        RCF_CHECK( x1 == x2 );
        RCF_CHECK( y1 == y2 );
        RCF_CHECK( z1 == z2 );
    }


    template<typename IStream, typename OStream>
    void testContainers()
    {
        using namespace Test;

        std::string s0, s1, s2, s3, s4, s5;
        std::list<int> L1,L2;
        std::set<double> S1,S2;
        std::map<std::string, bool> M1,M2;
        std::vector< std::pair<int,int> > V1,V2;

        s0 = std::string();
        s1 = std::string("");
        s2 = std::string("a");

        L1.push_back(1);
        L1.push_back(2);
        L1.push_back(3);

        S1.insert(1.1);
        S1.insert(2.2);
        S1.insert(3.3);

        M1["one"] = true;
        M1["two"] = false;
        M1["three"] = true;

        std::string str1 = "In the beginning", str2;

        std::ostringstream os;
        OStream ostr(os);
        ostr << s0 << s1 << s2;
        ostr << L1;
        ostr << S1;
        ostr << M1;
        ostr << V1;
        ostr << str1;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> s3 >> s4 >> s5;
        istr >> L2;
        istr >> S2;
        istr >> M2;
        istr >> V2;
        istr >> str2;

        RCF_CHECK(s0 == s3 && s1 == s4 && s2 == s5);
        RCF_CHECK( L2.size() == L1.size() );
        RCF_CHECK( std::equal(L1.begin(), L1.end(), L2.begin()) );
        RCF_CHECK( S2.size() == S1.size() );
        RCF_CHECK( std::equal(S1.begin(), S1.end(), S2.begin()) );
        RCF_CHECK( M2.size() == M1.size() );
        RCF_CHECK( std::equal(M1.begin(), M1.end(), M2.begin()) );
        RCF_CHECK( V2.size() == V1.size() );
        RCF_CHECK( std::equal(V1.begin(), V1.end(), V2.begin()) );
        RCF_CHECK( str2.size() == str1.size() );
        RCF_CHECK( str1 == str2 );

        {
            // fast vector serialization

            std::vector<int> vn0(3000);
            for (int i=0; i<vn0.size(); ++i)
            {
                vn0[i] = i;
            }

            std::ostringstream os;
            OStream ostr(os);
            ostr << vn0;

            std::vector<int> vn1;

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> vn1;

            RCF_CHECK(vn0 == vn1);
        }


    }

    template<typename IStream, typename OStream>
    void testGraphs()
    {
        using namespace Test;

        A a1,a2;
        B b1,b2;
        C c1,c2;
        D d1,d2;

        int n = 17;
        int *pn = &n;

        int m = 0;
        int *pm = NULL;

        std::ostringstream os;
        OStream ostr(os);
        ostr.enableContext();

        {
            a1.pb = &b1;
            b1.pc = &c1;
            c1.pa = &a1;
            std::vector<D *> vec;
            vec.push_back(&a1);
            vec.push_back(&b1);
            vec.push_back(&c1);
            vec.push_back(&a1);
            vec.push_back(&b1);
            vec.push_back(&c1);

            ostr << &n << pn;
            ostr << d1;
            ostr << vec;
            ostr.clearContext();
            ostr << a1;
            ostr.clearContext();

            a1.pb = NULL;
            b1.pc = NULL;
            c1.pa = NULL;
            vec.clear();
            vec.push_back(&a1);
            vec.push_back(&b1);
            vec.push_back(&c1);
            vec.push_back(&a1);
            vec.push_back(&b1);
            vec.push_back(&c1);

            ostr << vec;
            ostr.clearContext();
            ostr << a1 ;
            ostr.clearContext();
        }

        std::istringstream is(os.str());
        IStream istr(is);
        {
            a2.pb = NULL;
            b2.pc = NULL;
            c2.pa = NULL;
            std::vector<D *> vec;

            istr >> m >> pm;
            istr >> d2;
            istr >> vec;
            istr >> a2 ;

            RCF_CHECK( m == n);
            RCF_CHECK( pm == &m );
            RCF_CHECK( vec.size() == 6 );
            RCF_CHECK( vec[0] == vec[3] );
            RCF_CHECK( vec[1] == vec[4] );
            RCF_CHECK( vec[2] == vec[5] );
            RCF_CHECK( ((A *) vec[0])->pb == ((B *) vec[1]) );
            RCF_CHECK( ((B *) vec[1])->pc == ((C *) vec[2]) );
            RCF_CHECK( ((C *) vec[2])->pa == ((A *) vec[0]) );

            RCF_CHECK( a2.pb->pc->pa == &a2 );

            delete vec[0];
            delete vec[1];
            delete vec[2];
            vec.clear();

            delete a2.pb->pc;
            delete a2.pb;

            istr >> vec;
            istr >> a2 ;

            RCF_CHECK( vec.size() == 6 );
            RCF_CHECK( vec[0] == vec[3] );
            RCF_CHECK( vec[1] == vec[4] );
            RCF_CHECK( vec[2] == vec[5] );
            RCF_CHECK( ((A *) vec[0])->pb == NULL );
            RCF_CHECK( ((B *) vec[1])->pc == NULL );
            RCF_CHECK( ((C *) vec[2])->pa == NULL );

            RCF_CHECK( a2.pb == NULL );

            delete vec[0];
            delete vec[1];
            delete vec[2];
        }
    }


    template<typename IStream, typename OStream>
    void testPolymorphism()
    {
        using namespace Test;

        Rect rect(1,2,3,4);
        GeomObject geom(1,2);
        GeomObject *pg1 = NULL, *pg2 = NULL, *pg3 = NULL;
        Rect *pr1 = NULL, *pr2 = NULL;
        GeomObject &rg1 = geom, &rg2 = rect;

        GeomObject g;
        Rect r;
        GeomObject &rg3 = g;
        GeomObject &rg4 = r;

        G g1;
        g1.mE = "E";
        g1.mF = "F";
        g1.mG = "G";

        G g2;

        pg1 = NULL;
        pg2 = (GeomObject *) &geom;
        pg3 = (GeomObject *) &rect;
        pr1 = NULL;
        pr2 = &rect;

        std::ostringstream os;
        OStream ostr(os);
        ostr << pg1 << pg2 << pg3;
        ostr << pr1 << pr2;
        ostr << rg1 << rg2;
        ostr << g1;

        pg1 = reinterpret_cast<GeomObject *>(1);
        pg2 = reinterpret_cast<GeomObject *>(1);
        pg3 = reinterpret_cast<GeomObject *>(1);
        pr1 = reinterpret_cast<Rect *>(1);
        pr2 = reinterpret_cast<Rect *>(1);

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> pg1 >> pg2 >> pg3;
        istr >> pr1 >> pr2;
        istr >> rg3 >> rg4;
        istr >> g2;

        RCF_CHECK( pg1 == NULL );
        RCF_CHECK( *pg2 == geom );
        RCF_CHECK( dynamic_cast<Rect*>(pg3) != NULL && * (dynamic_cast<Rect*>(pg3)) == rect );
        RCF_CHECK( pr1 == NULL );
        RCF_CHECK( *pr2 == rect );
        RCF_CHECK( rg1 == rg3 );
        RCF_CHECK( *(static_cast<Rect *>(&rg2)) == *(static_cast<Rect *>(&rg4)) );
        RCF_CHECK( g1.mE == g2.mE && g1.mF == g2.mF && g1.mG == g2.mG );

        delete pg2;
        delete pg3;
        delete pr2;
    }

    namespace MI {

        class A
        {
        public:

            A() : a()
            {}

            A(int a) : a(a)
            {}

            virtual ~A()
            {}

            void serialize(SF::Archive &ar)
            {
                ar & a;
            }

            bool operator==(const A &rhs)
            {
                return a == rhs.a;
            }

        private:
            int a;
        };

        class B
        {
        public:
            B() : b(RCF_DEFAULT_INIT)
            {}

            B(float b) : b(b)
            {}

            virtual ~B()
            {}

            void serialize(SF::Archive &ar)
            {
                ar & b;
            }

            bool operator==(const B &rhs)
            {
                return fpt_compare(&b, &rhs.b, 1);
            }

        private:
            float b;
        };

        class C
        {
        public:

            C()
            {}

            C(std::string c) : c(c)
            {}

            virtual ~C()
            {}

            void serialize(SF::Archive &ar)
            {
                ar & c;
            }

            bool operator==(const C &rhs)
            {
                return c == rhs.c;
            }

        private:
            std::string c;
        };

        class D : public A, public B, public C
        {
        public:
            D() : d()
            {
                // for vc6
                d.first = d.second = 0;
            }

            D(std::pair<int,int> d) : A(3), B(3.14), C("pi"), d(d)
            {}

            void serialize(SF::Archive &ar)
            {
                serializeParent( (A*) 0, ar, *this);
                serializeParent( (B*) 0, ar, *this);
                serializeParent( (C*) 0, ar, *this);
                ar & d;
            }

            bool operator==(const D &rhs)
            {
                return
                    d == rhs.d &&
                    A::operator==(rhs) &&
                    B::operator==(rhs) &&
                    C::operator==(rhs);
            }
        private:
            std::pair<int,int> d;
        };

    }

    template<typename IStream, typename OStream>
    void testMultipleInheritance()
    {

        using MI::A;
        using MI::B;
        using MI::C;
        using MI::D;

        D d0(std::make_pair(22,7));

        std::ostringstream os;
        OStream ostr(os);
        ostr << d0 << &d0;
        ostr << (A *)&d0 << (B *)&d0 << (C *)&d0 ;

        {
            std::istringstream is(os.str());
            IStream istr(is);
            for (int i=0; i<5; ++i)
            {
                D d1;
                istr >> d1;
                RCF_CHECK(d0 == d1);
            }
        }

        {
            std::istringstream is(os.str());
            IStream istr(is);
            for (int i=0; i<5; ++i)
            {
                std::auto_ptr<D> apd;
                istr >> apd;
                RCF_CHECK(apd.get() && d0 == *apd);
            }
        }

        {
            std::istringstream is(os.str());
            IStream istr(is);
            D d;
            istr >> d >> d;
            for (int i=0; i<3; ++i)
            {
                std::auto_ptr<A> apd;
                istr >> apd;
                D *pd = dynamic_cast<D *>(apd.get());
                RCF_CHECK(apd.get() && d0 == *pd);
            }
        }

        {
            std::istringstream is(os.str());
            IStream istr(is);
            D d;
            istr >> d >> d;
            for (int i=0; i<3; ++i)
            {
                std::auto_ptr<B> apd;
                istr >> apd;
                D *pd = dynamic_cast<D *>(apd.get());
                RCF_CHECK(apd.get() && d0 == *pd);
            }
        }

        {
            std::istringstream is(os.str());
            IStream istr(is);
            D d;
            istr >> d >> d;
            for (int i=0; i<3; ++i)
            {
                std::auto_ptr<C> apd;
                istr >> apd;
                D *pd = dynamic_cast<D *>(apd.get());
                RCF_CHECK(apd.get() && d0 == *pd);
            }
        }
    }

    template<typename IStream, typename OStream>
    void testSmartPointers()
    {
        using namespace Test;

        {
            // std::auto_ptr

            int *pn = new int;
            *pn = 1;
            std::auto_ptr<int> apn(pn);
            std::auto_ptr<double> apd( (double *) NULL );

            std::ostringstream os;
            OStream ostr(os);
            ostr.enableContext();
            ostr << pn << apn << apd;

            pn = NULL;
            apn.reset();

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> pn >> apn >> apd;
    
            RCF_CHECK( pn != NULL && pn == apn.get() );
            RCF_CHECK( apd.get() == NULL );
        }

        {

            // boost::shared_ptr, boost::scoped_ptr

            int *pn = new int;
            *pn = 2;
            boost::shared_ptr<int> spn1(pn);
            boost::shared_ptr<int> spn2 = spn1;
            boost::shared_ptr<int> spn3 = spn2;
            boost::shared_ptr<double> spd( (double *) NULL );
            boost::scoped_ptr<int> scpn1( new int(53) );
            boost::scoped_ptr<int> scpn2;

            std::ostringstream os;
            OStream ostr(os);
            ostr.enableContext();
            ostr << pn;
            ostr << spn1;
            ostr << spn2;
            ostr << spn3;
            ostr << spd;
            ostr << scpn1;

            pn = NULL;
            spn1.reset();
            spn2.reset();
            spn3.reset();

            std::istringstream is(os.str());
            IStream istr(is);
            istr >> pn >> spn1 >> spn2 >> spn3 >> spd >> scpn2;

            RCF_CHECK( pn != NULL );
            RCF_CHECK( pn == spn1.get() && pn == spn2.get() && pn == spn3.get() );
            RCF_CHECK( spn1.use_count() == 3 );
            RCF_CHECK( spd.get() == NULL );
            RCF_CHECK( scpn1.get() && scpn2.get() && *scpn1 == *scpn2 );
        }
    }

    template<typename IStream, typename OStream>
    void testADL()
    {
        // argument dependent lookup

        Test::K1::A a1(1),a2(0);
        Test::K1::K2::B b1(1),b2(0);

        std::ostringstream os;
        OStream ostr(os);
        ostr << a1 << b1;

        std::istringstream is(os.str());
        IStream istr(is);
        istr >> a2 >> b2;

        RCF_CHECK( a1 == a2 );
        RCF_CHECK( b1 == b2 );
    }

    template<typename IStream, typename OStream>
    void testBoostVariant()
    {
        boost::variant<int, std::string> v1(1);
        boost::variant<int, std::string> v2("one");

        std::ostringstream os;
        SF::OBinaryStream(os) << v1 << v2;

        boost::variant<int, std::string> w1;
        boost::variant<int, std::string> w2;

        std::istringstream is(os.str());
        SF::IBinaryStream(is) >> w1 >> w2 ;

        RCF_CHECK(v1 == w1);
        RCF_CHECK(v2 == w2);
    }

    template<typename IStream, typename OStream>
    void testBoostAny()
    {
        SF::registerType( (int *) NULL, "int");
        SF::registerType( (std::string *) NULL,  "std::string");

        SF::registerAny( (int *) NULL);
        SF::registerAny( (std::string *) NULL);


        boost::any a1( int(1));
        boost::any a2( std::string("one"));

        std::ostringstream os;
        SF::OBinaryStream(os) << a1 << a2;

        boost::any b1;
        boost::any b2;

        std::istringstream is(os.str());
        SF::IBinaryStream(is) >> b1 >> b2 ;

        RCF_CHECK( boost::any_cast<int>(a1) == boost::any_cast<int>(b1));
        RCF_CHECK( boost::any_cast<std::string>(a2) == boost::any_cast<std::string>(b2));
    }

    template<typename IStream, typename OStream>
    void testBoostTuple()
    {
        typedef boost::tuples::tuple<int, std::string, int, std::string> MyTuple;
        MyTuple t1(1, "one", 2, "two");
        MyTuple t2(3, "three", 4, "four");

        // Tuple serialization not yet working on vc6
#if !defined(_MSC_VER) || _MSC_VER >= 1310

        std::ostringstream os;
        SF::OBinaryStream(os) << t1 << t2;

        MyTuple u1;
        MyTuple u2;

        std::istringstream is(os.str());
        SF::IBinaryStream(is) >> u1 >> u2;

        RCF_CHECK( t1 == u1 );
        RCF_CHECK( t2 == u2 );
#else
        RCF_CHECK( 1==0 && "tuple serialization not working on vc6");
#endif

    }

    class X
    {
    public:
        
        X() : n(0)
        {
        }

        int n;
        std::string s;

        void serialize(SF::Archive & ar)
        {
            int version = ar.getArchiveVersion();
            ar & n;
            if (version > 7)
            {
                ar & s;
            }
        }
    };

    enum Numbers { Zero = 0, One = 1 };

    template<typename IStream, typename OStream>
    void testArchiveVersioning()
    {
        X x1;
        X x2;

        {
            std::ostringstream os;
            SF::OBinaryStream obs(os);

            // Header not suppressed.
            obs.suppressVersionStamp(false);
            obs.setRuntimeVersion(6);
            obs.setArchiveVersion(7);
            
            x1.n = 1;
            x1.s = "one";
            obs << x1;

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // No versions, version stamp override.
                ibs.ignoreVersionStamp(false);

                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == "");
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit incorrect versions, version stamp override.
                ibs.ignoreVersionStamp(false);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(8);

                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == "");
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit correct versions, version stamp override.
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(7);
                
                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == "");
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit incorrect versions, no version stamp override.
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(8);

                try
                {
                    ibs >> x2;
                    RCF_CHECK(1==0);
                }
                catch(...)
                {
                    RCF_CHECK(1==1);
                }
            }
        }

        {
            std::ostringstream os;
            SF::OBinaryStream obs(os);

            // Header suppressed.
            obs.suppressVersionStamp(true);
            obs.setRuntimeVersion(6);
            obs.setArchiveVersion(7);
            
            x1.n = 1;
            x1.s = "one";
            obs << x1;

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit incorrect versions, version stamp override.
                ibs.ignoreVersionStamp(false);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(8);

                try
                {
                    ibs >> x2;
                    RCF_CHECK(1==0);
                }
                catch(...)
                {
                    RCF_CHECK(1==1);
                }
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit correct versions, version stamp override.
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(5);
                ibs.setArchiveVersion(7);
                
                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == "");
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                // Explicit incorrect versions, no version stamp override.
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(8);

                try
                {
                    ibs >> x2;
                    RCF_CHECK(1==0);
                }
                catch(...)
                {
                    RCF_CHECK(1==1);
                }
            }
        }

        {
            std::ostringstream os;
            SF::OBinaryStream obs(os);
            obs.setRuntimeVersion(6);
            obs.setArchiveVersion(8);
            x1.n = 1;
            x1.s = "one";
            obs << x1;

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(7);
                
                try
                {
                    ibs >> x2;
                    RCF_CHECK(1==0);
                }
                catch(...)
                {
                    RCF_CHECK(1==1);
                }
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(6);
                ibs.setArchiveVersion(8);
                
                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == x1.s);
            }            

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);
                ibs.ignoreVersionStamp(false);

                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == x1.s);
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);

                ibs >> x2;

                RCF_CHECK(x2.n == x1.n && x2.s == x1.s);
            }
        }

        {
            int n = 1;

            std::ostringstream os;
            SF::OBinaryStream obs(os);
            obs.setRuntimeVersion(2);
            RCF_CHECK(obs.getArchiveVersion() == 0);

            obs << n;

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(1);
                RCF_CHECK(obs.getArchiveVersion() == 0);

                try
                {
                    Numbers numbers;
                    ibs >> numbers;
                    RCF_CHECK(1==0);
                }
                catch(...)
                {
                    RCF_CHECK(1==1);
                }
            }

            {
                std::istringstream is(os.str());
                SF::IBinaryStream ibs(is);
                ibs.ignoreVersionStamp(true);
                ibs.setRuntimeVersion(2);
                RCF_CHECK(obs.getArchiveVersion() == 0);

                Numbers numbers;
                ibs >> numbers;

                RCF_CHECK(numbers == One);
            }            

        }

    }


    template<typename IStream, typename OStream>
    void testAll()
    {
        testAtomic<IStream,OStream>();
        testComposite<IStream,OStream>();
        testEmptyComposite<IStream,OStream>();
    
        testEnum<IStream,OStream>();
        testContainers<IStream,OStream>();
        testGraphs<IStream,OStream>();
        testPolymorphism<IStream,OStream>();
        testMultipleInheritance<IStream,OStream>();
        testSmartPointers<IStream,OStream>();
        testADL<IStream,OStream>();
        
        testBoostAny<IStream,OStream>();
        testArchiveVersioning<IStream,OStream>();

#if defined(__BORLANDC__) || (defined(__MINGW32__) && __GNUC__ < 3)

#else
        testArrays<IStream,OStream>();
#endif

#if (defined(_MSC_VER) && _MSC_VER < 1310) || defined(__BORLANDC__)

#else
        testBoostTuple<IStream,OStream>();
#endif

#if defined(__BORLANDC__)

#else
        testBoostVariant<IStream,OStream>();
#endif

    }

} // namespace Test_Serialization

//***********************************************************

SF_ADL_WORKAROUND(Test_Serialization::Test, A)
SF_ADL_WORKAROUND(Test_Serialization::Test, B)
SF_ADL_WORKAROUND(Test_Serialization::Test, C)
SF_ADL_WORKAROUND(Test_Serialization::Test, D)

SF_ADL_WORKAROUND(Test_Serialization::Test, GeomObject)
SF_ADL_WORKAROUND(Test_Serialization::Test, Rect)

SF_ADL_WORKAROUND(Test_Serialization::Test, TestOne<bool>)
SF_ADL_WORKAROUND(Test_Serialization::Test, Props)
SF_ADL_WORKAROUND(Test_Serialization::Test, Empty)

namespace Test_Serialization { namespace Test {
    typedef TestTwo<std::string, double> TestTwo_string_double;
} }
SF_ADL_WORKAROUND(Test_Serialization::Test, TestTwo_string_double)

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::A)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::B)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::C)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::D)

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::MI::A)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::MI::B)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::MI::C)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::MI::D)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::Rect)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::GeomObject)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::TestOne<bool>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::K1::A)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::K1::K2::B)

typedef Test_Serialization::Test::TestTwo<std::string, double> TestTwo_string_double;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(TestTwo_string_double)

typedef std::pair<int, int> pair_int_int;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(pair_int_int)

typedef std::pair<char, int> pair_char_int;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(pair_char_int)

typedef std::pair<std::string, bool> pair_string_bool;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(pair_string_bool)

typedef std::pair<const std::string, bool> pair_const_string_bool;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(pair_const_string_bool)

typedef std::map<std::string, bool> map_string_bool;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(map_string_bool)

typedef std::vector< std::pair<int, int> > vector_pair_int_int;
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(vector_pair_int_int)

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::Props)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::Empty)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::vector<int>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::list<int>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::set<double>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::vector<Test_Serialization::Test::D*>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<Test_Serialization::MI::A>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<Test_Serialization::MI::B>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<Test_Serialization::MI::C>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<Test_Serialization::MI::D>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_Serialization::Test::TestEnum)

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<int>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(std::auto_ptr<double>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(boost::shared_ptr<int>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(boost::shared_ptr<double>)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(boost::scoped_ptr<int>)

int test_main(int argc, char **argv)
{

    RCF::RcfInitDeinit rcfinitDeinit;

    util::CommandLine::getSingleton().parse(argc, argv);

    printTestHeader(__FILE__);

    using namespace Test_Serialization;

    SF::registerType( (Test::A*) 0, "Test::A");
    SF::registerType( (Test::B*) 0, "Test::B");
    SF::registerType( (Test::C*) 0, "Test::C");

    SF::registerBaseAndDerived( (Test::D*) 0,(Test::A*) 0);
    SF::registerBaseAndDerived( (Test::D*) 0,(Test::B*) 0);
    SF::registerBaseAndDerived( (Test::D*) 0,(Test::C*) 0);

    SF::registerType( (Test::Rect*) 0, "Test::Rect");
    SF::registerBaseAndDerived( (Test::GeomObject*) 0, (Test::Rect*) 0);

    SF::registerType( (MI::D*) 0, "MI::D");

    SF::registerBaseAndDerived( (MI::A*) 0, (MI::D*) 0);
    SF::registerBaseAndDerived( (MI::B*) 0, (MI::D*) 0);
    SF::registerBaseAndDerived( (MI::C*) 0, (MI::D*) 0);

    SF::registerBaseAndDerived( (MI::D*) 0, (MI::D*) 0);
    

    testAll<SF::ITextStream, SF::OTextStream>();
    testAll<SF::IBinaryStream, SF::OBinaryStream>();
    testAll<SF::INativeBinaryStream, SF::ONativeBinaryStream>();
    return boost::exit_success;
}

//#include "RCF/../../src/RCF/util/Trace.cpp"
//#include "RCF/../../src/RCF/util/Platform.cpp"
//
//#include "RCF/../../src/RCF/ByteOrdering.cpp"
//#include "RCF/../../src/RCF/InitDeinit.cpp"
//#include "RCF/../../src/RCF/Exception.cpp"
//#include "RCF/../../src/RCF/Tools.cpp"
//#include "RCF/../../src/RCF/Version.cpp"
//
//#include "RCF/../../src/RCF/RcfBoostThreads/RcfBoostThreads.cpp"
