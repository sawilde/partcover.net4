#ifndef RPC_IFS_PARAMETERS_H
#define RPC_IFS_PARAMETERS_H

#include "parameters.h"

#ifdef RPC_NO_IFS_PARAMETERS_SPECIALIZATION
	#define RPC_DECLARE_IFS_PARAMETERS_SPECIALIZATION(ifs_type)
#else


namespace parameters
{
	namespace impl
	{


		template <class T, class ClientProxy>
		struct ifs_pointer_serialize_traits_ex_client: public parameters::impl::rpc_out_pointer_type_serializer_client<T>
		{
			template <typename Archive, typename P>
			static void Read(Archive& ar, P **a)
			{
				if (a)
				{
					rpclib::Token t;
					ar >> t;
					*a = (P *)ClientProxy::library_type::CreateClientObject(ar, t, RPC_ID(P));
				}
			}
		};

		template <class I>
		struct rpc_inout_ifs_pointer_type_serializer_server: public rpc_out_pointer_type_serializer_server<I>
		{
			template <typename Archive, class T>
			static void Store(Archive& ar, const T &a)
			{
				if (a.value)
					ar << (rpclib::Token)*a.value;
			}
		};


		struct ifs_pointer_type_serializer_client
		{
			template <typename Archive, class T>
			static void Store(Archive& ar, const T *a)
			{
				rpclib::Token t = a ? static_cast<const rpclib::RPC_CLIENT_PROXY(IEmptyBase)<Archive, T> *>(a)->m_tok : 0;
				ar << t;
			}

			template <typename Archive, class T>
			static void Read(Archive& ar, T * &)
			{
			}
		};

		struct ifs_pointer_type_serializer_server
		{
			template <typename Archive, class T>
			static void Store(Archive& ar, const T *a)
			{
			}

			template <typename Archive, class T>
			static void Read(Archive& ar, T *&a)
			{
				rpclib::Token t;
				ar >> t;
				a = reinterpret_cast<T *>(t);
			}
		};

	}
}


#define RPC_DECLARE_IFS_PARAMETERS_SPECIALIZATION(ifs_type) \
template<>\
struct serialize_traits_ex_client<ifs_type **>\
{\
	template <class P, parameters::param_direction nDirection>\
	struct inner: public parameters::impl::ifs_pointer_serialize_traits_ex_client<ifs_type **, P>\
	{\
	};\
};\
\
namespace parameters\
{\
	namespace impl\
	{\
		template <> struct pointer_type_traits<ifs_type **>\
		{\
			typedef pointer_type_traits_impl<ifs_type **, ifs_type *>::type type;\
		};\
		template <>\
		struct rpc_inout_pointer_type_serializer_server<ifs_type **>: public rpc_inout_ifs_pointer_type_serializer_server<ifs_type **>\
		{\
		};\
		template <> struct pointer_type_traits <const ifs_type *>\
		{\
			typedef const ifs_type * type;\
		};\
		template <> struct pointer_type_traits<ifs_type *>\
		{\
			typedef ifs_type * type;\
		};\
		template <>\
		struct rpc_inout_pointer_type_serializer_client<ifs_type *>: public ifs_pointer_type_serializer_client\
		{\
		};\
		template <>\
		struct rpc_in_pointer_type_serializer_client<const ifs_type *>: public ifs_pointer_type_serializer_client\
		{\
		};\
		template <>\
		struct rpc_inout_pointer_type_serializer_server<ifs_type *>: public ifs_pointer_type_serializer_server\
		{\
		};\
		template <>\
		struct rpc_in_pointer_type_serializer_server<const ifs_type *>: public ifs_pointer_type_serializer_server\
		{\
		};\
	}\
}

#endif


#endif // RPC_IFS_PARAMETERS_H
