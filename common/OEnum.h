#ifndef _OLDPART_OENUM_H
#define _OLDPART_OENUM_H
#include <mutex>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#define OENUM  _EXPAND(z,d,i,e) BOOST_PP_COMMA_IF(i) BOOST_PP_TUPLE_ELEM(2,0,e) = BOOST_PP_TUPLE_ELEM(2,1,e)
#define OENUM  _VALUE(seq) BOOST_PP_SEQ_FOR_EACH_I(OENUM  _EXPAND,-,seq)
#define OENUM  _PARSE(z,d,i,e) gString2Enum->insert(std::make_pair(BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2,0,e)),BOOST_PP_TUPLE_ELEM(2,0,e)));
#define OENUM  _TOSTR(z,d,i,e) gEnum2String->insert(std::make_pair(BOOST_PP_TUPLE_ELEM(2,0,e),BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2,0,e))));

#define OENUM  _IMPL_TO_STRING(EnumName,seq) \
    const std::string& EnumName::ToString(EnumName::type v)\
    {\
        static std::unordered_map<EnumName::type,std::string>* gEnum2String = nullptr;\
        static std::once_flag initMap;\
        static const std::string UNKNOWN_VALUE("Unknow value for "#EnumName);\
        std::call_once(initMap\
        ,[]()\
        {\
            gEnum2String = new std::unordered_map<EnumName::type,std::string>();\
            BOOST_PP_SEQ_FOR_EACH_I(OENUM  _TOSTR,~,seq)\
        });\
        auto itResult = gEnum2String->find(v);\
        if(itResult != gEnum2String->end())\
        {\
            return itResult->second;\
        }\
        return UNKNOWN_VALUE;\
    }

#define OENUM  _IMPL_PARSE(EnumName,seq)\
    bool EnumName::Parse(const std::string& value,EnumName::type& v)\
    {\
        static std::unordered_map<std::string,EnumName::type>* gString2Enum = nullptr;\
        static std::once_flag initMap;\
        std::call_once(initMap\
        ,[]()\
		{\
            gString2Enum = new std::unordered_map<std::string,EnumName::type>();\
            BOOST_PP_SEQ_FOR_EACH_I(OENUM  _PARSE,~,seq)\
		});\
        auto itResult = gString2Enum->find(value);\
        if(gString2Enum->end() != itResult)\
        {\
            v = itResult->second;\
            return true;\
        }\
        return false;\
   }

#define OENUM  _IMPL_PARSE_EX(EnumName, seq)\
    bool EnumName::ParseEx(const std::string& value,int32_t& v)\
    {\
        std::std::vector<std::string> actions;\
        boost::algorithm::split(actions,value,boost::is_any_of("|"),boost::algorithm::token_compress_on);\
\
        v = 0;\
        for(auto& action : actions)\
        {\
            EnumName::type tmpAction;\
            if(Parse(action,tmpAction))\
            {\
                v |= tmpAction;\
            }\
            else\
            {\
                return false;\
            }\
        }\
\
        return true;\
    }


#endif
