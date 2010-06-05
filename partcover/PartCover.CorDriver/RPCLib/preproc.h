//	preproc.h: Preprocessor helper library
//	Developer:	Andrew Solodovnikov
//	E-mail:		none
//	Date:		20.05.2007
//  Version     1.0.0

#ifndef	__PREPROC_H__
#define __PREPROC_H__


#ifndef PPC_USE_BOOST_PP

#define PPC_CAT_(x,y) x##y
#define PPC_CAT(x,y) PPC_CAT_(x,y)

#define PPC_STRINGIZE_(x) #x
#define PPC_STRINGIZE(x) PPC_STRINGIZE_(x)


#define PPC_MAX_REPEAT 16
#define PPC_COMMA() ,
#define PPC_EMPTY() 
#else


#include <boost/preprocessor/limits.hpp> 
#include <boost/preprocessor/cat.hpp> 
#include <boost/preprocessor/expand.hpp> 
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>

#define PPC_MAX_REPEAT BOOST_PP_LIMIT_SEQ


#define PPC_COMMA BOOST_PP_COMMA
#define PPC_EMPTY BOOST_PP_EMPTY

#define PPC_CAT BOOST_PP_CAT
#define PPC_STRINGIZE BOOST_PP_STRINGIZE

#define PPC_EVAL(x)  x

#define PPC_BOOL     BOOST_PP_BOOL
#define PPC_IF       BOOST_PP_IF
#define PPC_COMMA_IF BOOST_PP_COMMA_IF

#define PPC_DEC       BOOST_PP_DEC

#define PPC_SEQ_ENUM BOOST_PP_SEQ_ENUM
#define PPC_SEQ_SIZE BOOST_PP_SEQ_SIZE
#define PPC_SEQ_POP_FRONT BOOST_PP_SEQ_POP_FRONT

#define PPC_SEQ_FOR_EACH_I  BOOST_PP_SEQ_FOR_EACH_I

#define PPC_REPEAT_PARAMS BOOST_PP_ENUM_PARAMS
#define PPC_REPEAT_TRAILING_PARAMS BOOST_PP_ENUM_TRAILING_PARAMS


#endif

#endif
