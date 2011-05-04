///////////////////////////////////////////////////////////////////////////////
// protocol.hpp header file
// twit-library : http://code.google.com/p/twit-library/
//
// 標準の通信専用クラスだよー
// これでhttp/httpsの同期・非同期通信できるっ！

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>
#include <map>

#include <boost/asio.hpp> //Boost.Asio用ヘッダ
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/exception/all.hpp>
#include <boost/thread.hpp>

//Boost.AsioでSSLを使用するために必要(OpenSSL)
#ifndef NO_USE_SSL
#pragma warning(disable: 4996)
#include <boost/asio/ssl.hpp>
#pragma comment(lib, "libeay32MDd.lib")
#pragma comment(lib, "ssleay32MDd.lib")
#pragma warning(default: 4996)
#endif

#include "util.hpp"

namespace oauth{
	typedef boost::error_info<struct status_code,int> err_status_code;
	typedef boost::error_info<struct response_header,std::string> err_response_header;
	typedef boost::error_info<struct response_body,std::string> err_response_body;

	class response_error : public boost::exception, public std::exception
	{
	public:
		typedef std::string response_type;
		response_error(const int status_code,const response_type& response_header,const response_type& response_body="")
		{
			*this << err_status_code(status_code);
			*this << err_response_header(response_header);
			*this << err_response_body(response_body);
		}
		virtual ~response_error(){}

	};

	namespace protocol{
		typedef std::string string_type;

		class sync_base{
		public:
			sync_base(boost::asio::io_service& io_service,string_type protocol)
				: io_service_(io_service), resolver_(io_service), protocol_(protocol)
			{}
			~sync_base()
			{}
			virtual std::string request(const std::string& host,boost::asio::streambuf *request){return "";}
			const string_type protocol_;
		protected:
			template<class Socket>
			std::string read(Socket* socket)
			{
				boost::system::error_code err;
				boost::asio::streambuf buffer;
				while(boost::asio::read(*socket,buffer,boost::asio::transfer_at_least(1),err));
		
				std::istream response_stream(&buffer);
				std::string http_version;
				response_stream >> http_version;
				unsigned int status_code;
				response_stream >> status_code;
				std::string status_message;
				response_stream >> status_message;
		
				auto ret = static_cast<std::string>(boost::asio::buffer_cast<const char*>(buffer.data()));
		
				if (!response_stream || http_version.substr(0, 5) != "HTTP/")
				{
					BOOST_THROW_EXCEPTION(response_error(0,ret));
				}

				std::string::size_type chunk = ret.find("Transfer-Encoding: chunked");				
				std::string header = ret.substr(0,ret.find("\r\n\r\n"));
				ret.erase(0,ret.find("\r\n\r\n")+4);
				
				if(chunk != std::string::npos)	ret = remove_chunk(ret); //チャンク処理
				
				if ((200<=status_code)&&(status_code<300))
				{
					//ステータスコード200番台ならレスポンスを返してあげる
					return ret;
				}
				else
				{
					//ステータスコード200番台以外なら意味不明だから例外投げる。
					//後処理はcatchでよろしく＾＾
					BOOST_THROW_EXCEPTION(response_error(status_code,header,ret));
				}

				return ""; //異常というか何故ここに来る？
			}
			std::string remove_chunk(std::string base) const
			{
				std::string res;
				while(!base.empty())
				{
					unsigned int chunk;					
					chunk = util::convert::hex_to_dec(base.substr(0,base.find("\r\n")));
					base.erase(0,base.find("\r\n")+2);

					std::string str;
					while(str.length() < chunk)
					{
						str += base.substr(0,base.find("\r\n"));
						base.erase(0,base.find("\r\n")+2);
					}
					res += str;
				}

				return res;
			}
			boost::asio::io_service& io_service_;	
			boost::asio::ip::tcp::resolver resolver_;
		};

		class async_base{
		public:
			async_base(boost::asio::io_service& io_service,string_type* response,string_type protocol)
				: io_service_(io_service), work_(std::auto_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(io_service))), resolver_(io_service), protocol_(protocol)
			{
				response_ = response;
				disconnect = false;
				thread = boost::thread(boost::bind(&boost::asio::io_service::run,&io_service_));
			}
			~async_base()
			{
				chunk_thread.detach();
				chunk_thread.join();
				thread.detach();
				thread.join();
				io_service_.post(boost::bind(&boost::asio::io_service::stop,&io_service_));
				//io_service_.stop();
				//io_service_.reset();
			}

			virtual void start(const std::string& host,boost::asio::streambuf *request){}
			virtual void stop(){}
			const string_type protocol_;

		protected:
			//ボディー内が0x00-0x7Fまでだったら正常に動く。
			//マルチバイト文字列が含まれていると予期しない動作。
			//そのへんは暇なときに考える。
			void chunk_process()
			{
				std::string cushion; //長さを測るためのクッション
				unsigned int data_len=0; //チャンクに書いてあったバイト数を保存
				while(!disconnect) //ソケットがつながっている限り、回し続ける。
				{
					if(buffer.find("\r\n") != std::string::npos) //バッファ内に文末が現れたら
					{
						if(data_len==0) //チャンクを取る
						{
							data_len = util::convert::hex_to_dec(buffer.substr(0,buffer.find("\r\n")));
							buffer.erase(0,buffer.find("\r\n")+2);
						}
						else //もうチャンクサイズが解ってる場合
						{									
							while(cushion.length() < data_len) //既定サイズに達するまで
							{
								while(buffer.find("\r\n") == std::string::npos); //バッファに\r\nが見えるまで止める。
								cushion += buffer.substr(0,buffer.find("\r\n")+2);
								buffer.erase(0,buffer.find("\r\n")+2);
							}
							data_len = 0;
						}
						*response_ += cushion; //クッションから書き出し
						cushion = "";
					}
				}
			}
			//改行があったら書き出すだけの簡単実装。
			void read_process()
			{
				while(!disconnect)
				{
					while(buffer.find("\r\n") == std::string::npos); //バッファに\r\nが見えるまで止める。
					*response_ += buffer.substr(0,buffer.find("\r\n")+2);
					buffer.erase(0,buffer.find("\r\n")+2);
				}
			}

			//ヘッダー処理関数
			std::string header_process(std::string &data)
			{
				//最初のみ、ステータスコードの読み取り
				if(first_read)
				{
					//レスポンスチェック
					std::istringstream response_stream(data);
					std::string http_version;
					response_stream >> http_version;
					unsigned int status_code;
					response_stream >> status_code;
					std::string status_message;
					
					if (!response_stream || http_version.substr(0, 5) != "HTTP/")
					{
						BOOST_THROW_EXCEPTION(response_error(0,data));
					}
					
					if ((200<=status_code)&&(status_code<300))
					{
						//ステータスコード200番台ならチャンクの有無によってスレッド操作の変更
						chunked = (data.find("Transfer-Encoding: chunked") != std::string::npos) ? true : false;					
						if(chunked)	chunk_thread = boost::thread(boost::bind(&async_base::chunk_process,this));
						else				chunk_thread = boost::thread(boost::bind(&async_base::read_process,this));
					}
					else
					{
						//ステータスコード200番台以外なら意味不明だから例外投げる。
						//後処理はcatchでよろしく＾＾
						std::string header = data.substr(0,data.find("\r\n\r\n"));
						data.erase(0,data.find("\r\n\r\n")+4);
						BOOST_THROW_EXCEPTION(response_error(status_code,header,data));
					}
									
					//ヘッダー切り捨て
					data.erase(0,data.find("\r\n\r\n")+4);

					first_read=false; //もうこのブロックは読ませない
				}

				return data;
			}
			
			boost::asio::io_service& io_service_;	
			std::auto_ptr<boost::asio::io_service::work> work_;
			boost::asio::ip::tcp::resolver resolver_;

			boost::thread thread;
			boost::thread chunk_thread;

			bool chunked;
			bool first_read;
			bool disconnect;

			std::string *response_;
			std::string buffer;
		};

		class http : public sync_base{
		public:
			http(boost::asio::io_service& io_service)
				: sync_base(io_service,"http"), socket_(io_service)
			{
			}
			~http()
			{
			}

			std::string request(const std::string& host,boost::asio::streambuf *request)
			{
				boost::asio::ip::tcp::resolver::query query(host,protocol_);
				boost::asio::ip::tcp::resolver::iterator pos = resolver_.resolve(query);
				boost::asio::ip::tcp::resolver::iterator end;

				boost::system::error_code err = boost::asio::error::host_not_found;
				while(err && pos != end)
				{
					socket_.close();
					socket_.connect(*pos++,err);
				}
				if(err) throw boost::system::system_error(err);

				boost::asio::write(socket_,*request);
				return read(&socket_);
			}

		private:
			boost::asio::ip::tcp::socket socket_;
		};

		class async_http : public async_base{
		public:
			async_http(boost::asio::io_service& io_service,string_type* response)
				: async_base(io_service,response,"http"), socket_(io_service)
			{
			}

			~async_http(){
			}

			void start(const std::string& host,boost::asio::streambuf *request){
				std::ostream os(&request_);
				os << request;
				boost::asio::ip::tcp::resolver::query query(host, protocol_);
				resolver_.async_resolve(
					query,
					boost::bind(&async_http::handle_resolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator)
					);
			}
			void stop()
			{
				disconnect = true;
				socket_.close();
			}

		private:
			//async_resolve後ハンドル
			void handle_resolve(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.async_connect(
						endpoint,
						boost::bind(&async_http::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_connect後ハンドル
			void handle_connect(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常接続
					boost::asio::async_write(
						socket_,
						request_,
						boost::bind(&async_http::handle_write_request, this, boost::asio::placeholders::error)
						);
				}
				else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
				{
					//接続ミス
					socket_.close();
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.async_connect(
						endpoint,
						boost::bind(&async_http::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);

				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_write後ハンドル
			void handle_write_request(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常送信
					first_read = true;
					chunked = false;
					read();
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			void read()
			{
				response_stream_ = new boost::asio::streambuf();
				boost::asio::async_read(
						socket_,
						*response_stream_,
						boost::asio::transfer_at_least(1),
						boost::bind(&async_http::handle_read,this,boost::asio::placeholders::error)
						);
			}

			//readハンドル
			void handle_read(const boost::system::error_code& err)
			{
				if(!err)
				{
					buffer += header_process((static_cast<std::string>(boost::asio::buffer_cast<const char*>(response_stream_->data()))).substr(0,response_stream_->size()));
					delete response_stream_;
					response_stream_ = NULL;

					if(disconnect)
						return;

					read();
				}
				else if (err == boost::asio::error::eof || err == boost::asio::error::shut_down )
				{
					return;
				}
			}

			boost::asio::ip::tcp::socket socket_;
			boost::asio::streambuf request_;
			boost::asio::streambuf *response_stream_;
		};

#ifndef NO_USE_SSL
		template<boost::asio::ssl::context_base::method SSL_TYPE = boost::asio::ssl::context::sslv23>
		class https : public sync_base{
		public:
			https(boost::asio::io_service& io_service)
				: sync_base(io_service,"https"), ctx_(io_service_,SSL_TYPE), socket_(io_service_,ctx_)/*, resolver_(io_service_)*/
			{
			}
			~https()
			{
			}

			std::string request(const std::string& host,boost::asio::streambuf *request)
			{
				boost::asio::ip::tcp::resolver::query query(host,protocol_);
				boost::asio::ip::tcp::resolver::iterator pos = resolver_.resolve(query);
				boost::asio::ip::tcp::resolver::iterator end;

				boost::system::error_code err = boost::asio::error::host_not_found;
				while(err && pos != end)
				{
					socket_.lowest_layer().close();
					socket_.lowest_layer().connect(*pos++,err);
					socket_.handshake(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::client);
				}
				if(err) throw boost::system::system_error(err);

				boost::asio::write(socket_,*request);		
				return read(&socket_);
			}

		private:
			boost::asio::ssl::context ctx_;
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
		};
		
		template<boost::asio::ssl::context_base::method SSL_TYPE = boost::asio::ssl::context::sslv23>
		class async_https : public async_base{
		public:
			async_https(boost::asio::io_service& io_service,string_type* response)
				: async_base(io_service,response,"https"), ctx_(io_service,SSL_TYPE), socket_(io_service,ctx_)
			{
			}

			~async_https(){
			}

			void start(const std::string& host,boost::asio::streambuf *request){
				std::ostream os(&request_);
				os << request;
				boost::asio::ip::tcp::resolver::query query(host, protocol_);
				resolver_.async_resolve(
					query,
					boost::bind(&async_https::handle_resolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator)
					);
			}
						
			void stop(){
				disconnect=true;
				socket_.lowest_layer().close();
			}
				
		private:
			//async_resolve後ハンドル
			void handle_resolve(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.lowest_layer().async_connect(
						endpoint,
						boost::bind(&async_https::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);

				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_connect後ハンドル
			void handle_connect(const boost::system::error_code& err,boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
			{
				if(!err)
				{
					//正常接続
					socket_.async_handshake(
						boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::client,
						boost::bind(&async_https::handle_handshake, this, boost::asio::placeholders::error)
						);		
				}
				else if(endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
				{
					//接続ミス
					socket_.lowest_layer().close();
					boost::asio::ip::tcp::endpoint endpoint= *endpoint_iterator;
					socket_.lowest_layer().async_connect(
						endpoint,
						boost::bind(&async_https::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator)
						);

				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_handshake後ハンドル
			void handle_handshake(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常ハンドシェイク
					boost::asio::async_write(
					socket_,
					request_,
					boost::bind(&async_https::handle_write_request, this, boost::asio::placeholders::error)
					);
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			//async_write後ハンドル
			void handle_write_request(const boost::system::error_code& err)
			{
				if(!err)
				{
					//正常送信
					first_read = true;
					chunked = false;
					read();
				}
				else throw std::invalid_argument("Error: "+err.message());
			}

			void read()
			{
				response_stream_ = new boost::asio::streambuf();
				boost::asio::async_read(
						socket_,
						*response_stream_,
						boost::asio::transfer_at_least(1),
						boost::bind(&async_https::handle_read,this,boost::asio::placeholders::error)
						);
			}

			//readハンドル
			void handle_read(const boost::system::error_code& err)
			{
				if(!err)
				{
					buffer += header_process((static_cast<std::string>(boost::asio::buffer_cast<const char*>(response_stream_->data()))).substr(0,response_stream_->size()));
					delete response_stream_;
					response_stream_ = NULL;

					if(disconnect)
						return;

					read();
				}
				else if (err == boost::asio::error::eof || err == boost::asio::error::shut_down )
				{
					std::cout << "Error: "+err.message();
				}
			}

			boost::asio::ssl::context ctx_;
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
			boost::asio::streambuf request_;
			boost::asio::streambuf *response_stream_;
		};
#endif

	}//End protocol namespace
}//End oauth namespace

#endif
