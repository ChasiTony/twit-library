#include <iostream>
#include <vector>
#include "../../include/hatena.hpp"


//1.�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g����肵�܂�
//2.�����ɓ\��t���āAdefine�ɂĂ�������
//1.Get ConsumerKey and ConsumerSecret.
//2.Paste those defines.
#define CONSUMER_KEY			"Please ConsumerKey"
#define CONSUMER_SECRET		"Please ConsumerSecret"

int main(){
	std::cout << "Standby...";
	
	//�͂Ăȗp�̃N���X���쐬���܂�
	//Make Hatena Class
	oauth::client::hatena hatena;

	//�R���V���[�}�L�[�ƃR���V���[�}�V�[�N���b�g��o�^���܂�
	//ConsumerKey and ConsumerSecret are Setting
	hatena.set_consumer(CONSUMER_KEY,CONSUMER_SECRET);

	std::cout << "\n\nConnect...";
	
	//OAuth�F��(���N�G�X�g�g�[�N���擾)
	//Scope�ɂ��Ă̎Q�� : http://developer.hatena.ne.jp/ja/documents/auth/apis/oauth/scopes
	//�X�R�[�v���R���}�łȂ��Ă�������
	//OAuth Authorize(Request Token)
	//Scope Reference : http://developer.hatena.ne.jp/ja/documents/auth/apis/oauth/scopes
	//Combines with a comma
	hatena.oauth_req("read_public,read_private");

	//�s�����擾���܂�
	//PIN Gets
	char pin[16];
	std::cout << "\n\n" << hatena.oauth_authorize() << "\n";
	std::cout << "Please PIN:";
	std::cin >> pin;

	//OAuth�F��(�A�N�Z�X�g�[�N���擾)
	//OAuth Authorize(Accsess Token)
	hatena.oauth_acc(pin);

	//API�̎g�p
	//Use APIs
	{
		hatena.set_host("b.hatena.ne.jp");
		hatena.request("GET","/atom");

		//etc.
	}
}