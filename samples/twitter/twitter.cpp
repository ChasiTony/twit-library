#include <iostream>
#include "../../include/twitter.hpp"

//1.�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g����肵�܂�
//2.�����ɓ\��t���āAdefine�ɂĂ�������
//1.Get ConsumerKey and ConsumerSecret.
//2.Paste those defines.
#define CONSUMER_KEY			"Please ConsumerKey"
#define CONSUMER_SECRET		"Please ConsumerSecret"

int main()
{
	std::cout << "Standby...";
	
	//Twitter�p�̃N���X���쐬���܂�
	//����SSL�ʐM���g���̂ł���� oauth::client::twitter<oauth::protocol::https<>> oauth_base; �ɂ��Ă�������
	//Make Twitter Class
	//If use SSL oauth::client::twitter<oauth::protocol::https<>> twitter;
	oauth::client::twitter<oauth::protocol::http> twitter;
	
	//�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g��o�^���܂�
	//ConsumerKey and ConsumerSecret are Settings
	twitter.set_consumer(CONSUMER_KEY,CONSUMER_SECRET);

	std::cout << "\n\nConnect...";

	//OAuth�F�؂��g�p
	//Use OAuth
	{
		//OAuth�F��(���N�G�X�g�g�[�N���擾)
		//OAuth Authorize(Request Token)
		twitter.oauth_req();

		//�s�����擾���܂�
		//PIN Gets
		char pin[16];
		std::cout << "\n\n" << twitter.oauth_authorize() << "\n";
		std::cout << "Please PIN:";
		std::cin >> pin;

		//OAuth�F��(�A�N�Z�X�g�[�N���擾)
		//OAuth Authorize(Accsess Token)
		twitter.oauth_acc(pin);
	}

	//xAuth�F�؂��g�p
	//Use xAuth
	{
		//ID��Password�ŔF�؂��܂�
		//Authorize with ID and Password.
		twitter.xauth("---user_ID---","---PassWord--");
	}

	//API�̎g�p
	//Use APIs
	{
		//�c�C�[�g
		//Tweet
		oauth::param_type status;
		status["status"] = util::url::encode("TEST TWEET");
		twitter.request("POST","/statuses/update.xml",status);

		//�ʐ^�̃A�b�v���[�h
		//Picture Upload
		std::vector<oauth::content::multipart_form_data> datas;
		oauth::content::multipart_form_data(oauth::content::content_type::Image,"image","---file_path---/","---file_name---");
		twitter.request("POST","/account/update_profile_image.json",datas);

		//Twitpic�̎g�p
		//Use TwitPic
		twitter.twitpic_upload("xml","---image_path---/","---image_name---","test message");

		//etc.
	}
}