#ifndef RPC_PARAMETERS_H
#define RPC_PARAMETERS_H


#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/call_traits.hpp>
#include <boost/type_traits/broken_compiler_spec.hpp>
#include <boost/mpl/if.hpp>
#include <vector>
#include <string>


namespace parameters
{
	//////////////////////////////////////////////////////////////////////
	struct io_base 
	{
	};

	enum param_direction {none = 0, in = 1, out = 2, inout = in|out};

    struct binary_data
    {
        binary_data(void * adata, size_t alen)
            : data(adata), len(alen)
        {
        }

        binary_data(const void* adata, size_t alen)
            : data(const_cast<void*>(adata)), len(alen)
        {
        }

        void * data;
        size_t len;
    };

}

BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(parameters::binary_data)

namespace parameters
{
	namespace impl
	{
		typedef char yes_type;
		struct no_type
		{
		   char padding[8];
		};
	
		yes_type is_const_tester(const volatile void*);
		no_type is_const_tester(volatile void *);

		struct is_const_pointer_helper
		{
			template <typename T> struct result_
			{
				static T t;
				enum { value = sizeof(yes_type) == 
					sizeof(is_const_tester(t))
				};
			};
		};

		template <typename T> 
			struct is_const_pointer : is_const_pointer_helper::result_<T>
		{
		};

		template <typename T> 
			struct read_traits
			{
				typedef T type;
			};

		template <typename T>
			struct common_type_traits
			{
				typedef typename read_traits<T>::type type;
			};

		template <typename P, typename T>
			struct pointer_type_traits_impl
			{
				struct type
				{
					type() : value(&temp)
					{
					}

					operator P&()
					{
						return value;
					}

					operator const P& () const
					{
						return value;
					}


					P value;
					typename read_traits<T>::type temp;
				};
			};


		template <typename T>
			struct pointer_type_traits
			{
			typedef typename pointer_type_traits_impl<T, 
				typename boost::remove_const<typename boost::remove_pointer<T>::type>::type>::type type;
			};

		template <class T> struct char_pointer_store
			{
				operator const T *() const
				{
					return value;
				}

				const T * value;
				std::vector<T> buf;
			};

		template <> struct pointer_type_traits <const char *>
			{
				typedef char_pointer_store<char> type;
			};
		template <> struct pointer_type_traits <const wchar_t *>
			{
				typedef char_pointer_store<wchar_t> type;
			};


		template <typename T>
			struct reference_type_traits
			{
				typedef typename read_traits<
					typename boost::remove_const<
					typename boost::remove_reference<T>::type>::type>::type type;
			};


        struct binary_data_server_proxy
        {
			binary_data_server_proxy()
				: temp((void*)0, 0)
			{
			}

			operator const binary_data& () const
			{
				return temp;
			}

			operator binary_data& ()
			{
				return temp;
			}

            void resize(size_t len)
            {
                buf.resize(len);
                temp.data = (buf.empty() ? 0 : &buf.front());
                temp.len  = len; 
            }

            binary_data temp;
			std::vector<char> buf;
		};

		template <> struct read_traits<binary_data>
			{
				typedef binary_data_server_proxy type;
			};


        //template <>
		//struct reference_type_traits<binary_data&>
		//{
        //    typedef binary_data_server_proxy type;
        //};

        //template <>
		//struct reference_type_traits<binary_data const &>
		//{
        //    typedef binary_data_server_proxy type;
        //};


		/////////////////////////////////////////////////////////////////////////////////
		// serialization

		//////////////////////////////////////////////////////////////////////
		// REFERENCE TYPES SERIALIZATION
		//////////////////////////////////////////////////////////////////////

			// IN 
			template <typename T>
				struct rpc_in_reference_type_serializer_client
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
						ar << a;
					}

					template <typename Archive>
					static void Read(Archive& ar, T a)
					{
					}
				};

			template <typename T>
				struct rpc_in_reference_type_serializer_server
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
					}

					template <typename Archive>
					static void Read(Archive& ar, typename reference_type_traits<T>::type& a)
					{
						ar >> a;
					}
				};

			template <typename T, bool bServer>
				struct rpc_in_reference_type_serializer : boost::mpl::if_c
						<
						bServer,
						typename rpc_in_reference_type_serializer_server<T>,
						typename rpc_in_reference_type_serializer_client<T>
						>::type
				{
				};

			
			// OUT
			template <typename T>
				struct rpc_out_reference_type_serializer_client
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
					}

					template <typename Archive>
					static void Read(Archive& ar, T a)
					{
						ar >> a;
					}
				};

			template <typename T>
				struct rpc_out_reference_type_serializer_server
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
						ar << a;
					}

					template <typename Archive>
					static void Read(Archive& ar, typename reference_type_traits<T>::type& a)
					{
					}
				};

			template <typename T, bool bServer>
				struct rpc_out_reference_type_serializer : boost::mpl::if_c
						<
						bServer,
						rpc_out_reference_type_serializer_server<T>,
						rpc_out_reference_type_serializer_client<T>
						>::type
				{
				};


			// IN/OUT

			template <typename T>
			struct rpc_inout_reference_type_serializer_client
			{
				template <typename Archive>
				static void Store(Archive& ar, T a)
				{
					rpc_in_reference_type_serializer_client<T>::Store(ar, a);
				}

				template <typename Archive>
				static void Read(Archive& ar, T a)
				{
					rpc_out_reference_type_serializer_client<T>::Read(ar, a);
				}
			};

			template <typename T>
			struct rpc_inout_reference_type_serializer_server
			{
				// Server side
				template <typename Archive>
				static void Store(Archive& ar, T a)
				{
					rpc_out_reference_type_serializer_server<T>::Store(ar, a);
				}

				template <typename Archive>
				static void Read(Archive& ar, typename reference_type_traits<T>::type& a)
				{
					rpc_in_reference_type_serializer_server<T>::Read(ar, a);
				}
			};

			template <typename T, bool bServer>
				struct rpc_inout_reference_type_serializer : boost::mpl::if_c
						<
						bServer,
						rpc_inout_reference_type_serializer_server<T>,
						rpc_inout_reference_type_serializer_client<T>
						>::type
				{
				};

			template <typename T, param_direction nDirection, bool bServer>
			struct reference_type_serializer : boost::mpl::if_c
					<
					boost::is_const<typename boost::remove_reference<T>::type>::value || (nDirection == in),
					rpc_in_reference_type_serializer<T, bServer>,
					typename boost::mpl::if_c
						<
					    nDirection != out,
					    rpc_inout_reference_type_serializer<T, bServer>, 
					    rpc_out_reference_type_serializer<T, bServer>
						>::type
					>::type
			{
			};

			///////////////////////////////////////////////////////////
			// POINTER TYPE SERIALIZATION
			///////////////////////////////////////////////////////////

			// IN 
			template <typename T>
				struct rpc_in_pointer_type_serializer_client
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
						ar << (char)!!a;
						if (a)
						{
							rpc_in_reference_type_serializer_client<
								boost::remove_pointer<T>::type>::Store(ar, *a);
						}
					}

					template <typename Archive>
					static void Read(Archive& ar, T a)
					{
					}
				};

			template <typename T>
				struct rpc_in_pointer_type_serializer_server
				{
					template <typename Archive>
					static void Store(Archive& ar, const typename pointer_type_traits<T>::type& a)
					{
					}

					template <typename Archive>
					static void Read(Archive& ar, typename pointer_type_traits<T>::type& a)
					{
						char bIsNotNull;
						ar >> bIsNotNull;
						if (bIsNotNull)
						{
							ar >> a.temp;
						}
						else
							a.value = 0;
					}
				};

			template <typename T, bool bServer>
				struct rpc_in_pointer_type_serializer : boost::mpl::if_c
					<
					bServer,
					rpc_in_pointer_type_serializer_server<T>,
					rpc_in_pointer_type_serializer_client<T>
					> :: type
				{
				};


			// OUT
			template <typename T>
				struct rpc_out_pointer_type_serializer_client
				{
					template <typename Archive>
					static void Store(Archive& ar, T a)
					{
						ar << (char)!!a;
					}

					template <typename Archive>
					static void Read(Archive& ar, T a)
					{
						if (a)
							ar >> *a;
					}
				};

			template <typename T>
				struct rpc_out_pointer_type_serializer_server
				{
					template <typename Archive>
					static void Store(Archive& ar, const typename pointer_type_traits<T>::type& a)
					{
						if (a.value)
							ar << *a.value;
					}

					template <typename Archive>
					static void Read(Archive& ar, typename pointer_type_traits<T>::type& a)
					{
						char bIsNotNull;
						ar >> bIsNotNull;
						if (!bIsNotNull)
							a.value = 0;
					}
				};

			template <typename T, bool bServer>
				struct rpc_out_pointer_type_serializer : boost::mpl::if_c
					<
					bServer,
					rpc_out_pointer_type_serializer_server<T>,
					rpc_out_pointer_type_serializer_client<T>
					> :: type
				{
				};


			// IN/OUT

			template <typename T>
			struct rpc_inout_pointer_type_serializer_client
			{
				template <typename Archive>
				static void Store(Archive& ar, T a)
				{
					rpc_in_pointer_type_serializer_client<T>::Store(ar, a);
				}

				template <typename Archive>
				static void Read(Archive& ar, T a)
				{
					rpc_out_pointer_type_serializer_client<T>::Read(ar, a);
				}
			};

			template <typename T>
			struct rpc_inout_pointer_type_serializer_server
			{
				template <typename Archive>
				static void Store(Archive& ar, const typename pointer_type_traits<T>::type& a)
				{
					rpc_out_pointer_type_serializer_server<T>::Store(ar, a);
				}

				template <typename Archive>
				static void Read(Archive& ar, typename pointer_type_traits<T>::type& a)
				{
					rpc_in_pointer_type_serializer_server<T>::Read(ar, a);
				}
			};

			template <typename T, bool bServer>
				struct rpc_inout_pointer_type_serializer : boost::mpl::if_c
					<
					bServer,
					rpc_inout_pointer_type_serializer_server<T>,
					rpc_inout_pointer_type_serializer_client<T>
					> :: type
				{
				};


			template <typename T>
			struct rcp_in_string_serializer_client
			{
				static size_t len_traits(const char * a)
				{
					return strlen(a);
				}
				static size_t len_traits(const wchar_t * a)
				{
					return wcslen(a);
				}

				template <typename Archive>
				static void Store(Archive& ar, const T* a)
				{
					size_t len = size_t(a ? len_traits(a) : -1);
					ar << len;
					if (a)
					{
						ar.write((const char*)a, len * sizeof(T));
					}
				}

				template <typename Archive>
				static void Read(Archive& ar, const T * a)
				{
				}
			};

			template <typename T>
			struct rcp_in_string_serializer_server
			{
				template <typename Archive>
				static void Store(Archive& ar, const typename pointer_type_traits<const T *>::type& a)
				{
				}

				template <typename Archive>
				static void Read(Archive& ar, typename pointer_type_traits<const T *>::type& a)
				{
					size_t len = 0;
					ar >> len;
					if (len != size_t(-1))
					{
						a.buf.resize(len+1);
						if (len)
							ar.read((char*)(&*a.buf.begin()), len * sizeof(T));
						a.buf.back() = 0;
						a.value = &a.buf.front();
					}
					else
						a.value = 0;
				}
			};

			///
		template <>
			struct rpc_in_pointer_type_serializer_client<const char*>
				: public rcp_in_string_serializer_client<char>
			{
			};

		template <>
			struct rpc_in_pointer_type_serializer_client<const wchar_t*>
				: public rcp_in_string_serializer_client<wchar_t>
			{
			};

		template <>
			struct rpc_in_pointer_type_serializer_server<const char*>
				: public rcp_in_string_serializer_server<char>
			{
			};

		template <>
			struct rpc_in_pointer_type_serializer_server<const wchar_t*>
				: public rcp_in_string_serializer_server<wchar_t>
			{
			};

	
			template <typename T, param_direction nDirection, bool bServer>
			struct pointer_type_serializer : boost::mpl::if_c
					<
					(is_const_pointer<T>::value) || (nDirection == in),
					rpc_in_pointer_type_serializer<T, bServer>,
					typename boost::mpl::if_c
						<
						(nDirection != out),
						rpc_inout_pointer_type_serializer<T, bServer>,
						rpc_out_pointer_type_serializer<T, bServer>
						>::type
					>::type
			{
			};




		template <typename T>
			struct common_type_serializer_client
                : rpc_in_reference_type_serializer_client<T&>
			{
			};

		template <typename T>
			struct common_type_serializer_server
                //: rpc_in_reference_type_serializer_server<T&>
			{
			
				template <typename Archive>
				static void Store(Archive& ar, const T& a)
				{
					// Do nothing
				}

				template <typename Archive>
				static void Read(Archive& ar, typename common_type_traits<T>::type& a)
				{
					ar >> a;
				}
			};

		template <typename T, bool bServer>
			struct common_type_serializer : boost::mpl::if_c
					<
					bServer,
					common_type_serializer_server<T>,
					common_type_serializer_client<T>
					>::type
			{
			};


		//////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////
		template <typename T, bool bServer>
			struct in_param_serializer : boost::mpl::if_c
						<
						boost::is_reference<T>::value,
						typename rpc_in_reference_type_serializer<T, bServer>,
						typename boost::mpl::if_c
							<
							boost::is_pointer<T>::value,
							typename rpc_in_pointer_type_serializer<T, bServer>,
							typename common_type_serializer<T, bServer>
							>::type
						>::type
			{
			};


		//////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////
		template <typename T, bool bServer>
			struct out_param_serializer : boost::mpl::if_c
						<
						boost::is_reference<T>::value,
						rpc_out_reference_type_serializer<T, bServer>,
						rpc_out_pointer_type_serializer<T, bServer>
						>::type
			{
			};

		//////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////
		template <typename T, bool bServer>
			struct inout_param_serializer : boost::mpl::if_c
						<
						boost::is_reference<T>::value,
						rpc_inout_reference_type_serializer<T, bServer>,
						rpc_inout_pointer_type_serializer<T, bServer>
						>::type
			{
			};

		//////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////
		template <typename T, bool bServer>
			struct inout_serializer_traits : boost::mpl::if_c
						<
						T::is_in::value,
						typename boost::mpl::if_c
							<
							T::is_out::value,
							typename inout_param_serializer<typename T::type, bServer>,
							typename in_param_serializer<typename T::type, bServer>
							> :: type,
						typename out_param_serializer<typename T::type, bServer>
						> :: type
			{
			};


		//////////////////////////////////////////////////////////////////////
		//  binary data specification
		//////////////////////////////////////////////////////////////////////
		template <>
			struct rpc_in_reference_type_serializer_client<const binary_data&>
			{
				template <typename Archive>
				static void Store(Archive& ar, const binary_data& a)
				{
					ar << a.len;
                    if (a.len)
                        ar.write(a.data, a.len);
				}

				template <typename Archive>
				static void Read(Archive& ar, const binary_data& a)
				{
				}
			};

		template <>
			struct rpc_in_reference_type_serializer_server<const binary_data&>
			{
				template <typename Archive>
				static void Store(Archive& ar, const binary_data& a)
				{
				}

				template <typename Archive>
				static void Read(Archive& ar, 
                    //reference_type_traits<const binary_data&>::type& a)
					binary_data_server_proxy& a)
				{
                    size_t len = 0;
                    ar >> len;
                    a.resize(len);
                    if (len)
                        ar.read(a.temp.data, len);
				}
			};

		template <>
			struct rpc_in_reference_type_serializer_client<binary_data&>
				: rpc_in_reference_type_serializer_client<const binary_data&>
			{
			};

		template <>
			struct rpc_in_reference_type_serializer_server<binary_data&>
				: rpc_in_reference_type_serializer_server<const binary_data&>
			{
			};

		// OUT
		template <>
			struct rpc_out_reference_type_serializer_client<binary_data&>
			{
				template <typename Archive>
				static void Store(Archive& ar, const binary_data& a)
				{
					ar << a.len;
				}

				template <typename Archive>
				static void Read(Archive& ar, binary_data& a)
				{
                    ar >> a.len;
                    if (a.len)
                        ar.read(a.data, a.len);
				}
			};

		template <>
			struct rpc_out_reference_type_serializer_server<binary_data&>
			{
				template <typename Archive>
				static void Store(Archive& ar, const binary_data& a)
				{
					ar << a.len;
                    if (a.len)
                        ar.write(a.data, a.len);
				}

				template <typename Archive>
				static void Read(Archive& ar, 
                    //reference_type_traits<binary_data&>::type& a)
					binary_data_server_proxy& a)
				{
                    size_t len = 0;
                    ar >> len;
                    a.resize(len);
				}
			};

	} // end namespace impl


	// Forward declaration
	template<typename> struct type_traits;

	template <typename T, bool isIn, bool isOut> 
		struct rpc_io_base : public io_base
		{
			typedef rpc_io_base<T, isIn, isOut> param;
			typedef typename boost::mpl::if_c<boost::is_pointer<T>::value, T, 
				typename boost::mpl::if_c<isIn & !isOut, 
				typename boost::call_traits<T>::const_reference, 
				typename boost::call_traits<T>::reference>::type>::type store_type;

			typedef T type;

			struct is_in
			{
				enum { value = isIn };
			};

			struct is_out
			{
				enum { value = isOut };
			};

			rpc_io_base(store_type a) : value(a)
			{
			}

			operator store_type ()
			{
				return value;
			}

			store_type value;

			rpc_io_base<T, isIn, isOut> & operator = (typename boost::call_traits<T>::param_type t)
			{
				value = t;
				return *this;
			}
		};

	template <typename T, bool isIn, bool isOut> 
		struct rpc_io_base_proxy
		{
			typedef typename type_traits<T>::type proxy_type;

			struct is_in
			{
				enum { value = isIn };
			};

			struct is_out
			{
				enum { value = isOut };
			};

			operator typename rpc_io_base<T, is_in::value, is_out::value>::param ()
			{
				return rpc_io_base<T, is_in::value, is_out::value>::param(value);
			}

			operator proxy_type& ()
			{
				return value;
			}

			proxy_type value;
		};

	template <typename T> struct rpc_in
		{
			typedef typename rpc_io_base<T, true, false>::param param;
		};

	template <typename T> struct rpc_out
		{
			typedef typename rpc_io_base<T, false, true>::param param;
		};

	template <typename T> struct rpc_inout
		{
			typedef typename rpc_io_base<T, true, true>::param param;
		};
	
	template <typename T>
		struct inout_type_traits : rpc_io_base_proxy<typename T::type, 
			T::is_in::value, T::is_out::value>
		{
		typedef typename rpc_io_base_proxy<typename T::type, 
			T::is_in::value, T::is_out::value> type;
		};

	template <typename T>
		struct type_traits : boost::mpl::if_c
				<
				boost::is_base_and_derived<io_base, T>::value,
				typename inout_type_traits<T>,
				typename boost::mpl::if_c
					<
					boost::is_reference<T>::value,
					typename impl::reference_type_traits<T>,
					typename boost::mpl::if_c
						<
						boost::is_pointer<T>::value,
						typename impl::pointer_type_traits<T>,
						typename impl::common_type_traits<T>
						>::type
					>::type
				>::type
		{
		};

	/// SERIALIZE TRAITS
	template <typename T, param_direction nDirection, bool bServer>
		struct serialize_traits
			: boost::mpl::if_c
				<
				boost::is_reference<T>::value,
				typename impl::reference_type_serializer<T, nDirection, bServer>,
				typename boost::mpl::if_c
					<
					boost::is_pointer<T>::value,
					typename impl::pointer_type_serializer<T, nDirection, bServer>,
					typename impl::common_type_serializer<T, bServer>
					>::type
				>::type
		{
		};

	template <typename T, param_direction nDirection, bool bServer>
		struct serialize_traits_ex
			: boost::mpl::if_c
				<
				boost::is_base_and_derived<io_base, T>::value,
				impl::inout_serializer_traits<T, bServer>,
				serialize_traits<T, nDirection, bServer>
				>::type
		{
		};

}


#endif // RPC_PARAMETERS_H
