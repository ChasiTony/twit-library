//
// base64.hpp
// ~~~~~~~~~~
//
// Base64�F��
//

#ifndef TWIT_LIB_UTILITY_BASE64_HPP
#define TWIT_LIB_UTILITY_BASE64_HPP

#include <string>

namespace oauth{
namespace utility{
    
class base64
{
public:
    base64();
    virtual ~base64(); // = default;

    template<typename InputIterator, typename OutputIterator>
    OutputIterator encode(InputIterator first, InputIterator last, OutputIterator out) const;
    const std::string encode(const std::string& data) const;
    
    const std::string decode(const std::string& data) const;

private:
    //������table��static const std::string�Ŕz�u���ď�����������(C++11)
};

inline const std::string base64_encode(const std::string& data);

} // namespace utility
} // namespace oauth

#ifdef TWIT_LIB_BUILD
#include "impl/base64.ipp"
#endif

#endif
