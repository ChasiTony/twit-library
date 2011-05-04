#ifndef TWITTER_API_ACCOUNT_HPP
#define TWITTER_API_ACCOUNT_HPP

namespace oauth{
	namespace api{
		namespace twitter{
			namespace account{
				//GET account/verify_credentials
				class verify_credentials : public twitter_base{
				public:
					verify_credentials(const string_type format)
						: twitter_base("GET","https","api.twitter.com","/1/account/verify_credentials."+format){}
				private:
				};

				//POST account/end_session
				class end_session : public twitter_base{
				public:
					end_session(const string_type format)
						: twitter_base("POST","https","api.twitter.com","/1/account/end_session."+format){}
				private:
				};

				//GET account/rate_limit_status
				class rate_limit_status : public twitter_base{
				public:
					rate_limit_status(const string_type format)
						: twitter_base("GET","https","api.twitter.com","/1/account/rate_limit_status."+format){}
				private:
				};

				//POST account/update_delivery_device
				class update_delivery_device : public twitter_base{
				public:
					update_delivery_device(const string_type format,const string_type device)
						: device_(device), twitter_base("POST","https","api.twitter.com","/1/account/update_delivery_device."+format){}
					param_type sort() const{
						param_type sort;
						if(!device_.empty()) sort["device"]=device_;
						return sort;
					}
				private:
					const string_type device_;
				};

				//POST account/update_profile_colors
				class update_profile_colors : public twitter_base{
				public:
					update_profile_colors(const string_type format,const string_type profile_background_color="",const string_type profile_text_color="",const string_type profile_link_color="",const string_type profile_sidebar_fill_color="",const string_type profile_sidebar_border_color="")
						: profile_background_color_(profile_background_color), profile_text_color_(profile_text_color), profile_link_color_(profile_link_color), profile_sidebar_fill_color_(profile_sidebar_fill_color), profile_sidebar_border_color_(profile_sidebar_border_color), twitter_base("POST","https","api.twitter.com","/1/account/update_profile_colors."+format){}
					param_type sort() const{
						param_type sort;
						if(!profile_background_color_.empty()) sort["profile_background_color"]=profile_background_color_;
						if(!profile_text_color_.empty()) sort["profile_text_color"]=profile_text_color_;
						if(!profile_link_color_.empty()) sort["profile_link_color"]=profile_link_color_;
						if(!profile_sidebar_fill_color_.empty()) sort["profile_sidebar_fill_color"]=profile_sidebar_fill_color_;
						if(!profile_sidebar_border_color_.empty()) sort["profile_sidebar_border_color"]=profile_sidebar_border_color_;
						return sort;
					}
				private:
					const string_type profile_background_color_;
					const string_type profile_text_color_;
					const string_type profile_link_color_;
					const string_type profile_sidebar_fill_color_;
					const string_type profile_sidebar_border_color_;
				};

				//POST account/update_profile_image
				class update_profile_image : public twitter_base{
				public:
					update_profile_image(const string_type format,const string_type image_path,const string_type image_name)
						: image_path_(image_path), image_name_(image_name), twitter_base("POST","https","api.twitter.com","/1/account/update_profile_image."+format,"multipart/form-data"){}
					std::vector<oauth::content::multipart_form_data> data() const{
						std::vector<oauth::content::multipart_form_data> datas;
						datas.push_back(oauth::content::multipart_form_data(oauth::content::Image,"image",image_path_,image_name_));
						
						return std::vector<oauth::content::multipart_form_data>(datas);

					}
				private:
					const string_type image_path_;
					const string_type image_name_;
				};

				//POST account/update_profile_background_image
				class update_profile_background_image : public twitter_base{
				public:
					update_profile_background_image(const string_type format,const string_type image_path,const string_type image_name,const string_type tile="")
						: image_path_(image_path), image_name_(image_name), tile_(tile), twitter_base("POST","https","api.twitter.com","/1/account/update_profile_background_image."+format,"multipart/form-data"){}
					param_type sort() const{
						param_type sort;
						if(!tile_.empty()) sort["tile"]=tile_;
						return sort;
					}
					std::vector<oauth::content::multipart_form_data> data() const{
						std::vector<oauth::content::multipart_form_data> datas;
						datas.push_back(oauth::content::multipart_form_data(oauth::content::Image,"image",image_path_,image_name_));
						
						return std::vector<oauth::content::multipart_form_data>(datas);

					}
				private:
					const string_type image_path_;
					const string_type image_name_;
					const string_type tile_;
				};

				//POST account/update_profile
				class update_profile : public twitter_base{
				public:
					update_profile(const string_type format,const string_type name="",const string_type url="",const string_type location="",const string_type description="")
						: name_(name), url_(url), location_(location), description_(description), twitter_base("POST","https","api.twitter.com","/1/account/update_profile."+format){}
					param_type sort() const{
						param_type sort;
						if(!name_.empty()) sort["name"]=name_;
						if(!url_.empty()) sort["url"]=url_;
						if(!location_.empty()) sort["location"]=location_;
						if(!description_.empty()) sort["description"]=description_;
						return sort;
					}
				private:
					const string_type name_;
					const string_type url_;
					const string_type location_;
					const string_type description_;
				};

			}//End account namespace
		}//End twitter namespace
	}//End api namespace
}//End oauth namespace

#endif
