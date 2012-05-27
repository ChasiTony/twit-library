//
// authentication.hpp
// ~~~~~~~~~~
//
// �F�،n�@�\��utility���W
//

//
// ��H�S��inline�t���ĂĂ����������āH����Ȃ̋C�ɂ����畉������I
//

#ifndef TWIT_LIB_UTILITY_AUTHENTICATION
#define TWIT_LIB_UTILITY_AUTHENTICATION

#include <map>
#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/random.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/bind.hpp>

namespace oauth{
namespace utility{


inline const int hex_to_dec(const std::string& hex)
{
  int dec = 0;
  for(std::string::const_iterator it = hex.cbegin(); it != hex.cend(); ++it)
  {
    dec *= 16;
    if(*it >= '0' && *it <= '9')
    {
      dec += ((*it)-'0');
    }
    else if(*it >= 'A' && *it <= 'F')
    {
      dec += ((*it)-'A'+10);
    }
    else if(*it >= 'a' && *it <= 'f')
    {
      dec += ((*it)-'a'+10);
    }
  }
  return dec;
}

inline const std::string url_encode(const std::string& base_string)
{
  std::string encoded="";
  for(std::string::const_iterator it = base_string.cbegin();it!=base_string.cend();++it)
  {
    if((0x30<=*it && *it<=0x39) || (0x41<=*it && *it<=0x5A) || (0x61<=*it && *it<= 0x7A) || *it==0x2D || *it==0x2E || *it==0x5F || *it==0x7E)
      encoded.append(it,it+1);
    else
      encoded.append((boost::format("%%%02X") % (*it & 0xff)).str());
  }
  return encoded;
}

// +��' '�ɂ͂��Ȃ�
inline const std::string url_decode(const std::string& base_string)
{
  std::string decoded="";
  for(std::string::const_iterator it = base_string.begin();it!=base_string.end();++it)
  {
    if(*it == '%') //%xx
    {
      decoded += (char)hex_to_dec(std::string(it+1,it+3));
      it+=2;
    }
    else decoded += *it;
  }
  return decoded;
}


// std::string::push_back �� append ���ɓ��ꂵ�����Ȃ�
inline const std::string base64_encode(const std::string& data)
{
  static const char table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string result;
  for(std::string::size_type i = 0; i<data.size(); ++i)
  {
    switch(i%3)
    {
    case 0:
      result.push_back(table[(data[i] >> 2) & 0x3F/*0011 1111*/]);
      if(i+1>=data.size())
      {
        result.push_back(table[(data[i] << 4) & 0x30/*0011 0000*/]);
        result.append("==");
      }
      break;
    case 1:
      result.push_back(table[((data[i-1] << 4) & 0x30/*0011 0000*/) | ((data[i] >> 4) & 0x0F/*0000 1111*/)]);
      if(i+1>=data.size())
      {
        result.push_back(table[(data[i] << 2) & 0x3C/*0011 1100*/]);
        result.append("=");
      }
      break;
    case 2:
      result.push_back(table[((data[i-1] << 2) & 0x3C/*0011 1100*/) | ((data[i] >> 6) & 0x03/*0000 0011*/)]);
      result.push_back(table[data[i] & 0x3F/*0011 1111*/]);
      break;
    }
  }
  return result;
}

namespace{
  inline unsigned int left_shift(int bits, unsigned int word)
  {
    return (((word) << (bits)) | ((word) >> (32-(bits))));
  }
}

inline std::string sha1_hash(std::string data,unsigned long long length)
{
  // 0x80���l�߂�
  data.push_back((char)0x80);

  // 64*n + (64-8)bit�ɒB����悤��0x00���l�߂�
  const int add_length = 56 - data.length() % 64;
  data.append((add_length < 0) ? add_length + 64 : add_length, (char)0x00);
  
  //������8�o�C�g�ŋl�߂�
  length *= 8;
  for(int i=56; i>=0; i-=8) data.push_back((char)(length >> i) & 0xFF);
  
  unsigned int hash[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
  unsigned int w[80];
  const unsigned int num_block = data.length() / 64;
  for(unsigned int i = 0; i < num_block; ++i)
  {
    for(int j = 0; j < 16; ++j)
    {
      w[j]  = ((unsigned char)data[i*64+j*4  ] << 24);
      w[j] |= ((unsigned char)data[i*64+j*4+1] << 16);
      w[j] |= ((unsigned char)data[i*64+j*4+2] <<  8);
      w[j] |= ((unsigned char)data[i*64+j*4+3]      );
    }
    for(int j = 16; j < 80; ++j) w[j] = left_shift(1, w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16]);
    
    unsigned int a = hash[0];
    unsigned int b = hash[1];
    unsigned int c = hash[2];
    unsigned int d = hash[3];
    unsigned int e = hash[4];

    for(int i = 0; i < 80; ++i)
    {
      unsigned int f, k;
      if(i<20)
      {
        f = (b & c) | (~b & d);
        k = 0x5A827999;
      }
      else if(i<40)
      {
        f = b ^ c ^ d;
        k = 0x6ED9EBA1;
      }
      else if(i<60)
      {
        f = (b & c) | (b & d) | (c & d);
        k = 0x8F1BBCDC;
      }
      else
      {
        f = b ^ c ^ d;
        k = 0xCA62C1D6;
      }

      unsigned int temp = left_shift(5,a) + f + e + k + w[i];
      e = d;
      d = c;
      c = left_shift(30,b);
      b = a;
      a = temp;
    }

    hash[0] += a;
    hash[1] += b;
    hash[2] += c;
    hash[3] += d;
    hash[4] += e;
  }
  
  std::string result;
  for(int i = 0; i < 5; ++i) for(int shift = 24; shift >= 0; shift -= 8) result.push_back((char)(hash[i] >> shift) & 0xFF);
  return result;
}

inline std::string hmac_sha1(const std::string& key,const std::string& data)
{
  std::string k_ipad,k_opad;
  k_ipad = k_opad = (key.length() > 64) ? sha1_hash(key,key.length()) : key;
  
  while(k_ipad.length()!=64)
  {
    k_ipad.push_back((char)0x00);
    k_opad.push_back((char)0x00);
  }

  for(int i=0;i<64;++i)
  {
    k_ipad[i] ^= 0x36;
    k_opad[i] ^= 0x5c;
  }

  return sha1_hash(k_opad + sha1_hash(k_ipad + data, 64+data.length()), 64 + 20);
}


inline const std::string get_timestamp()
{
  return (boost::format("%1%") % static_cast<unsigned long>(std::time(0))).str();
}

inline const std::string get_nonce()
{
  // initlize
  static const boost::uniform_int<unsigned long> range(0,62-1);
  static boost::mt19937 gen(static_cast<unsigned long>(std::time(0)));
  static boost::variate_generator<
    boost::mt19937&,boost::uniform_int<unsigned long>
  > rand(gen,range);  
  static const unsigned char nonce_base[] = 
  {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '1','2','3','4','5','6','7','8','9','0'
  };
  
  //Calc
  unsigned int nonce_length = 0;
  while(nonce_length<12) nonce_length = rand();

  std::string nonce="";
  for(unsigned int i=0;i<nonce_length;++i)  nonce += nonce_base[rand()];
  
  return nonce;
}

//Karma�g����H�̂���
inline const std::string get_signature(const std::string& method,const std::string& uri,const std::string& key,const std::map<std::string,std::string>& values)
{
  typedef std::pair<const std::string,std::string> Value_Pair;

  std::string base_string;
  BOOST_FOREACH(const Value_Pair& p,values)
    base_string += url_encode(p.first) + "=" + url_encode(p.second) + "&";

  base_string.erase(base_string.size()-1);
  base_string = method + "&" + url_encode(uri) + "&" + url_encode(base_string);

  return base64_encode(hmac_sha1(key,base_string));
}

//get_authorization_field��get_urlencoded���܂Ƃ߂�ꂻ���D
inline const std::string get_authorization_field(const std::map<std::string,std::string>& values)
{
  typedef std::pair<const std::string,std::string> Value_Pair;

  std::string field_string = "";
  BOOST_FOREACH(const Value_Pair& p,values)
    field_string += (url_encode(p.first) + "=\"" + url_encode(p.second) + "\",");

  if(!field_string.empty()) field_string.erase(field_string.size()-1);
  return field_string;
}

inline const std::string get_urlencoded(const std::map<std::string,std::string>& values)
{
  typedef std::pair<const std::string,std::string> Value_Pair;
  
  std::string field_string = "";
  BOOST_FOREACH(const Value_Pair& p,values)
    field_string += (url_encode(p.first) + "=" + url_encode(p.second) + "&");

  if(!field_string.empty()) field_string.erase(field_string.size()-1);
  return field_string;
}

inline const std::map<std::string,std::string> parse_urlencoded(const std::string& encoded)
{
  namespace qi = boost::spirit::qi;

  const auto value_rule = ((+(qi::char_ - "&") >> -qi::lit("&")));
  const auto pair_rule =
    +(qi::char_ - "=") >> "=" >> value_rule;
  const auto rule = 
    *pair_rule;
  
  auto it = encoded.cbegin();
  std::map<std::string,std::string> parsed;
  qi::parse(it,encoded.cend(),rule,parsed);

  return parsed;
}

inline const boost::tuple<std::string,std::string,std::string> get_scheme_host_path(const std::string& uri)
{
  namespace qi = boost::spirit::qi;
  
  const auto rule = 
    +(qi::char_ - ":") >> qi::lit(":") >> *qi::lit("/") >> +(qi::char_ - "/") >> +(qi::char_ - "?");

  std::string::const_iterator it = uri.cbegin();
  std::string scheme="",host="",path="";
  qi::parse(it,uri.cend(),rule,scheme,host,path);

  return boost::tuple<const std::string,const std::string,const std::string>(scheme,host,path);
}

inline const std::string get_scheme(const std::string& uri)
{
  return get_scheme_host_path(uri).get<0>();
}
inline const std::string get_host(const std::string& uri)
{
  return get_scheme_host_path(uri).get<1>();
}
inline const std::string get_path(const std::string& uri)
{
  return get_scheme_host_path(uri).get<2>();
}

} // namespace utility
} // namespace oauth

#endif
