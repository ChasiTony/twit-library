#include <iostream>
#include <vector>
#include "../../include/oauth.hpp"

//1.�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g����肵�܂�
//2.�����ɓ\��t���āAdefine�ɂĂ�������
//1.Get ConsumerKey and ConsumerSecret.
//2.Paste those defines.
#define CONSUMER_KEY			"Please ConsumerKey"
#define CONSUMER_SECRET		"Please ConsumerSecret"

int main(){
	std::cout << "Standby...";
	
	//OAuth�p�̃N���X���쐬���܂�
	//����SSL�ʐM���g���̂ł���� oauth::client::twitter<oauth::protocol::https<>> oauth_base; �ɂ��Ă�������
	//Make OAuth Class
	//If use SSL "oauth::client::twitter<oauth::protocol::https<>> oauth_base;"
	oauth::client::client_base<oauth::protocol::http> oauth_base;

	//�z�X�g�̃A�h���X���w�肵�܂�
	//Host Address are Setting
	oauth_base.set_host("api.twitter.com");

	//�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g��o�^���܂�
	//ConsumerKey and ConsumerSecret are Setting
	oauth_base.set_consumer(CONSUMER_KEY,CONSUMER_SECRET);

	std::cout << "\n\nConnect...";

	//OAuth�F��(���N�G�X�g�g�[�N���擾)
	//OAuth Authorize(Request Token)
	oauth_base.oauth_req("/oauth/request_token");

	//�s�����擾���܂�
	//PIN Gets
	char pin[16];
	std::cout << "\n\n" << oauth_base.oauth_authorize("http://api.twitter.com/oauth/authorize") << "\n";
	std::cout << "Please PIN:";
	std::cin >> pin;

	//OAuth�F��(�A�N�Z�X�g�[�N���擾)
	//OAuth Authorize(Accsess Token)
	oauth_base.oauth_acc(pin,"/oauth/access_token");

	//API�̎g�p
	//Use APIs
	{
		//application/x-www-form-urlencoded
		oauth::param_type status;
		status["status"] = util::url::encode("TEST TWEET");
		oauth_base.request("POST","/statuses/update.xml",status);

		//multipart/form-data
		std::vector<oauth::content::multipart_form_data> datas;
		oauth::content::multipart_form_data(oauth::content::content_type::Image,"image","---file_path---/","---file_name---");
		oauth_base.request("POST","/account/update_profile_image.json",datas);
		
		//etc.
	}
}