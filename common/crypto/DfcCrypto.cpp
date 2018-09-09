#include "common/crypto/DfcCrypto.h"
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <cassert>
#include <stdio.h>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <boost/filesystem.hpp>
#include<openssl/aes.h>
#include<string.h>
//#include<malloc.h>
using namespace std;


#define AES_BLOCK_SIZE 16

bool  AESEncrypt(std::string &ciphertext, const std::string &key, const std::string &plaintext,int & padNum)
{
       AES_KEY aes;
       unsigned char iv[AES_BLOCK_SIZE];
       if(AES_set_encrypt_key((const unsigned char *)key.c_str(),AES_BLOCK_SIZE*8,&aes)<0)
       {
           return false;
       }
       unsigned int len=0;
       if((plaintext.length())%AES_BLOCK_SIZE==0)
       {
           len=plaintext.length();
           padNum = 0;
       }
       else
       {
           len=((plaintext.length())/AES_BLOCK_SIZE+1)*AES_BLOCK_SIZE;
           padNum = len - plaintext.length();
       }
       for(int i=0;i<16;i++)
       {
           iv[i]=0;
       }
       ciphertext.resize(len);
       AES_cbc_encrypt((const unsigned char *)plaintext.c_str(),(unsigned char*)ciphertext.c_str(),len,&aes,iv,AES_ENCRYPT);
       return true;
}

bool  AESEncrypt(char *ciphertext, int &cipherLen, const std::string &key, const char *plaintext,int plainLen,int & padNum)
{
       AES_KEY aes;
       unsigned char iv[AES_BLOCK_SIZE];
       if(AES_set_encrypt_key((const unsigned char *)key.c_str(),AES_BLOCK_SIZE*8,&aes)<0)
       {
           return false;
       }
       unsigned int len=0;
       if(plainLen%AES_BLOCK_SIZE==0)
       {
           len=plainLen;
           padNum = 0;
       }
       else
       {
           len=((plainLen)/AES_BLOCK_SIZE+1)*AES_BLOCK_SIZE;
           padNum = len - plainLen;
       }
       memset(iv,0,16);
       if(cipherLen<len)
           return false;
       cipherLen=len;
       AES_cbc_encrypt((const unsigned char *)plaintext,(unsigned char*)ciphertext,len,&aes,iv,AES_ENCRYPT);
       return true;
}

bool  AESDecrypt(std::string &decrypttext, const std::string &key,const std::string &ciphertext,int padNum)
{
    AES_KEY aes;
    unsigned char iv[AES_BLOCK_SIZE];
    if(AES_set_decrypt_key((const unsigned char *)key.c_str(),AES_BLOCK_SIZE*8,&aes)<0)
    {
        return false;
    }
    unsigned int len=0;
    len=ciphertext.length();
    for(int i=0;i<16;i++)
    {
        iv[i]=0;
    }
    decrypttext.resize(len);
    AES_cbc_encrypt((const unsigned char *)ciphertext.c_str(),(unsigned char*)decrypttext.c_str(),len,&aes,iv,AES_DECRYPT);
    decrypttext.resize(len - padNum);
    return true;
}

bool  AESDecrypt(char* decrypttext,int &decryptLen, const std::string &key,const char* ciphertext,int cipherLen,int padNum)
{
    AES_KEY aes;
    unsigned char iv[AES_BLOCK_SIZE];
    if(AES_set_decrypt_key((const unsigned char *)key.c_str(),AES_BLOCK_SIZE*8,&aes)<0)
    {
        return false;
    }
    unsigned int len=0;
    len=cipherLen;
    memset(iv,0,16);

    if(decryptLen<len - padNum)
        return false;
    decryptLen=len;
    AES_cbc_encrypt((const unsigned char *)ciphertext,(unsigned char*)decrypttext,len,&aes,iv,AES_DECRYPT);
    decryptLen=len - padNum;

    return true;
}

bool  RSAEncryptWithStr(std::string &ciphertext,const std::string& pubKeyStr, const std::string& plaintext)
{
    BIO* pBufIO = BIO_new_mem_buf((void*)pubKeyStr.c_str(), pubKeyStr.length());
    RSA* pRSAPublicKey = RSA_new();
    if(nullptr == pBufIO || (pRSAPublicKey = PEM_read_bio_RSA_PUBKEY(pBufIO, 0, 0, 0)) == NULL)
    {
        return false;
    }

    int nLen = RSA_size(pRSAPublicKey);
    char* pEncode = new char[nLen + 1];
    int ret = RSA_public_encrypt(plaintext.length(), (const unsigned char*)plaintext.c_str(), (unsigned char*)pEncode, pRSAPublicKey, RSA_PKCS1_PADDING);
    if (ret >= 0)
    {
        ciphertext = std::string(pEncode, ret);
    }
    delete[] pEncode;
    RSA_free(pRSAPublicKey);
    CRYPTO_cleanup_all_ex_data();
    return true;
}

bool  RSAEncrypt(std::string &ciphertext,const std::string& publicKeyFileURL, const std::string& plaintext)
{
    if (publicKeyFileURL.empty() || plaintext.empty())
    {
        return false;
    }

    ifstream fKey(publicKeyFileURL.c_str(),std::ios::binary | std::ios::in);
    istreambuf_iterator<char> beg(fKey), end;
    std::string keyString(beg,end);

    return RSAEncryptWithStr(ciphertext,keyString,plaintext);
}

bool  RSADecryptWithStr(std::string &decrypttext , const std::string& privateKeyStr, const std::string& ciphertext)
{
    RSA* pRSAPriKey = RSA_new();
    BIO* pBufIO = BIO_new_mem_buf((void*)privateKeyStr.c_str(), privateKeyStr.length());
    if(nullptr == pBufIO || (pRSAPriKey = PEM_read_bio_RSAPrivateKey(pBufIO, &pRSAPriKey, 0, 0)) == NULL)
    {
        return false;
    }

    int nLen = RSA_size(pRSAPriKey);
    char* pDecode = new char[nLen+1];

    int ret = RSA_private_decrypt(ciphertext.length(), (const unsigned char*)ciphertext.c_str(), (unsigned char*)pDecode, pRSAPriKey, RSA_PKCS1_PADDING);
    if(ret >= 0)
    {
        decrypttext = std::string((char*)pDecode, ret);
    }
    delete [] pDecode;
    RSA_free(pRSAPriKey);
    CRYPTO_cleanup_all_ex_data();
    return true;
}

bool  RSADecrypt( std::string &decrypttext ,const std::string& privateKeyFileURL, const std::string& ciphertext )
{
    if (privateKeyFileURL.empty() || ciphertext.empty())
    {
        return false;
    }

    ifstream fKey(privateKeyFileURL.c_str(),std::ios::binary | std::ios::in);
    istreambuf_iterator<char> beg(fKey), end;
    std::string keyString(beg,end);

    return RSADecryptWithStr(decrypttext,keyString,ciphertext);
}


bool BlowFishEncrypt(string &ciphertext, const string &key, const string &plaintext)
{
    BF_KEY stBF_Key = {0};
    char ivec[32] = {0};

    BF_set_key(&stBF_Key, key.length(), reinterpret_cast<const unsigned char*>(key.c_str()));
    int nNum = 0;
    unsigned char po_pszDataEncrypt[100] = {0};
    BF_cfb64_encrypt((unsigned char *)plaintext.c_str(), (unsigned char *)po_pszDataEncrypt, plaintext.size(),
                     &stBF_Key, (unsigned char *)&ivec, &nNum, BF_ENCRYPT);
    ciphertext = std::string(reinterpret_cast<const char*>(po_pszDataEncrypt));
    return nNum != 0;
}
