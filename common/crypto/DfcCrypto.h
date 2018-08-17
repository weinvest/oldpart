#ifndef _DFCCRYPTO_H
#define _DFCCRYPTO_H

#include <string>
#include <openssl/des.h>
#include <boost/program_options.hpp>
#include <openssl/aes.h>
#include <openssl/blowfish.h>
#include "common/common_global.h"

bool COMMON_EXPORT AESEncrypt(std::string &ciphertext, const std::string &key, const std::string &plaintext,int & padNum);
bool COMMON_EXPORT AESDecrypt(std::string &decrypttext, const std::string &key,const std::string &ciphertext,int padNum);

bool COMMON_EXPORT AESEncrypt(char *ciphertext, int &cipherLen, const std::string &key, const char *plaintext,int plainLen,int & padNum);
bool COMMON_EXPORT AESDecrypt(char* decrypttext,int &decryptLen, const std::string &key,const char* ciphertext,int cipherLen,int padNum);

//notice: strData can't be more than 117 charater
bool COMMON_EXPORT RSAEncryptWithStr(std::string &ciphertext,const std::string& pubKeyStr, const std::string& plaintext);
bool COMMON_EXPORT RSADecryptWithStr(std::string &decrypttext , const std::string& privateKeyStr, const std::string& ciphertext);
bool COMMON_EXPORT RSAEncrypt(std::string &ciphertext,const std::string& publicKeyFileURL, const std::string& plaintext);
bool COMMON_EXPORT RSADecrypt(std::string &decrypttext , const std::string& privateKeyFileURL, const std::string& ciphertext );

bool COMMON_EXPORT BlowFishEncrypt(std::string &ciphertext, const std::string &key, const std::string &plaintext);
//#include<iostream>
//#include<openssl/aes.h>
//#include<openssl/rsa.h>

//using namespace std;

//bool AESEncrypt(std::string &_return,const std::string &key,const std::string &input_data,int &padNum);
//bool AESDecrypt(std::string &_return,const std::string &key,const std::string &input_data,int padNum);

//bool RsaEncrypt(std::string &_return,const std::string &aesKey,const std::string &pubKey);
//bool RsaDecrypt(std::string &_return,const std::string &priKeyUrl,const std::string &pubKey);
#endif

