#include <string>
#include <iostream>
#include "common/crypto/DfcCrypto.h"
void test_encrypt2_decrytp1(const char* key)
{
    //2个字符串,调用两次加密算法，解压一次
    std::string s1("I'm shgli,ok1112");
    std::string s2("I'm lanjieokkwtw");
    int32_t len = s1.length() + s2.length();
    int32_t padNum(0);
    int32_t enLen = s1.length();
    char outEncrypt[len+1];
    auto r1 = AESEncrypt(outEncrypt, enLen, key, s1.c_str(), s1.length(), padNum);
    assert(r1);
    assert(0==padNum);
    auto r2 = AESEncrypt(outEncrypt+enLen, enLen,key, s2.c_str(), s2.length(), padNum);
    assert(r2);
    assert(0==padNum);

    char outDecrypt[len+1];
    auto r3 = AESDecrypt(outDecrypt, len, key, outEncrypt, len, padNum);
    std::cout << outDecrypt << std::endl;
}

void test_encrytp1_decrypt2(const char* key)
{
    //1个字符串,调用一次加密算法，解压2次
}

int main(int argc, char** argv)
{
    auto key = argv[1];
    test_encrypt2_decrytp1(key);
    return 0;
}
