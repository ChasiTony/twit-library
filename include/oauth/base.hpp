///////////////////////////////////////////////////////////////////////////////
// base.hpp header file
// twit-library : http://code.google.com/p/twit-library/
//
// OAuthの標準クラスだよー
// これが無いと何も始まらないぜ

#ifndef OAUTH_BASE_HPP
#define OAUTH_BASE_HPP

#include "util.hpp"

#include <string>			//string_type使用
#include <utility>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/assign.hpp>

namespace oauth{
	namespace tokens{
		//
		// トークン保存用クラス(v1.0)
		//
		class oauth_ver1{
		public:
			oauth_ver1(){}
			oauth_ver1(const string_type& c_key,const string_type& c_sec,const string_type& a_tok,const string_type& a_sec)
				: consumer_key_(c_key), consumer_secret_(c_sec), access_token_(a_tok), access_token_secret_(a_sec){oauth_ver1();}

			// key : consumer key
			// secret : consumer secret
			// scname : screen name
			void set_consumer(const string_type& key,const string_type& secret)
			{
				consumer_key_ = key;
				consumer_secret_ = secret;
			}
			void set_access(const string_type& key,const string_type& secret)
			{
				access_token_ = key;
				access_token_secret_ = secret;
			}
			void get_consumer(string_type& key,string_type& secret) const
			{
				key = consumer_key_;
				secret = consumer_secret_;
			}
			void get_access(string_type& key,string_type& secret) const
			{
				key = access_token_;
				secret = access_token_secret_;
			}

		protected:
			string_type consumer_key_;
			string_type consumer_secret_;
			string_type access_token_;
			string_type access_token_secret_;
		};

		//
		// トークン保存用クラス(v2.0)
		//
		class oauth_ver2{
		public:
			oauth_ver2(){}
			oauth_ver2(const string_type& api_key,const string_type& client_id,const string_type& client_secret,const string_type& access_token)
				: api_key_(api_key), client_id_(client_id), client_secret_(client_secret), access_token_(access_token){oauth_ver2();}
			
			void set_api_key(const string_type& api_key)
			{
				api_key_ = api_key;
			}
			void set_client_id(const string_type& client_id,const string_type& client_secret)
			{
				client_id_ = client_id;
				client_secret_ = client_secret;
			}
			void set_access(const string_type& key)
			{
				access_token_ = key;
			}

		protected:
			string_type api_key_;
			string_type client_id_;
			string_type client_secret_;
			string_type access_token_;
		};

	}//End namespace tokens

		class client_shared{
		public:
			client_shared(){}
			client_shared(const string_type& host)
				: host_(host){client_shared();}
		
			// host : host Address
			void get_host(string_type& host) const{
				host = host_;
			}
	
			void set_host(const string_type& host){
				host_ = host;
			}			

			//引数の有効期限は第一引数と同じ
			boost::asio::streambuf& make_request(
				boost::asio::streambuf& buffer,
				const string_type& method,
				const string_type& host,
				const string_type& path,
				const param_type& head,
				const string_type& body=""
				)
			{
				std::ostream sbuf(&buffer);

				//リクエスト本文の作成
				if(method=="GET" && !body.empty())
					sbuf << boost::format("%s %s?%s HTTP/1.1\r\n") %method %path %body;
				else
					sbuf << boost::format("%s %s HTTP/1.1\r\n") %method %path;

				sbuf << "Host: "+host+"\r\n";
				sbuf << "Connection: close\r\n";

				for(param_type::const_iterator it = head.begin();it != head.end();it++){
					sbuf << boost::format("%s: %s\r\n") %it->first %it->second;
				}

				if((method!="GET") && (!body.empty())){
					sbuf << "Content-Length: " << body.length() << "\r\n";
					sbuf << "\r\n";
					sbuf << body;
				}
				else
				{
					sbuf << "Content-Length: " << "0" << "\r\n";
					sbuf << "\r\n";
				}
				sbuf << std::flush;

				return (boost::asio::streambuf&)buffer;
			}

			string_type make_body(const std::vector<oauth::content::multipart_form_data>& datas,const string_type& boundary)
			{
				string_type body;

				//引数の回数だけ繰り返す
				BOOST_FOREACH(oauth::content::multipart_form_data data,datas)
				{
					if(data.content_type == content::Data)
					{
						//データの場合
						body += "--"+boundary+"\r\n";
						body += "Content-Disposition: form-data; name=\""+data.name+"\"" + "\r\n";
						body += "\r\n";
						body += data.value + "\r\n";
					}
					else if(data.content_type == content::Image)
					{
						//イメージの場合
						body += "--"+boundary+"\r\n";
						body += "Content-Disposition: form-data; name=\""+data.name+"\"; filename=\""+data.filename+"\";\r\n";
						body += "Content-Type: "+util::image::chk_type(data.filename)+"\r\n";
						body += "\r\n";
						body += util::image::loader(data.filepath+data.filename)+"\r\n";
					}
					else throw std::invalid_argument("Not Support Content-Type");
				}

				//最後のboundaryで閉じる
				body += "--"+boundary+"--"+"\r\n\r\n";

				return body;
			}

		protected:
			string_type host_;
		};
	
	namespace version{
		class oauth_ver1 : public virtual oauth::tokens::oauth_ver1 , public virtual oauth::client_shared{
		public:
			virtual std::string request(const std::string &host,boost::asio::streambuf *buffer){return "";}
			virtual std::string protocol(){return "";}
			
			virtual res_type oauth_req(const string_type& path,param_type& add_arg=param_type())
			{
				//基本情報の用意
				const string_type method = "POST";
				const string_type url = protocol()+"://"+host_+path;

				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization(make_request_signature(method,url,add_arg));
				head["Content-Type"] = "application/x-www-form-urlencoded";
				
				//ボディーの生成
				auto body = util::map::serialization(add_arg);

				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,"")
						);
							
					param_type res_sort = util::map::deserialization(res);
					access_token_ = util::url::decode(res_sort["oauth_token"]);
					access_token_secret_ = util::url::decode(res_sort["oauth_token_secret"]);

					return res;
				}
				catch(...){}
				return "";
			}

			virtual string_type oauth_authorize(const string_type& url) const
			{
				return url + "?oauth_token=" +util::url::encode(access_token_);
			}

			virtual res_type oauth_acc(const string_type& path,const string_type& pin,param_type& add_arg=param_type())
			{
				//基本情報の用意
				const string_type method = "POST";
				const string_type url = protocol()+"://"+host_+path;

				add_arg["oauth_verifier"] = pin;
				
				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization(make_basic_signature(method,url,add_arg));
				head["Content-Type"] = "application/x-www-form-urlencoded";

				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,util::map::serialization(add_arg))
						);

					param_type res_sort = util::map::deserialization(res);
					access_token_ = util::url::decode(res_sort["oauth_token"]);
					access_token_secret_ = util::url::decode(res_sort["oauth_token_secret"]);

					return res;
				}
				catch(...){}
				return "";
			}

		
			//基本的なAPIはすべて動かせるはず
			res_type request(const string_type& method,const string_type& path,param_type& add_arg=param_type())
			{
				//基本情報の用意
				const string_type url = protocol()+"://"+host_+path;

				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization(make_basic_signature(method,url,add_arg));
				head["Content-Type"] = "application/x-www-form-urlencoded";

				const auto body = util::map::serialization(add_arg);
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body)
						);
					
					return res;
				}
				catch(...){}
				return "";
			}

			//基本的なAPIはすべて動かせるはず
			res_type request(const string_type& method,const string_type& path,const std::vector<oauth::content::multipart_form_data>& data)
			{
				//基本情報の用意
				const string_type url = protocol()+"://"+host_+path;
				const string_type boundary = "---------------------------"+util::make_nonce(10);

				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization(make_basic_signature(method,url));
				head["Content-Type"] = "multipart/form-data; boundary="+boundary;

				const auto body = make_body(data,boundary);
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body)
						);

					return res;
				}
				catch(...){}
				return "";
			}

			res_type request(const oauth::api::base *api_type)
			{
				set_host(api_type->host());
								
				//基本情報の用意
				const string_type method = api_type->method();
				const string_type path = api_type->path();
				const string_type url = protocol()+"://"+host_+path;
				const param_type add_arg(api_type->sort());

				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization(make_basic_signature(method,url,add_arg));
				head["Content-Type"] = api_type->content_type();
				util::map::map_sum(head,api_type->header());

				//ボディーの作成
				std::stringstream body;
				if(head["Content-Type"] == "application/x-www-form-urlencoded")
				{
					body << util::map::serialization(add_arg);
					body << std::flush;
				}
				else if(head["Content-Type"] == "multipart/form-data")
				{
					const string_type boundary = "---------------------------"+util::make_nonce(10);
					head["Content-Type"] += ("; boundary="+boundary);
					std::vector<oauth::content::multipart_form_data> outside = api_type->data();
					body << make_body(outside,boundary);
					body << std::flush;
				}
				else if(head["Content-Type"] == "application/x.atom+xml")
				{
					using namespace boost::property_tree::xml_parser;
					boost::property_tree::ptree pt = api_type->xml_ptree();
					boost::property_tree::xml_parser::write_xml(body,pt);
					body << std::flush;
				}
				else throw std::exception("Unsupported Content-Type");
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body.str())
						);
					
					return res;
				}
				catch(...){}
				return "";				
			}

		protected:
			inline param_type get_request_args() const
			{
				param_type args;
				args["oauth_consumer_key"] = util::url::encode(consumer_key_);
				args["oauth_nonce"] = util::make_nonce();
				args["oauth_signature_method"] = "HMAC-SHA1";
				args["oauth_timestamp"] = util::make_time();
				args["oauth_version"] = "1.0";
				return args;
			}
			inline param_type get_basic_args() const
			{
				param_type args=get_request_args();
				args["oauth_token"] = util::url::encode(access_token_);
				return args;
			}
			
			//リクエストトークン取得時のみ用signature作成関数
			//戻り値: signatureを含むmap
			inline param_type make_request_signature(const string_type& method,const string_type& url,const param_type& add_arg=param_type()) const
			{
				const string_type key =util::url::encode(consumer_secret_)+"&";

				//列挙
				param_type args = this->get_request_args();

				//署名(Signature)作成
				util::map::map_sum(args,add_arg);
				args["oauth_signature"] = util::crypt::signature(key,method+"&"+util::url::encode(url)+"&"+util::url::encode(util::map::serialization(args)));
				//util::map::map_sub(args,add_arg);

				return args;
			}
			
			//signature作成関数
			//戻り値: signatureを含むmap
			inline param_type make_basic_signature(const string_type& method,const string_type& url,const param_type& add_arg=param_type()) const
			{
				const string_type key =util::url::encode(consumer_secret_)+"&"+util::url::encode(access_token_secret_);

				//列挙
				param_type args = this->get_basic_args();

				//署名(Signature)作成
				util::map::map_sum(args,add_arg);
				args["oauth_signature"] = util::crypt::signature(key,method+"&"+util::url::encode(url)+"&"+util::url::encode(util::map::serialization(args)));
				util::map::map_sub(args,add_arg);

				return args;
			}
			
			inline string_type make_authorization(const param_type& args) const
			{
				return "OAuth "+util::map::serialization(args,"=",", ","\"");
			}
		};
		
		class oauth_ver2 : public virtual oauth::tokens::oauth_ver2 , public virtual oauth::client_shared{
		public:
			virtual std::string request(const std::string &host,boost::asio::streambuf *buffer){return "";}
			virtual std::string protocol(){return "";}

			virtual std::string oauth_start(const std::string& url,param_type& add = param_type())
			{
				std::string temp = url +"?client_id="+client_id_+"&response_type=code";
				if(!add.empty()) return temp + "&" + util::map::serialization(add);
				return temp;
			}
			virtual std::string oauth_authorization_code(const std::string& path,const std::string& code,const std::string& redirect_uri,param_type& add = param_type())
			{
				std::string body = "grant_type=authorization_code&client_id=" + client_id_ + "&client_secret=" + client_secret_ + "&code=" + code +"&redirect_uri="+redirect_uri;
				if(!add.empty()) body += "&" + util::map::serialization(add);

				boost::asio::streambuf buffer;
				std::ostream sbuf(&buffer);

				param_type head;
				head["Content-Type"] = "application/x-www-form-urlencoded";
				head["Connection"] = "close";
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,"POST",host_,path,head,body)
						);
					
					return res;
				}
				catch(...){}
				return "";
			}
			virtual std::string oauth_refresh_token(const std::string& path,const std::string& refresh_token,param_type& add = param_type())
			{
				std::string body = "grant_type=refresh_token&client_id=" + client_id_ + "&client_secret=" + client_secret_ + "&refresh_token=" + refresh_token;
				if(!add.empty()) body += "&" + util::map::serialization(add);

				boost::asio::streambuf buffer;
				std::ostream sbuf(&buffer);

				param_type head;
				head["Content-Type"] = "application/x-www-form-urlencoded";
				head["Connection"] = "close";
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,"POST",host_,path,head,body)
						);
					
					return res;
				}
				catch(...){}
				return "";
			}
			
			//基本的なAPIはすべて動かせるはず
			virtual res_type request(const string_type& method,const string_type& path,const std::string& body="")
			{
				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization();

				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body)
						);

					return res;
				}
				catch(...){}
				return "";
			}
			
			//基本的なAPIはすべて動かせるはず
			virtual res_type request(const string_type& method,const string_type& path,const param_type& add_arg)
			{
				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization();
				head["Content-Type"] = "application/x-www-form-urlencoded";

				const auto body = util::map::serialization(add_arg);
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body)
						);
					
					return res;
				}
				catch(...){}
				return "";
			}
			
			res_type request(const string_type& method,const string_type& path,const std::vector<oauth::content::multipart_form_data>& data)
			{
				const string_type boundary = "---------------------------"+util::make_nonce(10);

				//ヘッダーの作成
				param_type head;
				head["Authorization"] = make_authorization();
				head["Content-Type"] = "multipart/form-data; boundary="+boundary;
				
				const auto body = make_body(data,boundary);
				
				try
				{
					//通信
					boost::asio::streambuf buffer;
					string_type res = request(
						host_,
						&make_request(buffer,method,host_,path,head,body)
						);
					
					return res;
				}
				catch(...){}
				return "";
			}
		protected:
			res_type request(const oauth::api::base *api_type)
			{
				return "";
			}
		private:
			inline string_type make_authorization() const
			{
				return "OAuth "+access_token_;
			}
		};
	}//End version namespace
	
	namespace client{
		template<class Protocol_Type,class OAuth_Version=oauth::version::oauth_ver1>
		class client_base : public OAuth_Version{
		public:
			client_base<Protocol_Type,OAuth_Version>():client_(io_service_){}
			//client_base<Protocol_Type,OAuth_Version>(const string_type& consumer_key,const string_type& consumer_secret) : client_(io_service_),consumer_key_(consumer_key),consumer_secret_(consumer_secret){}
			//client_base<Protocol_Type,OAuth_Version>(const token keys) : client_(io_service_)/*,token(keys)*/{}
			//client_base<Protocol_Type,OAuth_Version>(const std::pair<string_type,string_type>& consumers) : client_(io_service_),consumer_key(consumers.first), consumer_secret_(consumers.second){}

			virtual ~client_base<Protocol_Type,OAuth_Version>(){}

			std::string request(const std::string &host,boost::asio::streambuf *buffer)
			{
				return client_.request(host,buffer);
			}
			
			res_type request(const string_type& method,const string_type& path,param_type& add_arg=param_type())
			{
				return OAuth_Version::request(method,path,add_arg);
			}
			res_type request(const string_type& method,const string_type& path,const string_type& body)
			{
				return OAuth_Version::request(method,path,body);
			}
			res_type request(const string_type& method,const string_type& path,const std::vector<oauth::content::multipart_form_data>& data)
			{
				return OAuth_Version::request(method,path,data);
			}
			res_type request(const oauth::api::base *api_type)
			{
				return OAuth_Version::request(api_type);
			}

			std::string protocol()
			{
				return client_.protocol_;
			}

		
			boost::asio::io_service io_service_;
			Protocol_Type client_;
		};
	}//End client namespace
}//End oauth namespace

#endif
