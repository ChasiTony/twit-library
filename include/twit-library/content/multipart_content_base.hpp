//
// multipart_content_base.hpp
// ~~~~~~~~~~
//
// Content-Type���X�����g���O(multipart/form-data�n)
//

#ifndef TWIT_LIB_CONTENT_MULTI_BASE
#define TWIT_LIB_CONTENT_MULTI_BASE

#include <map>
#include "content_base.hpp"

namespace oauth{
namespace content{

class multipart_content_base : public content_base{
public:
    multipart_content_base(){}
    virtual ~multipart_content_base(){}

    const std::map<std::string,std::string> get_addition_param() const { return std::map<std::string,std::string>(); }
    virtual const std::string get_part_body() const { return ""; }
};

} // namespace content
} // namespace oauth

#endif
