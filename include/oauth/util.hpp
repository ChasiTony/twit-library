///////////////////////////////////////////////////////////////////////////////
// util.hpp header file
// twit-library : http://code.google.com/p/twit-library/
//
// OAuthを組み立てるときに使う、便利な関数達。
// 実を言うとライブラリを使いやすくしただけ。

#ifndef UTIL_HPP
#define UTIL_HPP

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include <string>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>

#pragma warning(disable: 4101)
#pragma warning(disable: 4309)
#pragma warning(disable: 4996)
#define __BBL_DISABLE_BINARY__
#define __BBL_DISABLE_SELECTORS__
#include <babel.cpp>
#pragma warning(default: 4101)
#pragma warning(default: 4309)
#pragma warning(default: 4996)

namespace util{
	namespace convert{
		inline std::string sjis_utf8(const std::string& base)
		{
			babel::init_babel();
			return babel::sjis_to_utf8(base);
		}
		inline std::string utf8_sjis(const std::string& base)
		{
			babel::init_babel();
			return babel::utf8_to_sjis(base);
		}
		inline unsigned long binary_to_dec(const std::string binary)
		{
			unsigned long result=0;
			BOOST_FOREACH(char ch,binary)
			{
				result *= 2;
				if(ch == '1') //正しければ0を返すのでelseに移るだろう
					result += 1; //ここに来るのは1が収納されてたという事。
				else{} //特に何もしない
			}
			return result;
		}
		inline std::string dec_to_binary(unsigned long dec)
		{
			std::string result;
			const char ch[] = "01";
			do{
				int i = dec%2;
				result=ch[i]+result;
			}
			while((dec/=2)!=0);

			return result;
		}
		inline unsigned long hex_to_dec(std::string hex)
		{
			std::string::size_type i = hex.find(" ");
			const std::string ss = "0123456789ABCDEF";
			if(i!=std::string::npos)
				hex.erase(hex.find(" "));
			boost::algorithm::to_upper(hex);

			unsigned long dec=0;
			BOOST_FOREACH(char ch,hex)
			{
				dec*=16;
				dec += (int)ss.find(ch);
			}
			return dec;
		}
		inline std::string dec_to_hex(unsigned long dec)
		{
			std::string hex;
			const char ch[] = "0123456789ABCDEF";
			do{
				int i = dec%16;
				hex=ch[i]+hex;
			}
			while((dec/=16)!=0);

			return hex;
		}
	}//End convert namespace

	namespace image{
		inline std::string chk_type(std::string image_name)
		{
			image_name.erase(0,image_name.find_last_of(".")+1);
			boost::to_lower(image_name);

			if((image_name == "jpeg")||(image_name == "jpg"))
			{
				return "image/jpeg";
			}
			else if(image_name == "png")
			{
				return "image/png";
			}
			else if(image_name == "gif")
			{
				return "image/gif";
			}
			throw std::exception("Not Support Format");
		}
		inline std::string loader(const std::string& image_location){
			std::ifstream fin(image_location,std::ios_base::binary);
			if(!fin) throw std::exception("Not Found Image");
			
			std::string image = "";
			while(!fin.eof()){
				unsigned char c;
				fin.read((char*)&c,sizeof(unsigned char));
				image += c;
			}
			fin.close();

			return image;
		}
	}//End image namespace
	
	namespace map{
		//serializeを行う
		//std::stringの処理しかできない(作者の技術力のなさで)
		std::string serialization(std::map<std::string,std::string> data,const std::string tie="=",const std::string separate="&",const std::string enclose=""){
			std::string ret;
			std::string first;
			std::string second;
			BOOST_FOREACH(boost::tie(first,second),data)
				ret += first + tie + enclose+ second + enclose + separate;

			if(!ret.empty()) ret.erase(ret.find_last_of(separate)-separate.size()+1);
			return ret;
		}

		//deserializeを行う
		//std::stringの処理しかできない(作者の技術力のなさで)
		std::map<std::string,std::string> deserialization(std::string data,const std::string tie="=",const std::string separate="&"){
			std::map<std::string,std::string> args;
			std::string first;
			std::string second;

			while(!data.empty()){
				first = second = data;
				first.erase(first.find(tie));
				second.erase(0,second.find(tie)+1);

				if(second.find(separate)!= std::string::npos){
					second.erase(second.find(separate));
				}
				if(data.find(separate)!= std::string::npos){
					data.erase(0,data.find(separate)+1);
				}
				else data.erase(0);

				args[first] = second;
			}

			return args;
		}

		template<typename T1,typename T2>
		inline std::map<T1,T2> map_sum(std::map<T1,T2>&data1,const std::map<T1,T2>&data2){
			T1 first;
			T2 second;
			BOOST_FOREACH(boost::tie(first,second),data2)
				data1[first] = second;

			return data1;
		}

		template<typename T1,typename T2>
		inline std::map<T1,T2> map_sub(std::map<T1,T2>&data1,const std::map<T1,T2>&data2){
			T1 first;
			T2 second;
			BOOST_FOREACH(boost::tie(first,second),data2)
				data1.erase(first);

			return data1;
		}
	}//End map namespace
	
	namespace url{
		// パーセントエンコードをする
		// base : Encode Word
		std::string encode(const std::string& base)
		{
			std::string ret = "";

			//Windowsのみ
	#ifdef _WIN32
			const std::string word = util::convert::sjis_utf8(base);
	#else
			const std::string word = base;
	#endif

			BOOST_FOREACH(char ch,word)
			/*for(;it!=it_end;it++)*/{
				if((ch>='0' && ch<='9')	
				||	(ch>='A' && ch<='Z')
				||	(ch>='a' && ch<='z')
				||	(ch=='-')
				||	(ch=='_')
				||	(ch=='.')
				||	(ch=='~'))
				{
					ret+=ch;
				}
				else
				{
					char str[4];
					sprintf_s(str,sizeof(str),"%%%02X",(unsigned char)ch);
					ret+= str;
				}
			}
			return ret;
		}

		// パーセントデコードをする
		// base : Encode Word
		std::string decode(const std::string& base)
		{
			std::string ret;
		
			std::string::const_iterator it = base.begin();
			std::string::const_iterator it_end = base.end();

			for(;it!=it_end;it++){
				if(*it!='%')
				{
					ret+=*it;
				}
				else
				{
					char str[4] = "";
					int code = 0;

					for(int i=1;i<=2;i++)
					{
						code *= 16;
						if((*(it+i) >= '0') && (*(it+i) <= '9'))
						{
							str[0] = *(it+i);
							code += atoi(str);
						}
						else
						{
							switch(*(it+i)){
							case 'A':
							case 'a':
								code +=10;
								break;
							case 'B':
							case 'b':
								code +=11;
								break;
							case 'C':
							case 'c':
								code +=12;
								break;
							case 'D':
							case 'd':
								code +=13;
								break;
							case 'E':
							case 'e':
								code +=14;
								break;
							case 'F':
							case 'f':
								code +=15;
								break;
							}
						}
					}
				
					it+=2;

					str[0] = code;
					str[1] = 0x00;
					ret+=str;
				}
			}
			return ret;
		}
	} //End url namespace

	namespace base64{
		std::string encode(const std::string& data)
		{
			//フィルタ作成
			BIO *encoder = BIO_new(BIO_f_base64());
			BIO *bmem = BIO_new(BIO_s_mem());
			encoder = BIO_push(encoder,bmem);
			BIO_write(encoder,data.c_str(),data.length());
			BIO_flush(encoder);

			//結果をここに
			BUF_MEM *bptr;
			BIO_get_mem_ptr(encoder,&bptr);
	
			//charに移行処理
			char *buff = (char *)malloc(bptr->length);
			memcpy(buff, bptr->data, bptr->length-1);
			buff[bptr->length-1] = 0;
	
			//クリア
			BIO_free_all(encoder);

			return static_cast<std::string>(buff);
		}
		std::string decode(const std::string& data)
		{
			//バッファ領域の確保。デコードデータはエンコードデータよりも小さい。
			const int length = data.length();
			char *buff = (char*)malloc(length);
			memset(buff,0x00,length);

			//フィルタ作成
			BIO *decoder = BIO_new(BIO_f_base64());
			BIO *bmem = BIO_new_mem_buf((char*)data.c_str(),length);
			bmem = BIO_push(decoder,bmem);
			BIO_read(bmem,buff,length);

			BIO_free_all(bmem);

			return static_cast<std::string>(buff);
		}
	}//End base64 namespace

	namespace crypt{
		inline std::string hmac_sha1(const std::string& key,const std::string& data)
		{
			unsigned char res[SHA_DIGEST_LENGTH + 1];
			size_t  reslen;
			size_t  keylen = key.length();
			size_t  datalen = data.length();

			HMAC (EVP_sha1(), (const unsigned char*)key.c_str(), keylen, (const unsigned char*)data.c_str(), datalen, res, &reslen);

			std::string result((char*)res);
			result.erase(reslen);

			return result;
		}

		inline std::string signature(const std::string &key,const std::string &data)
		{
			return util::url::encode(util::base64::encode(hmac_sha1(key,data)));
		}
		}

	//現在時間を取得
	inline std::string make_time(){
		char ret[32];
		time_t now;
		time(&now);
		sprintf_s(ret,"%ld",now);
		return ret;
	}

	//Nonceの作成
	inline std::string make_nonce(int len = 0)
	{
		std::string ret;
		const char base[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	
		boost::mt19937 gen( static_cast<unsigned long>(time(0)) );
		boost::uniform_smallint<> dst( 1, 61 );
		boost::variate_generator<
			boost::mt19937&, boost::uniform_smallint<>
		> rand( gen, dst );

		if(len==0){
			len = rand();
		}

		for(int i=0;i<=len;i++){
			ret+=base[rand()];
		}
		return ret;
	}
}//End util namespace

#endif
