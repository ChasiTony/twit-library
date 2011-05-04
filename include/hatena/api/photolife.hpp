#ifndef HATENA_API_PHOTOLIFE_HPP
#define HATENA_API_PHOTOLIFE_HPP

namespace oauth{
	namespace api{
		namespace hatena{
			namespace photolife{
				namespace atom{
					//写真を新規投稿するためのエンドポイント
					class post : public hatena_base{
					public:
						post(const string_type& title,const string_type& image_path,const string_type& image_name,const string_type& subject="",const string_type& generator="")
							: title_(title), image_path_(image_path), image_name_(image_name), subject_(subject), generator_(generator), hatena_base("POST","http","f.hatena.ne.jp","/atom/post","application/x.atom+xml"){}
						boost::property_tree::ptree xml_ptree() const
						{
							boost::property_tree::ptree pt;
							
							boost::property_tree::ptree& entry = pt.add("entry","");
							entry.put("<xmlattr>.xmlns","http://purl.org/atom/ns#");

							pt.add("entry.title",title_);

							boost::property_tree::ptree& content = pt.add("entry.content",util::base64::encode(util::image::loader(image_path_+image_name_)));
							content.put("<xmlattr>.type",util::image::chk_type(image_name_));
							content.put("<xmlattr>.mode","base64");
							
							if(!subject_.empty()){
								pt.add("entry.dc:subject",subject_);
							}
							if(!generator_.empty()){
								pt.add("entry.generator",generator_);
							}

							return pt;
						}

					private:
						const string_type title_;
						const string_type image_path_;
						const string_type image_name_;
						const string_type subject_;
						const string_type generator_;
					};

					//特定の写真エントリのフィードを参照
					class edit_get : public hatena_base{
					public:
						edit_get(const string_type& photo_id)
							: hatena_base("GET","http","f.hatena.ne.jp","/atom/edit/"+photo_id){}
					private:
					};

					//写真の編集
					class edit_put : public hatena_base{
					public:
						edit_put(const string_type& photo_id,const string_type& title="",const string_type& subject="",const string_type& generator="")
							: title_(title), subject_(subject), generator_(generator), hatena_base("POST","http","f.hatena.ne.jp","/atom/edit/"+photo_id,"application/x.atom+xml"){}
						boost::property_tree::ptree xml_ptree() const
						{
							boost::property_tree::ptree pt;
							
							boost::property_tree::ptree& entry = pt.add("entry","");
							entry.put("<xmlattr>.xmlns","http://purl.org/atom/ns#");

							if(!title_.empty()){
								pt.add("entry.title",title_);
							}						
							if(!subject_.empty()){
								pt.add("entry.dc:subject",subject_);
							}
							if(!generator_.empty()){
								pt.add("entry.generator",generator_);
							}

							return pt;
						}

					private:
						const string_type title_;
						const string_type subject_;
						const string_type generator_;
					};

					//写真の削除
					class edit_delete : public hatena_base{
					public:
						edit_delete(const string_type& photo_id)
							: hatena_base("DELETE","http","f.hatena.ne.jp","/atom/edit/"+photo_id){}
					private:
					};
				}//End atom namespace
			}//End coco namespace
		}//End hatena namespace
	}//End api namespace
}//End oauth namespace

#endif
