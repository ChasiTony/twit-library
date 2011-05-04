#ifndef HATENA_API_HIKE_HPP
#define HATENA_API_HIKE_HPP

namespace oauth{
	namespace api{
		namespace hatena{
			namespace hike{
				namespace statuses{
					//statuses/public_timeline
					class public_timeline : public hatena_base{
					public:
						public_timeline(const string_type& format,const string_type& count="",const string_type& page="",const string_type& since="",const string_type& sort="")
							: count_(count), page_(page), since_(since), sort_(sort), hatena_base("GET","http","h.hatena.com","/api/statuses/public_timeline."+format){}
						param_type sort() const{
							param_type args;
							if(!count_.empty()) args["count"] = count_;
							if(!page_.empty()) args["page"] = page_;
							if(!since_.empty()) args["since"] = since_;
							if(!sort_.empty()) args["sort"] = sort_;

							return args;
						}
					private:
						const string_type count_;
						const string_type page_;
						const string_type since_;
						const string_type sort_;
					};
						
					//statuses/friends_timeline
					class friends_timeline : public hatena_base{
					public:
						friends_timeline(const string_type& format,const string_type& user_id,const string_type& count="",const string_type& page="",const string_type& since="",const string_type& sort="")
							: count_(count), page_(page), since_(since), sort_(sort), hatena_base("GET","http","h.hatena.com","/api/statuses/friends_timeline/"+user_id+"."+format){}
						param_type sort() const{
							param_type args;
							if(!count_.empty()) args["count"] = count_;
							if(!page_.empty()) args["page"] = page_;
							if(!since_.empty()) args["since"] = since_;
							if(!sort_.empty()) args["sort"] = sort_;

							return args;
						}
					private:
						const string_type count_;
						const string_type page_;
						const string_type since_;
						const string_type sort_;
					};
						
					//statuses/user_timeline
					class user_timeline : public hatena_base{
					public:
						user_timeline(const string_type& format,const string_type& user_id="",const string_type& count="",const string_type& page="",const string_type& since="",const string_type& sort="")
							: count_(count), page_(page), since_(since), sort_(sort), hatena_base("GET","http","h.hatena.com","/api/statuses/user_timeline" + (user_id.empty() ? "/"+user_id : "") + "."+format){}
						param_type sort() const{
							param_type args;
							if(!count_.empty()) args["count"] = count_;
							if(!page_.empty()) args["page"] = page_;
							if(!since_.empty()) args["since"] = since_;
							if(!sort_.empty()) args["sort"] = sort_;

							return args;
						}
					private:
						const string_type count_;
						const string_type page_;
						const string_type since_;
						const string_type sort_;
					};
						
					//statuses/keyword_timeline
					class keyword_timeline : public hatena_base{
					public:
						keyword_timeline(const string_type& format,const string_type& word,const string_type& count="",const string_type& page="",const string_type& since="",const string_type& sort="")
							: word_(word), count_(count), page_(page), since_(since), sort_(sort), hatena_base("GET","http","h.hatena.com","/api/statuses/keyword_timeline."+format){}
						param_type sort() const{
							param_type args;
							args["word"] = word_;
							if(!count_.empty()) args["count"] = count_;
							if(!page_.empty()) args["page"] = page_;
							if(!since_.empty()) args["since"] = since_;
							if(!sort_.empty()) args["sort"] = sort_;

							return args;
						}
					private:
						const string_type word_;
						const string_type count_;
						const string_type page_;
						const string_type since_;
						const string_type sort_;
					};
						
					//statuses/album
					class album : public hatena_base{
					public:
						album(const string_type& format,const string_type& word="",const string_type& count="",const string_type& page="",const string_type& since="",const string_type& sort="")
							: word_(word), count_(count), page_(page), since_(since), sort_(sort), hatena_base("GET","http","h.hatena.com","/api/statuses/album."+format){}
						param_type sort() const{
							param_type args;
							if(!word_.empty()) args["word"] = word_;
							if(!count_.empty()) args["count"] = count_;
							if(!page_.empty()) args["page"] = page_;
							if(!since_.empty()) args["since"] = since_;
							if(!sort_.empty()) args["sort"] = sort_;

							return args;
						}
					private:
						const string_type word_;
						const string_type count_;
						const string_type page_;
						const string_type since_;
						const string_type sort_;
					};
						
					//statuses/update
					class update : public hatena_base{
					public:
						update(const string_type& format,const string_type& status="",const string_type& keyword="",const string_type& in_reply_to_status_id="",const string_type& file_path="",const string_type& file_name="",const string_type& source="")
							: status_(status), keyword_(keyword), in_reply_to_status_id_(in_reply_to_status_id), file_path_(file_path), file_name_(file_name), source_(source), hatena_base("POST","http","h.hatena.com","/api/statuses/update."+format,"multipart/form-data"){}
						std::vector<oauth::content::multipart_form_data> data() const
						{
							std::vector<oauth::content::multipart_form_data> datas;

							if(!file_name_.empty())
								datas.push_back(oauth::content::multipart_form_data(oauth::content::Image,"file",file_path_,file_name_));
							
							if(!status_.empty())
								datas.push_back(oauth::content::multipart_form_data(oauth::content::Data,"status",status_));
								
							if(!keyword_.empty())
								datas.push_back(oauth::content::multipart_form_data(oauth::content::Data,"keyword",keyword_));
								
							if(!in_reply_to_status_id_.empty())
								datas.push_back(oauth::content::multipart_form_data(oauth::content::Data,"in_reply_to_status_id",in_reply_to_status_id_));
								
							if(!source_.empty())
								datas.push_back(oauth::content::multipart_form_data(oauth::content::Data,"source",source_));

							return std::vector<oauth::content::multipart_form_data>(datas);
						}
					private:
						const string_type status_;
						const string_type keyword_;
						const string_type in_reply_to_status_id_;
						const string_type file_path_;
						const string_type file_name_;
						const string_type source_;
					};
						
					//statuses/show
					class show : public hatena_base{
					public:
						show(const string_type& format,const string_type& user_id)
							: hatena_base("GET","http","h.hatena.com","/api/statuses/show/"+user_id+"."+format){}
					private:
					};
						
					//statuses/destroy
					class destroy : public hatena_base{
					public:
						destroy(const string_type& format,const string_type& eid,const string_type& author_url_name)
							: author_url_name_(author_url_name), hatena_base("POST","http","h.hatena.com","/api/statuses/destroy/"+eid+"."+format){}
						param_type sort() const{
							param_type args;
							args["author_url_name"] = author_url_name_;
							return args;
						}
					private:
						const string_type author_url_name_;
					};
				}//End statuses namespace

				namespace favorites{
					//favorites/create
					class create : public hatena_base{
					public:
						create(const string_type& format,const string_type& eid)
							: hatena_base("POST","http","h.hatena.com","/api/favorites/create/"+eid+"."+format){}
					private:
					};
						
					//favorites/destroy
					class destroy : public hatena_base{
					public:
						destroy(const string_type& format,const string_type& eid)
							: hatena_base("POST","http","h.hatena.com","/api/favorites/destroy/"+eid+"."+format){}
					private:
					};
				}//End favorites namespace

			}//End hike namespace
		}//End hatena namespace
	}//End api namespace
}//End oauth namespace

#endif
