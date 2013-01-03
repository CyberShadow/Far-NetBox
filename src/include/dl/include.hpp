/*
Авторские права (c) февраль 2012,
     Константин Масленников  (mkgorel2@gmail.ru), 
     Сергей Харченко         (sx80@mail.ru).

Данная лицензия разрешает лицам, получившим копию данного  программного обеспечения и  сопутствующей
документации  (в  дальнейшем  именуемыми  «Программное  Обеспечение»),   безвозмездно   использовать 
Программное Обеспечение без ограничений, включая неограниченное право на использование, копирование,
изменение,  добавление,  публикацию,  распространение,   сублицензирование   и/или   продажу   копий
Программного Обеспечения, также как и лицам, которым предоставляется данное Программное Обеспечение,
при соблюдении следующих условий:

Указанное выше уведомление об авторском праве и данные условия должны быть включены во все копии или
значимые части данного Программного Обеспечения.

ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, ЯВНО  ВЫРАЖЕННЫХ
ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ ГАРАНТИЯМИ ТОВАРНОЙ  ПРИГОДНОСТИ, СООТВЕТСТВИЯ  ПО 
ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ НАРУШЕНИЙ ПРАВ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ 
НЕ НЕСУТ ОТВЕТСТВЕННОСТИ ПО ИСКАМ О ВОЗМЕЩЕНИИ УЩЕРБА, УБЫТКОВ ИЛИ ДРУГИХ ТРЕБОВАНИЙ ПО  ДЕЙСТВУЮЩИМ
КОНТРАКТАМ,  ДЕЛИКТАМ  ИЛИ  ИНОМУ,  ВОЗНИКШИМ  ИЗ,  ИМЕЮЩИМ  ПРИЧИНОЙ  ИЛИ  СВЯЗАННЫМ С  ПРОГРАММНЫМ
ОБЕСПЕЧЕНИЕМ  ИЛИ  ИСПОЛЬЗОВАНИЕМ  ПРОГРАММНОГО  ОБЕСПЕЧЕНИЯ  ИЛИ  ИНЫМИ  ДЕЙСТВИЯМИ  С  ПРОГРАММНЫМ 
ОБЕСПЕЧЕНИЕМ.
*/

#pragma once

#include <dl/preprocessor.hpp>
#include <cassert>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <typeinfo>

#ifndef DL_DEBUG_LEVEL // 0 1 2
  #if defined(_DEBUG)
    #define DL_DEBUG_LEVEL 1
  #else
    #define DL_DEBUG_LEVEL 0
  #endif
#endif

/*--- DL interface -------------------------------------------------------------------------------*/

#define DL_BLOCK(content)       DLXR_BLOCK(content)
#define DL_NS_BLOCK(ns_content) \
	DL_BLOCK\
	(\
		DL_NAMESPACE( DLXI_1_OF_S2(ns_content) )\
		(\
			DLXI_2_OF_S2(ns_content)\
		)\
	)

#define DL_NAMESPACE(/*names*/...)/*(content)*/ DLXI_TWO_ARG_MACRO(DLXR_NAMESPACE, (__VA_ARGS__))
#define DL_RECORD(name)/*(fields)*/             DLXI_TWO_ARG_MACRO(DLXR_RECORD,    name)
#define DL_INTERFACE(name)/*(methods)*/         DLXI_TWO_ARG_MACRO(DLXR_INTERFACE, name)
#define DL_LIBRARY(name)/*(methods)*/           DLXI_TWO_ARG_MACRO(DLXR_LIBRARY,   name)
#define DL_C_LIBRARY(name)/*(methods)*/         DLXI_TWO_ARG_MACRO(DLXR_C_LIBRARY, name)
#define DL_INTERFACE_DECL(name)                 DLXR_INTERFACE_FWD(name)

#define DL_EXPORT(dl_library, implementation) extern"C" DLX_DLL_EXPORT ::dlx::library<dl_library>::ftable const* DLX_CALL dynamic_library() \
{\
	return &::dlx::library<dl_library>::globals<implementation>::ftable;\
}\

#define DL_MAX_ARGS_COUNT 10
#define DL_VOID_TYPE_void ,
#define DL_VOID_TYPE_Void DL_VOID_TYPE_void
#define DL_VOID_TYPE_VOID DL_VOID_TYPE_void

/*--- Realization (implementation) ---------------------------------------------------------------*/

#define DLXR_BLOCK(content) DLXR_BLOCK_A(PP_SEQ_TO_VEC(,content))
#define DLXR_BLOCK_A(content) PP_TPLVEC_FOR(content, DLXR_PROCESS_INFO_, ,I)

#define DLXR_PROCESS_INFO_4(r,i,d, element,ns_s,name,data) PP_CAT( DLXR_PROCESS, element )\
	( r, PP_SEQ_TO_VEC(,ns_s), DLXI_BUILD_NS(r, ns_s), name, data )

#define DLXR_NAMESPACE(ns,cnt) PP_SEQ_FOR(cnt, DLXR_NS_ADD, PP_VEC_DATA(PP_TPL_TO_VEC(ns)), I)
#define DLXR_NS_ADD(r,i,d,x) ((PP_TPL(1,x), d PP_TPL(2,x), PP_TPL(3,x), PP_TPL(4,x)))

#define DLXR_RECORD(name,fields)     (( _REC,         ,name, PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR(fields, DLXR_REC_,,I)) ))

#define DLXR_INTERFACE_FWD(name)     (( _IFACE_FWD,   ,name, ))

#define DLXR_INTERFACE(name,methods) (( _IFACE,       ,name, PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR(methods, DLXR_IFACE_,,I)) )) 

#define DLXR_LIBRARY(name,methods)   (( _LIB,         ,name, PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR(methods, DLXR_IFACE_,,I)) ))

#define DLXR_C_LIBRARY(name,methods) (( _CLIB,        ,name, PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR(methods, DLXR_CLIB_,,I)) ))


#define DLXR_REC_2(r,i,d, type,name)       (( type, name, ))
#define DLXR_REC_3(r,i,d, type,name,value) (( type, name, value))

#define DLXR_IFACE_2(r,i,d, type,name)      DLXR_IFACE_3(r,i,d,type,name,)
#define DLXR_IFACE_3(r,i,d, type,name,args) (( type, (name, PP_CAT(PP_CAT(dl_,i),name)), \
	PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR_R(r)(r, args, DLXR_ARGS_,,I)) ))

#define DLXR_CLIB_3(r,i,d, type,call,name)      DLXR_CLIB_4(r,i,d,call,type,name,)
#define DLXR_CLIB_4(r,i,d, type,call,name,args) (( type, call, PP_TPL_MACRO_R(r)(name,DLXR_CLIB_N_,i), \
	PP_SEQ_TO_VEC(,PP_TPLSEQ_FOR_R(r)(r, args, DLXR_ARGS_,,I)) ))

#define DLXR_CLIB_N_1(n, name)     DLXR_CLIB_N_2(n, name, #name)
#define DLXR_CLIB_N_2(n, name,str) ( name, PP_CAT(PP_CAT(dl_,n),name), str )

#define DLXR_ARGS_0(r,i,d)
#define DLXR_ARGS_1(r,i,d, type)            (( type, PP_CAT(_,i), ))
#define DLXR_ARGS_2(r,i,d, type,name)       (( type, name, ))
#define DLXR_ARGS_3(r,i,d, type,name,value) (( type, name, value ))


#define DLXR_WRAPPER_ARGS_3(r,i,d, type,name,value) ::dlx::arg<type>::t name value
#define DLXR_FN_ARG_TYPES_3(r,i,d, type,name,value) type
#define DLXR_FN_ARG_NAMES_3(r,i,d, type,name,value) name


#define DLXR_PROCESS_REC(r, ns_v, ns, name, data) \
	DLXI_NAMESPACE(r, ns_v,\
		DLX_PACKED(	struct name\
		{\
			explicit name(::dlx::no_init_t) {}\
			explicit name(PP_TPLVEC_ENUM_R(r)(r, data, DLXR_REC_C_ARG_,,I))\
				: PP_TPLVEC_ENUM_R(r)(r, data, DLXR_REC_C_INIT_,,I)\
			{}\
			PP_TPLVEC_FOR_R(r)(r, data, DLXR_REC_FIELD_,,I)\
		})\
		DLX_STATIC_CHECK_SIZE( ns::name, PP_TPLVEC_FOR_R(r)(r, data, DLXR_REC_F_SIZE_,,I) +0 )\
	)\
	namespace dlx\
	{\
		template<> struct rval< ns::name >\
		{\
			DLX_NONCOPYABLE(rval)\
		public:\
			typedef rval< ns::name > t;\
			\
			rval() : val_( ::dlx::no_init )       {}\
			operator ns::name const&() const      { return val_; }\
			void operator=( ns::name const& val ) { val_ = val; }\
			\
		private:\
			ns::name val_;\
		};\
	}
#define DLXR_REC_C_ARG_3(r,i,d, type,name,value) ::dlx::arg<type>::t name##_ value
#define DLXR_REC_C_INIT_3(r,i,d, type,name,value) name(name##_)
#define DLXR_REC_FIELD_3(r,i,d, type,name,value)  type name;
#define DLXR_REC_F_SIZE_3(r,i,d, type,name,value) sizeof(type) +


#define DLXR_PROCESS_CLIB(r, ns_v, ns, name, data) \
	DLXI_NAMESPACE(r, ns_v,\
		class name\
		{\
			DLX_NONCOPYABLE( name )\
		public:\
			explicit name(std::string const& path)\
				: dl_lib_(path)\
			{\
				::dlx::dynamic_lib::functions fn(dl_lib_, path);\
				PP_TPLVEC_FOR_R(r)(r, data, DLXR_CLIB_C_INIT_,,I)\
			}\
			PP_TPLVEC_FOR_R(r)(r, data, DLXR_CLIB_METHODS_,,I)\
		private:\
			::dlx::dynamic_lib dl_lib_;\
			PP_TPLVEC_FOR_R(r)(r, data, DLXR_CLIB_FIELD_,,I)\
		};\
	)
#define DLXR_CLIB_C_INIT_4(r,i,d, type,call,names,args)\
	fn( PP_TPL(3,names), PP_TPL(2,names) );
#define DLXR_CLIB_METHODS_4(r,i,d, type,call,names,args) \
	type PP_TPL(1,names) (PP_TPLVEC_ENUM_R(r)(r, args, DLXR_WRAPPER_ARGS_,,I)) const DLX_NOEXCEPT \
	{\
		return PP_TPL(2,names) (PP_TPLVEC_ENUM_R(r)(r, args, DLXR_FN_ARG_NAMES_,,I));\
	}
#define DLXR_CLIB_FIELD_4(r,i,d, type,call,names,args)\
	type (call* PP_TPL(2,names)) (PP_TPLVEC_ENUM_R(r)(r, args, DLXR_FN_ARG_TYPES_,,I));


#define DLXR_PROCESS_IFACE_FWD(r, ns_v, ns, name, data) \
	DLXI_NAMESPACE(r, ns_v,\
		class name;\
	)\
	namespace dlx\
	{\
		template<> struct is_fwd_interface< ns::name > : true_ {};\
	}


#define DLXR_PROCESS_IFACE(r, ns_v, ns, name, data) \
	DLXI_NAMESPACE(r, ns_v,\
		class name;\
	)\
	namespace dlx\
	{\
		template<> struct is_interface< ns::name > : true_ {};\
	}\
	DLXI_NAMESPACE(r, ns_v,\
	namespace dl_ {\
	namespace PP_CAT(dl_, name)\
	{\
		typedef void dl_object;\
		typedef ::dlx::error<>::info const dl_error;\
		struct dl_ftable\
		{\
			::dlx::hash::value (DLX_CALL* dl_get_hash)()           DLX_NOEXCEPT;\
			void               (DLX_CALL* dl_add_ref) (dl_object*) DLX_NOEXCEPT;\
			int                (DLX_CALL* dl_release) (dl_object*) DLX_NOEXCEPT;\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_FT_FUNC_,,I)\
		};\
		DLX_STATIC_CHECK_SIZE(dl_ftable, (3 + PP_VEC_SIZE(data)) * ::dlx::pointer_size )\
		inline ::dlx::hash::value DLX_CALL dl_hash() DLX_NOEXCEPT\
		{\
			namespace dl_ = ::dlx::hash;\
			typedef dl_::initial dl_hash0;\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_HASH_,,I)\
			return PP_CAT(dl_hash,PP_VEC_SIZE(data))::result;\
		}\
		template<typename DL_T> struct dl_funcs\
		{\
			static void DLX_CALL dl_add_ref(dl_object* dl_obj) DLX_NOEXCEPT\
			{\
				reinterpret_cast<DL_T*>(dl_obj)->dl_add_ref();\
			}\
			static int DLX_CALL dl_release(dl_object* dl_obj) DLX_NOEXCEPT\
			{\
				return reinterpret_cast<DL_T*>(dl_obj)->dl_release();\
			}\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_FUNCS_,,I)\
		};\
		DLX_ON_DEBUG_1\
		(\
			template<typename DL_T1> struct dl_funcs< ::dlx::reference<DL_T1> >\
			{\
				typedef ::dlx::reference<DL_T1> DL_T;\
				static void DLX_CALL dl_add_ref(dl_object* dl_obj) DLX_NOEXCEPT\
				{\
					reinterpret_cast<DL_T*>(dl_obj)->dl_add_ref();\
				}\
				static int DLX_CALL dl_release(dl_object* dl_obj) DLX_NOEXCEPT\
				{\
					return reinterpret_cast<DL_T*>(dl_obj)->dl_release();\
				}\
				PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_FUNCS_, ->get() ,I)\
			};\
		)\
		template<typename T = void> struct dl_globals\
		{\
			static const dl_ftable   null_ftable;\
			static const dl_ftable   shared_ftable;\
			static const dl_ftable   ref_ftable;\
			static char const* const qname;\
		};\
		template<typename DL_T> dl_ftable const dl_globals<DL_T>::null_ftable =\
		{\
			&dl_hash\
			, &::dlx::ftable::null<>::add_ref\
			, &::dlx::ftable::null<>::release\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_NULL_FUNCS_,,I)\
		};\
		template<typename DL_T> dl_ftable const dl_globals<DL_T>::shared_ftable =\
		{\
			&dl_hash\
			, &dl_funcs<DL_T>::dl_add_ref\
			, &dl_funcs<DL_T>::dl_release\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_NULL_SH_FUNCS_,,I)\
		};\
		template<typename DL_T> dl_ftable const dl_globals<DL_T>::ref_ftable =\
		{\
			&dl_hash\
			DLX_ON_NODBG_1\
			(\
				, &::dlx::ftable::null<>::add_ref\
				, &::dlx::ftable::null<>::release\
			)\
			DLX_ON_DEBUG_1\
			(\
				, &dl_funcs<DL_T>::dl_add_ref\
				, &dl_funcs<DL_T>::dl_release\
			)\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_NULL_SH_FUNCS_,,I)\
		};\
		template<typename T> char const* const dl_globals<T>::qname = PP_STR(ns::name);\
	}})\
	DLXI_NAMESPACE(r, ns_v,\
		class name\
		{\
		public:\
			name( ::dlx::nullptr_t = ::dlx::null_ptr ) DLX_NOEXCEPT\
				: dl_ftable( &dl_::PP_CAT(dl_,name)::dl_globals<>::null_ftable )\
				, dl_object( 0 )\
			{}\
			template<typename T> name( ::dlx::shared<T> const& s ) DLX_NOEXCEPT\
				: dl_ftable( &dl_::PP_CAT(dl_,name)::dl_globals< ::dlx::shared_class<T> >::shared_ftable )\
				, dl_object( ::dlx::shared_class<T>::get(s) )\
			{\
				dl_ftable->dl_add_ref(dl_object);\
			}\
			template<typename T> name(::dlx::reference<T> const& ref) DLX_NOEXCEPT\
				DLX_ON_NODBG_1\
				(\
					: dl_ftable( &dl_::PP_CAT(dl_,name)::dl_globals< T >::ref_ftable )\
					, dl_object( ref.get() )\
				)\
				DLX_ON_DEBUG_1\
				(\
					: dl_ftable( &dl_::PP_CAT(dl_,name)::dl_globals< ::dlx::reference<T> >::ref_ftable )\
					, dl_object( const_cast< ::dlx::reference<T>* >(&ref) )\
				)\
			{\
				DLX_ON_DEBUG_1( dl_ftable->dl_add_ref(dl_object); )\
			}\
			name(name const& other) DLX_NOEXCEPT\
				: dl_ftable( other.dl_ftable )\
				, dl_object( other.dl_object )\
			{\
				DLX_ASSERT_1( dl_::PP_CAT(dl_,name)::dl_hash() == other.dl_ftable->dl_get_hash() )\
				dl_ftable->dl_add_ref(dl_object);\
			}\
			name& operator=(name const& other) DLX_NOEXCEPT\
			{\
				DLX_ASSERT_1( dl_ftable->dl_get_hash() == other.dl_ftable->dl_get_hash() )\
				other.dl_ftable->dl_add_ref( other.dl_object );\
				dl_ftable->dl_release(dl_object);\
				dl_ftable = other.dl_ftable;\
				dl_object = other.dl_object;\
				return *this;\
			}\
			~name() DLX_NOEXCEPT\
			{\
				dl_ftable->dl_release(dl_object);\
			}\
			name const* operator->() const DLX_NOEXCEPT\
			{\
				return this;\
			}\
			operator ::dlx::safe_bool<name>::_() const DLX_NOEXCEPT\
			{\
				return dl_object ? &::dlx::safe_bool<name>::true_ : 0;\
			}\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_METHODS_,name,I)\
		private:\
			friend ::dlx::hash::value (::dlx::hash::hash_of< ns::name >)( ns::name const& );\
			friend int ::dlx::references_of< ns::name >( ns::name const& );\
			dl_::PP_CAT(dl_,name)::dl_ftable const* dl_ftable;\
			dl_::PP_CAT(dl_,name)::dl_object*       dl_object;\
		};\
		DLX_STATIC_CHECK_SIZE(name, 2 * ::dlx::pointer_size)\
	)\
	namespace dlx {\
	namespace hash\
	{\
		template<> inline value hash_of< ns::name >( ns::name const& i )\
		{\
			return i.dl_ftable->dl_get_hash();\
		}\
		template<> inline value hash_of< ns::name >()\
		{\
			return ns::dl_::PP_CAT(dl_,name)::dl_hash();\
		}\
	}}\

#define DLXR_IFACE_FT_FUNC_3(r,i,d, type,name,args) \
	DLXI_VOID_SELECT(type,DLXR_IFACE_FT_FUNC_PROC,DLXR_IFACE_FT_FUNC_FUNC)(r,type,name,args)
#define DLXR_IFACE_FT_FUNC_PROC(r,type,name,args) dl_error* (DLX_CALL* PP_TPL(2,name))\
	(dl_object* PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_,,I) ) DLX_NOEXCEPT;
#define DLXR_IFACE_FT_FUNC_FUNC(r,type,name,args) dl_error* (DLX_CALL* PP_TPL(2,name))\
	(dl_object* PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_,,I) ,::dlx::rval<type>::t& ) DLX_NOEXCEPT;
#define DLXR_IFACE_FN_TYPES_3(r,i,d, type,name,value) PP_COMMA()::dlx::arg<type>::t
#define DLXR_IFACE_FN_TYPES_ARGS_3(r,i,d, type,name,value) PP_COMMA()::dlx::arg<type>::t name
#define DLXR_IFACE_FN_TYPES_ARGS__F3(r,i,d, type,name,value) ::dlx::arg<type>::t name
#define DLXR_IFACE_FN_TYPES_ARGS_VALUES_3(r,i,d, type,name,value) PP_COMMA()::dlx::arg<type>::t name value
#define DLXR_IFACE_FN_TYPES_ARGS_VALUES__F3(r,i,d, type,name,value) ::dlx::arg<type>::t name value
#define DLXR_IFACE_FN_ARGS_3(r,i,d, type,name,value) PP_COMMA()name
#define DLXR_IFACE_FN_ARGS__F3(r,i,d, type,name,value) name
#define DLXR_IFACE_HASH_3(r,i,d, type,name,args) typedef PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_H1_,,I)\
	dl_::fn<PP_CAT(dl_hash, PP_DEC(i)), type, PP_VEC_SIZE(args)> PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_H2_,,I) PP_CAT(dl_hash, i);
#define DLXR_IFACE_H1_3(r,i,d, type,name,value) dl_::arg<
#define DLXR_IFACE_H2_3(r,i,d, type,name,value) PP_COMMA() type>
#define DLXR_IFACE_FUNCS_3(r,i,d, type,name,args) DLXI_VOID_SELECT(type,DLXR_IFACE_FUNCS_PROC,DLXR_IFACE_FUNCS_FUNC)(r,d,type,name,args)
#define DLXR_IFACE_FUNCS_PROC(r,data,type,name,args) static dl_error* DLX_CALL PP_TPL(2,name)\
	(dl_object* dl_obj PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_ARGS_,,I)) DLX_NOEXCEPT\
{\
	try\
	{\
		reinterpret_cast<DL_T*>(dl_obj)data->PP_TPL(1,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_ARGS_,,FI));\
		return 0;\
	}\
	catch (...)\
	{\
		return ::dlx::error<>::create();\
	}\
}
#define DLXR_IFACE_FUNCS_FUNC(r,data,type,name,args) static dl_error* DLX_CALL PP_TPL(2,name)\
	(dl_object* dl_obj PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_ARGS_,,I) ,::dlx::rval<type>::t& dl_rval) DLX_NOEXCEPT\
{\
	try\
	{\
		dl_rval = reinterpret_cast<DL_T*>(dl_obj)data->PP_TPL(1,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_ARGS_,,FI) );\
		return 0;\
	}\
	catch (...)\
	{\
		return ::dlx::error<>::create();\
	}\
}
#define DLXR_IFACE_NULL_FUNCS_3(r,i,d, type,name,args) DLXI_VOID_SELECT(type,DLXR_IFACE_NULL_FUNCS_PROC,DLXR_IFACE_NULL_FUNCS_FUNC)(r,type,args)
#define DLXR_IFACE_NULL_FUNCS_PROC(r,type,args) PP_COMMA() &::dlx::ftable::null<>::func<\
	dl_object* PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_,,I) >
#define DLXR_IFACE_NULL_FUNCS_FUNC(r,type,args) PP_COMMA() &::dlx::ftable::null<>::func<\
	dl_object* PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_,,I) ,::dlx::rval<type>::t& >
#define DLXR_IFACE_NULL_SH_FUNCS_3(r,i,d, type,name,args) PP_COMMA() &dl_funcs<DL_T>::PP_TPL(2,name)
#define DLXR_IFACE_METHODS_3(r,i,d, type,name,args) DLXI_VOID_SELECT(type,DLXR_IFACE_METHODS_PROC,DLXR_IFACE_METHODS_FUNC)(r,type,name,args,d)
#define DLXR_IFACE_METHODS_FUNC(r,type,name,args,cname) type PP_TPL(1,name) (PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_ARGS_VALUES_,,FI)) const\
	{\
		::dlx::rval<type>::t dl_rval;\
		if (::dlx::error<>::info const* err = dl_ftable->PP_TPL(2,name)(dl_object PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_ARGS_,,I) ,dl_rval))\
			::dlx::error<>::throw_ex(err, dl_::PP_CAT(dl_,cname)::dl_globals<>::qname, PP_STR(PP_TPL(1,name)));\
		return dl_rval;\
	}
#define DLXR_IFACE_METHODS_PROC(r,type,name,args,cname) void PP_TPL(1,name) (PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_TYPES_ARGS_,,FI)) const\
	{\
		if (::dlx::error<>::info const* err = dl_ftable->PP_TPL(2,name)(dl_object PP_TPLVEC_FOR_R(r)(r,args,DLXR_IFACE_FN_ARGS_,,I)))\
			::dlx::error<>::throw_ex(err, dl_::PP_CAT(dl_,cname)::dl_globals<>::qname, PP_STR(PP_TPL(1,name)));\
	}


#define DLXR_PROCESS_LIB(r, ns_v, ns, name, data) \
	DLXI_NAMESPACE(r, ns_v,\
	namespace dl_ {\
	namespace PP_CAT(dl_, name)\
	{\
		typedef ::dlx::error<>::info const dl_error;\
		struct dl_ftable\
		{\
			::dlx::hash::value (DLX_CALL* dl_get_hash)() DLX_NOEXCEPT;\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_LIB_FT_FUNC_,,I)\
		};\
		DLX_STATIC_CHECK_SIZE(dl_ftable, (1 + PP_VEC_SIZE(data)) * ::dlx::pointer_size )\
		inline ::dlx::hash::value DLX_CALL dl_hash() DLX_NOEXCEPT\
		{\
			namespace dl_ = ::dlx::hash;\
			typedef dl_::initial dl_hash0;\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_IFACE_HASH_,,I)\
			return PP_CAT(dl_hash,PP_VEC_SIZE(data))::result;\
		}\
		template<typename DL_T> struct dl_funcs\
		{\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_LIB_FUNCS_,,I)\
		};\
		template<typename DL_T = void> struct dl_globals\
		{\
			static const dl_ftable   ftable;\
			static char const* const qname;\
		};\
		template<typename DL_T> dl_ftable const dl_globals<DL_T>::ftable =\
		{\
			&dl_hash\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_LIB_FTABLE_FUNCS_,,I)\
		};\
		template<typename DL_T> char const* const dl_globals<DL_T>::qname = PP_STR(ns::name);\
	}})\
	DLXI_NAMESPACE(r, ns_v,\
		class name\
		{\
			DLX_NONCOPYABLE(name)\
		public:\
			name(std::string const& dll_path)\
				: dl_lib(dll_path)\
			{\
				::dlx::dynamic_lib::functions fn(dl_lib, dll_path);\
				dl_::PP_CAT(dl_,name)::dl_ftable const* (DLX_CALL * lib_fn)() DLX_NOEXCEPT;\
		        fn("_dynamic_library@0", lib_fn, std::nothrow);\
				if (lib_fn == 0)\
					fn("dynamic_library@0", lib_fn, std::nothrow);\
                if (lib_fn == 0)\
                    fn("dynamic_library", lib_fn, std::nothrow);\
				if (lib_fn == 0)\
					::dlx::throw_ex< ::dlx::runtime_error >( std::string("library '") + dll_path + "' is not a DL-library");\
				dl_ftable = lib_fn();\
				DLX_ASSERT_2(dl_ftable != 0);\
				DLX_ASSERT_1(dl_::PP_CAT(dl_,name)::dl_hash() == dl_ftable->dl_get_hash());\
			}\
			PP_TPLVEC_FOR_R(r)(r,data,DLXR_LIB_METHODS_,name,I)\
		private:\
			friend ::dlx::hash::value (::dlx::hash::hash_of< ns::name >)( ns::name const& );\
			::dlx::dynamic_lib                      dl_lib;\
			dl_::PP_CAT(dl_,name)::dl_ftable const* dl_ftable;\
		};\
	)\
	namespace dlx\
	{\
		template<> struct library< ns::name >\
		{\
			typedef ns::dl_::PP_CAT(dl_,name)::dl_ftable ftable;\
			template<typename T> struct globals : ns::dl_::PP_CAT(dl_,name)::dl_globals<T> {};\
		};\
		namespace hash\
		{\
			template<> inline value hash_of< ns::name >( ns::name const& i )\
			{\
				return i.dl_ftable->dl_get_hash();\
			}\
			template<> inline value hash_of< ns::name >()\
			{\
				return ns::dl_::PP_CAT(dl_,name)::dl_hash();\
			}\
		}\
	}
#define DLXR_LIB_FT_FUNC_3(r,i,d, type,name,args) \
	DLXI_VOID_SELECT(type,DLXR_LIB_FT_FUNC_PROC,DLXR_LIB_FT_FUNC_FUNC)(r,type,name,args)
#define DLXR_LIB_FT_FUNC_PROC(r,type,name,args) dl_error* (DLX_CALL* PP_TPL(2,name))\
	(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_TYPES_,,IL) ) DLX_NOEXCEPT;
#define DLXR_LIB_FT_FUNC_FUNC(r,type,name,args) dl_error* (DLX_CALL* PP_TPL(2,name))\
	(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_TYPES_,,I) ::dlx::rval<type>::t& ) DLX_NOEXCEPT;
#define DLXR_LIB_FN_TYPES_3(r,i,d, type,name,value) ::dlx::arg<type>::t PP_COMMA()
#define DLXR_LIB_FN_TYPES__L3(r,i,d, type,name,value) ::dlx::arg<type>::t
#define DLXR_LIB_FN_TYPES_ARGS_3(r,i,d, type,name,value) ::dlx::arg<type>::t name PP_COMMA()
#define DLXR_LIB_FN_TYPES_ARGS__L3(r,i,d, type,name,value) ::dlx::arg<type>::t name
#define DLXR_LIB_FN_ARGS_3(r,i,d, type,name,value) name PP_COMMA()
#define DLXR_LIB_FN_ARGS__L3(r,i,d, type,name,value) name
#define DLXR_LIB_FN_TYPES_ARGS_VALUES_3(r,i,d, type,name,value) ::dlx::arg<type>::t name value
#define DLXR_LIB_FUNCS_3(r,i,d, type,name,args) DLXI_VOID_SELECT(type,DLXR_LIB_FUNCS_PROC,DLXR_LIB_FUNCS_FUNC)(r,type,name,args)
#define DLXR_LIB_FTABLE_FUNCS_3(r,i,d, type, name, args) PP_COMMA()&dl_funcs<DL_T>::PP_TPL(2,name)
#define DLXR_LIB_FUNCS_PROC(r,type,name,args) static dl_error* DLX_CALL PP_TPL(2,name)\
	(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_TYPES_ARGS_,,IL)) DLX_NOEXCEPT\
{\
	try\
	{\
		DL_T::PP_TPL(1,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_ARGS_,,IL));\
		return 0;\
	}\
	catch (...)\
	{\
		return ::dlx::error<>::create();\
	}\
}
#define DLXR_LIB_FUNCS_FUNC(r,type,name,args) static dl_error* DLX_CALL PP_TPL(2,name)\
	(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_TYPES_ARGS_,,I) ::dlx::rval<type>::t& dl_rval) DLX_NOEXCEPT\
{\
	try\
	{\
		dl_rval = DL_T::PP_TPL(1,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_ARGS_,,IL));\
		return 0;\
	}\
	catch (...)\
	{\
		return ::dlx::error<>::create();\
	}\
}
#define DLXR_LIB_METHODS_3(r,i,d, type,name,args) DLXI_VOID_SELECT(type,DLXR_LIB_METHODS_PROC,DLXR_LIB_METHODS_FUNC)(r,type,name,args,d)
#define DLXR_LIB_METHODS_FUNC(r,type,name,args,cname) type PP_TPL(1,name) (PP_TPLVEC_ENUM_R(r)(r,args,DLXR_LIB_FN_TYPES_ARGS_VALUES_,,I)) const\
	{\
		::dlx::rval<type>::t dl_rval;\
		if (::dlx::error<>::info const* err = dl_ftable->PP_TPL(2,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_ARGS_,,I) dl_rval))\
			::dlx::error<>::throw_ex(err, dl_::PP_CAT(dl_,cname)::dl_globals<>::qname, PP_STR(PP_TPL(1,name)));\
		return dl_rval;\
	}
#define DLXR_LIB_METHODS_PROC(r,type,name,args,cname) void PP_TPL(1,name) (PP_TPLVEC_ENUM_R(r)(r,args,DLXR_LIB_FN_TYPES_ARGS_VALUES_,,I)) const\
	{\
		if (::dlx::error<>::info const* err = dl_ftable->PP_TPL(2,name)(PP_TPLVEC_FOR_R(r)(r,args,DLXR_LIB_FN_ARGS_,,IL)))\
			::dlx::error<>::throw_ex(err, dl_::PP_CAT(dl_,cname)::dl_globals<>::qname, PP_STR(PP_TPL(1,name)));\
	}

/*--- Instruments --------------------------------------------------------------------------------*/

#define DLXI_1_OF_S2(seq2) PP_INVOKE(DLXD_1_OF_S2, seq2 )
#define DLXI_2_OF_S2(seq2) PP_INVOKE(DLXD_2_OF_S2, seq2 )

#define DLXI_VOID_SELECT(type,expr_true,expr_false) DLXD_VC_APPLY( DLXD_VC_SELECT, (PP_CAT(DL_VOID_TYPE_,type),expr_true,expr_false,_) )

#define DLXI_TWO_ARG_MACRO(macro, arg1) macro PP_OB() arg1, DLXD_SECOND_ARG

#define DLXI_NAMESPACE(r, ns_v, ...) \
	PP_VEC_FOR_R(r)(r, ns_v, DLXD_NS_O,,I )\
	__VA_ARGS__\
	PP_VEC_FOR_R(r)(r, ns_v, DLXD_NS_C,,I )

#if defined(_MSC_VER)
  #define DLXI_BUILD_NS(r, ns_s) PP_APPLY_R(r)(PP_SEQ_FOR_R(r)(r,ns_s,DLXD_BUILD_FNAME_O,,I) PP_REPEAT_R(r)(r,PP_SEQ_SIZE(,ns_s),DLXD_BUILD_FNAME_C,) )
#else
  #define DLXI_BUILD_NS(r, ns_s) PP_SEQ_FOR_R(r)(r,ns_s,DLXD_BUILD_FNAME,,I)
#endif

/*--- Details ------------------------------------------------------------------------------------*/

#define DLXD_1_OF_S2(...) __VA_ARGS__ DLXD_1_OF_S2_A
#define DLXD_1_OF_S2_A(...)
#define DLXD_2_OF_S2(...) DLXD_2_OF_S2_A
#define DLXD_2_OF_S2_A(...) __VA_ARGS__

#define DLXD_SECOND_ARG(arg) arg PP_CB()

#define DLXD_NS_O(r,i,d,x) namespace x {
#define DLXD_NS_C(r,i,d,x) }

#if defined(_MSC_VER)
  #define DLXD_BUILD_FNAME_O(r,i,d,x) PP_CAT PP_OB() ::x ,
  #define DLXD_BUILD_FNAME_C(r,i,d) PP_CB()
#else
  #define DLXD_BUILD_FNAME(r,i,d,x) ::x
#endif

#define DLXD_VC_SELECT(_,__,expr,...) expr
#define DLXD_VC_APPLY(macro,args) DLXD_VC_APPLY_A(macro args)
#define DLXD_VC_APPLY_A(x) x

#define DLXD_TYPENAME(r,i,d) typename PP_CAT(T,i)
#define DLXD_TYPE(r,i,d) PP_CAT(T,i)

/*--- #include windows.h  -------------------------------------------------------*/

#ifndef _WINDOWS_
  struct HINSTANCE__;
  typedef HINSTANCE__* HMODULE;
  typedef int (__stdcall* FARPROC)();
  typedef long LONG;
  
  #if defined(_MSC_VER)
    #pragma comment(lib, "kernel32.lib")
    #define DLX_DLL_IMPORT __declspec(dllimport)
  #else //defined(__GNUC__) || defined(__CODEGEARC__)
    #define DLX_DLL_IMPORT
  #endif
  
  extern"C" DLX_DLL_IMPORT HMODULE  __stdcall LoadLibraryA(char const*);
  extern"C" DLX_DLL_IMPORT FARPROC  __stdcall GetProcAddress(HMODULE, char const*);
  extern"C" DLX_DLL_IMPORT int      __stdcall FreeLibrary(HMODULE);

  #if defined(_MSC_VER)
	extern"C" LONG __cdecl _InterlockedIncrement(LONG volatile*);
	extern"C" LONG __cdecl _InterlockedDecrement(LONG volatile*);
  #else
    extern"C" LONG __stdcall InterlockedIncrement(LONG volatile*);
    extern"C" LONG __stdcall InterlockedDecrement(LONG volatile*);
  #endif
#endif

#if defined(_MSC_VER)
  #define DLX_ATOMIC_INCREMENT(x)  _InterlockedIncrement(&x)
  #define DLX_ATOMIC_DECREMENT(x)  _InterlockedDecrement(&x)
#else
  #define DLX_ATOMIC_INCREMENT(x)  InterlockedIncrement(&x)
  #define DLX_ATOMIC_DECREMENT(x)  InterlockedDecrement(&x)
#endif

/*--- Classes -------------------------------------------------------------------*/

#define DLX_NOEXCEPT // noexcept
#define DLX_DELETE // = delete

#define DLX_CALL __stdcall
#define DLX_NONCOPYABLE(class_name)\
	private:\
		class_name(class_name const&) DLX_DELETE;\
		class_name& operator=(class_name const&) DLX_DELETE;

#if defined(_MSC_VER)
  #define DLX_EXC_THROW0()
  #define DLX_NOINLINE __declspec(noinline)
  #define DLX_PACKED(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop));
  #define DLX_DLL_EXPORT __declspec(dllexport)
#else //defined(__GNUC__) || defined(__CODEGEARC__)
  #define DLX_EXC_THROW0() throw()
  #define DLX_NOINLINE
  #if defined(__GNUC__)
    #define DLX_PACKED(...) __VA_ARGS__   __attribute__((packed));
	#define DLX_DLL_EXPORT __declspec(dllexport)
  #else//defined(__CODEGEARC__) 
    #define DLX_PACKED(...) __VA_ARGS__ // ???
	#define DLX_DLL_EXPORT              // ???
  #endif
#endif

#if DL_DEBUG_LEVEL < 1
  #define DLX_ASSERT_1(cond)
  #define DLX_ON_DEBUG_1(...)
  #define DLX_ON_NODBG_1(...) __VA_ARGS__
#else
  #define DLX_ASSERT_1(cond) assert( cond );
  #define DLX_ON_DEBUG_1(...) __VA_ARGS__
  #define DLX_ON_NODBG_1(...)
#endif

#if DL_DEBUG_LEVEL < 2
  #define DLX_ASSERT_2(cond)
#else
  #define DLX_ASSERT_2(cond) DLX_ASSERT_1(cond)
#endif


#if defined(_MSC_VER) && (1600 <= _MSC_VER)
  #define DLX_STATIC_ASSERT(cond, str)         static_assert(cond, str);
  #define DLX_STATIC_CHECK_SIZE(element, size) static_assert( sizeof(element) == (size), "Unsuitable compiler settings. The size of '" PP_STR(element) "' is invalid." );
  #define DLXD_FWD_TYPE_ARG(r,i,d) PP_CAT(T,i)&& PP_CAT(t,i)
  #define DLXD_FWD_ARG(r,i,d) std::forward<PP_CAT(T,i)>(PP_CAT(t,i))
#else
  namespace dlx
  {
	  template<bool _> struct static_assertion;
	  template<> struct static_assertion<true> {};
	  template<typename T, bool _> struct Unsuitable_compiler_settings_Invalid_size_of;
	  template<typename T> struct Unsuitable_compiler_settings_Invalid_size_of<T, true> {};
	  template<std::size_t N> struct inspector;
  }
  #define DLX_STATIC_ASSERT(cond, str)         typedef ::dlx::inspector<sizeof(::dlx::static_assertion<(cond)>)> static_assertion;
  #define DLX_STATIC_CHECK_SIZE(element, size) typedef ::dlx::inspector<sizeof(::dlx::Unsuitable_compiler_settings_Invalid_size_of<element, sizeof(element) == (size)>)> static_assertion;
  #define DLXD_FWD_TYPE_ARG(r,i,d) PP_CAT(T,i) const& PP_CAT(t,i)
  #define DLXD_FWD_ARG(r,i,d) PP_CAT(t,i)
#endif

/*--- Classes -------------------------------------------------------------------*/

namespace dlx
{
	class runtime_error : 
		public std::runtime_error
	{
	public:
		runtime_error(std::string const& str)
			: std::runtime_error( str )
		{}
	};


	class method_error : 
		public std::runtime_error
	{
	public:
		method_error(char const* exception, char const* class_name,	char const* method, char const* message)
			: std::runtime_error( format(exception, class_name, method, message) )
		{}

	private:
		static std::string format(char const* exception, char const* class_name, char const* method, char const* message)
		{
			DLX_ASSERT_1(exception != 0);
			DLX_ASSERT_1(class_name != 0);
			DLX_ASSERT_1(method != 0);
			DLX_ASSERT_1(message != 0);

			std::string result("exception '");
			result += exception;
			result += "' in method '";
			result += method;
			result += "' of class '";
			result += class_name;
			result += "' with message '";
			result += message;
			result += "'";

			return result;
		}
	};


	template<typename Exception> DLX_NOINLINE void throw_ex(std::string const& str)
	{
		throw Exception(str);
	}

	/*----------------------------------------------------------------------*/

	template<bool cond, typename T1, typename T2> struct select
	{
		typedef T1 result;
	};
	template<typename T1, typename T2> struct select<false, T1, T2>
	{
		typedef T2 result;
	};


	struct false_
	{
		static const bool value = true;
	};

	struct true_
	{
		static const bool value = true;
	};

	template<typename T> struct is_interface : false_ {};
	template<typename T> struct is_fwd_interface : false_ {};

	template<bool, typename T> struct arg_type_sel
	{
		static const bool result = 4 < sizeof(T) || is_interface<T>::value;
	};
	template<typename T> struct arg_type_sel<true, T>
	{
		static const bool result = true;
	};
	template<typename T> struct arg
	{
		typedef typename select< arg_type_sel<is_fwd_interface<T>::value,T>::result, T const&, T >::result t;
	};
	template<typename T> struct arg<T&>
	{
		typedef T& t;
	};
	template<typename T> struct arg<T const&>
	{
		typedef T const& t;
	};

	template<typename T> struct rval
	{
		typedef T t;
	};
	template<typename T> struct rval<T&>
	{
		DLX_NONCOPYABLE(rval)
	public:
		typedef rval<T&> t;

		operator T&() 
		{
			return *val_;
		}

		void operator=(T& val)
		{
			val_ = &val;
		}
				
	private:
		T* val_;
	};

	/*----------------------------------------------------------------------*/

	class dynamic_lib
	{
		DLX_NONCOPYABLE(dynamic_lib)
	public:
		explicit dynamic_lib(std::string const& path)
		{
			handle_ = ::LoadLibraryA(path.c_str());
			if (handle_ == 0)
				throw_ex< ::dlx::runtime_error >( "can't load library '" + path + "'" );
		}

		~dynamic_lib()
		{
			::FreeLibrary(handle_);
		}

		class functions
		{
			DLX_NONCOPYABLE(functions)
		public:
			functions(dynamic_lib& lib, std::string const& path)
				: lib_(lib)
				, path_(path)
			{}

			template<typename Fn> void operator()(char const* name, Fn& f)
			{
				f = reinterpret_cast<Fn>(get_addr(name));
			}

			template<typename Fn> void operator()(char const* name, Fn& f, std::nothrow_t) DLX_NOEXCEPT
			{
				f = reinterpret_cast<Fn>(get_addr(name, std::nothrow));
			}

		private:
			FARPROC get_addr(char const* name) const
			{
				FARPROC fn_addr = ::GetProcAddress(lib_.handle_, name);
				if (fn_addr == 0)
					throw_ex< ::dlx::runtime_error >( std::string("function '") + name + "' not found in library '" + path_ + "'");
				return fn_addr;
			}

			FARPROC get_addr(char const* name, std::nothrow_t) const DLX_NOEXCEPT
			{
				return ::GetProcAddress(lib_.handle_, name);
			}

			dynamic_lib&       lib_;
			std::string const& path_;
		};

	private:
		HMODULE handle_;
	};

	template<typename Lib> struct library;

	/*----------------------------------------------------------------------*/

	struct no_init_t {} const no_init  = no_init_t();
	struct nullptr_t {} const null_ptr = nullptr_t();

	const std::size_t pointer_size = sizeof(void *);

	typedef long             refs_count;
	
	template<typename ClassName> struct safe_bool
	{
		typedef void (safe_bool::*_)();
		void true_() {}
	};


	/*----------------------------------------------------------------------*/

	namespace hash
	{
		typedef unsigned __int64 value;
		template<typename T> value hash_of();
		template<typename T> value hash_of(T const&);

		template<typename H, value X> struct hash
		{
			static const value result = ((H::result & 0x000FFFFFFFFFFFFFull) << 11) + (H::result & 0x7FFFFFFFFFFFFFFFull) + X;
		};

		template <bool, typename T> struct type_hash
		{
			static const value result = is_interface<T>::value ? type_hash<true, T>::result : sizeof(T);
		};
		template <typename T> struct type_hash<true, T>
		{
			static const value result = 0x1ABCDEF;
		};

		template<typename T> struct type
		{
			static const value result = type_hash<is_fwd_interface<T>::value, T>::result;
		};
		template<> struct type<void>
		{
			static const value result = 0x87654321Aull;
		};
		template<typename T> struct type<T*>
		{
			static const value result = type<T>::result + 0xCDE12345ull;
		};
		template<typename T> struct type<T&>
		{
			static const value result = type<T>::result + 0xABC12345ull;
		};
		template<typename T> struct type<T const&>
		{
			static const value result = type<T>::result + 0xFCD12345ull;
		};

		struct initial
		{
			static const value result = 0x1234567890ABCDEFull;
		};

		template<typename H, typename T> struct arg
		{
			static const value result = hash<H, type<T>::result>::result;
		};

		template<typename H, typename R, unsigned C> struct fn
		{
			static const value result = hash<H, type<R>::result + (C + 1) * 0xA98765ull>::result;
		};
	}

	/*----------------------------------------------------------------------*/

	template<typename T = void> struct error
	{
	public:
		DLX_PACKED(	struct info
		{
			char const* data;
			void (DLX_CALL*free)(info const*) DLX_NOEXCEPT;
		})
		DLX_STATIC_CHECK_SIZE( info, 2 * pointer_size )

		// !!! функция должна вызываться внутри блока catch
		static DLX_NOINLINE info const* create() DLX_NOEXCEPT
		{
			try
			{
				throw;
			}
			catch (std::exception const& e)
			{
				return create_info(typeid(e).name(), e.what());
			}
			catch (...)
			{
				return create_info("unknown", "not available");
			}
		}

		static DLX_NOINLINE void throw_ex(info const* e, char const* class_name, char const* method)
		{
			ptr err(e);

			DLX_ASSERT_2(class_name != 0)
			DLX_ASSERT_2(method != 0)

			char const* exception  = err.data();
			char const* message    = exception + std::char_traits<char>::length(exception)  + 1;
			throw ::dlx::method_error( exception, class_name, method, message );
		}

		static void DLX_CALL do_nothing(info const*) DLX_NOEXCEPT
		{}

	private:
		static info const bad_alloc;

		static void DLX_CALL free(info const* inf) DLX_NOEXCEPT
		{
			delete[] inf->data;
			delete   inf;
		}

		class ptr
		{
			DLX_NONCOPYABLE( ptr )
		public:
			ptr(info const* inf) DLX_NOEXCEPT
				: info_(inf)
			{
				DLX_ASSERT_2(inf != 0);
			}

			char const* data() const DLX_NOEXCEPT
			{
				return info_->data;
			}

			~ptr() DLX_NOEXCEPT
			{
				info_->free(info_);
			}

		private:
			info const* info_;
		};

		static info const* create_info(char const* exception, char const* msg) DLX_NOEXCEPT
		{
			DLX_ASSERT_2(exception != 0)
			DLX_ASSERT_2(msg != 0)

			info* inf = 0;
			try
			{
				inf = new info;
				inf->free = &error<>::free;

				std::string str(exception);
				str.push_back(0);
				str += msg;
				str.push_back(0);

				char* data = new char[str.size()];

				// noexcept
				{ 
					std::copy(str.begin(), str.end(), data);

					inf->data = data;

					return inf;
				}
			}
			catch (...)
			{
				delete inf;
				return &bad_alloc;
			}
		}
	};
	
	template<typename T> typename error<T>::info const error<T>::bad_alloc = 
	{
		"std::bad_alloc\0not enough a memory",
		&error<>::do_nothing
	};
	
	/*----------------------------------------------------------------------*/

	template<typename T> class reference
	{
		// default copyable - корректное поведение в контексте диагностики неправильного
		//                    использования интерфейсов, указывающих на reference<T> !!!
	public:
		reference(T& obj) DLX_NOEXCEPT
			: obj_(&obj)
			DLX_ON_DEBUG_1( , refs_(1) )
		{}

		T* get() const DLX_NOEXCEPT
		{
			return obj_;
		}

		DLX_ON_DEBUG_1
		(
			~reference() DLX_NOEXCEPT
			{
				DLX_ASSERT_1( refs_ == 1 );
			}

			void DLX_CALL dl_add_ref() const DLX_NOEXCEPT
			{
				DLX_ATOMIC_INCREMENT( refs_ );
			}

			int DLX_CALL dl_release() const DLX_NOEXCEPT
			{
				DLX_ATOMIC_DECREMENT( refs_ );
				return static_cast<int>( refs_ );
			}
		)

	private:
		T* obj_;
		DLX_ON_DEBUG_1( mutable refs_count refs_; )
	};

	template<typename T> reference<T> ref(T& obj)
	{
		return obj;
	}

	/*----------------------------------------------------------------------*/

	template<typename T> class shared;


	template<typename T> class shared_class
		: public T
	{
		DLX_NONCOPYABLE( shared_class )
	public:
		shared_class()
			: T()
			, refs_(1)
		{}

#define DLXD_SH_CLASS_CTR(r,i,d)\
		template< PP_REPEAT_ENUM_R(r)(r, i, DLXD_TYPENAME,) >\
		shared_class( PP_REPEAT_ENUM_R(r)(r, i, DLXD_FWD_TYPE_ARG,) )\
			: T( PP_REPEAT_ENUM_R(r)(r, i, DLXD_FWD_ARG,) )\
			, refs_(1)\
		{}
PP_REPEAT(DL_MAX_ARGS_COUNT,DLXD_SH_CLASS_CTR,)

		void dl_add_ref() DLX_NOEXCEPT
		{
			DLX_ATOMIC_INCREMENT( refs_ );
		}

		int dl_release() DLX_NOEXCEPT
		{
			refs_count refs = DLX_ATOMIC_DECREMENT( refs_ );
			if (0 == refs)
				delete this;

			return static_cast<int>(refs);
		}

		static shared_class<T>* get(shared<T> const&) DLX_NOEXCEPT;

	private:
		refs_count refs_;
	};


	template<typename T> class shared
	{
	public:
		shared()
			: obj_( new shared_class<T>() )
		{}

#define DLXD_SH_CTR(r,i,d)\
		template< PP_REPEAT_ENUM_R(r)(r, i, DLXD_TYPENAME,) >\
		shared( PP_REPEAT_ENUM_R(r)(r, i, DLXD_FWD_TYPE_ARG,) )\
			: obj_( new shared_class<T>( PP_REPEAT_ENUM_R(r)(r, i, DLXD_FWD_ARG,) ) )\
		{}
PP_REPEAT(DL_MAX_ARGS_COUNT,DLXD_SH_CTR,)

		shared(shared const& other) DLX_NOEXCEPT
			: obj_( other.obj_ )
		{
			obj_->dl_add_ref();
		}

		shared& operator=(shared const& other) DLX_NOEXCEPT
		{
			other.obj_->dl_add_ref();

			obj_->dl_release();

			obj_ = other.obj_;

			return *this;
		}

		~shared() DLX_NOEXCEPT
		{
			obj_->dl_release();
		}

		T* operator->() DLX_NOEXCEPT
		{
			return obj_;
		}

	private:
		friend class shared_class<T>;
		shared_class<T>* obj_;
	};


	template<typename T> shared_class<T>* shared_class<T>::get(shared<T> const& s) DLX_NOEXCEPT
	{
		return s.obj_;
	}

	/*----------------------------------------------------------------------*/	

	namespace ftable
	{
		template<typename T = void> struct null
		{
			typedef void object;

			static error<>::info const invalid_call;

			static void DLX_CALL add_ref(void*) DLX_NOEXCEPT {}
			static int  DLX_CALL release(void*) DLX_NOEXCEPT { return 0; }

#define DLXD_NULL_FUNC(r,i,d)\
			template< PP_REPEAT_ENUM_R(r)(r, i, DLXD_TYPENAME,) >\
			static error<>::info const* DLX_CALL func( PP_REPEAT_ENUM_R(r)(r, i, DLXD_TYPE,) ) DLX_NOEXCEPT\
			{\
				return &invalid_call;\
			}
PP_REPEAT(DL_MAX_ARGS_COUNT, DLXD_NULL_FUNC, )
		};

		template<typename T> typename error<>::info const null<T>::invalid_call = 
		{
			"dl::runtime_error\0method call for null interface",
			&error<>::do_nothing
		};
	}

	/*----------------------------------------------------------------------*/	

	// !!! использование только для отладки
	template <typename Interface> int references_of( Interface const& i ) DLX_NOEXCEPT
	{
		DLX_ASSERT_2( i.dl_ftable != 0 );

		i.dl_ftable->dl_add_ref(i.dl_object);
		return i.dl_ftable->dl_release(i.dl_object);
	}
}

namespace dl
{
	using ::dlx::runtime_error;
	using ::dlx::method_error;

	using ::dlx::no_init;
	using ::dlx::ref;

	using ::dlx::shared;
	using ::dlx::null_ptr;

	using ::dlx::hash::hash_of;
	using ::dlx::references_of;
}
