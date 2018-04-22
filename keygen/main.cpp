#include <stdio.h>
#include <memory.h>
#include <iostream>
#include <string>
#include <chrono>

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

void GenerateActivationCode() {
    static char HexTable[] = "0123456789ABCDEF";
    
    char ActivationCode[30] = {};
    for (int i = 0; i < 29; ++i) {
        if (i == 5 || i == 11 || i == 17 || i == 23)
            ActivationCode[i] = '-';
        else
            ActivationCode[i] = HexTable[static_cast<unsigned int>(rand()) % 16];
    }
    
    std::cout << std::endl;
    std::cout
        << "ActivationCode:" << std::endl
        << ActivationCode << std::endl
        << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout
            << "Usage:" << std::endl
            << "    ./navicat-keygen <RSA-2048 PrivateKey file>" << std::endl
            << std::endl;
        return 0;
    }
    
    BIO* BIO_file = BIO_new_file(argv[1], "r");
    if (BIO_file == nullptr) {
        std::cout << "Failed to read file." << std::endl;
        return -1;
    }
    
    RSA* PrivateKey = PEM_read_bio_RSAPrivateKey(BIO_file, nullptr, nullptr, nullptr);
    if (PrivateKey == nullptr) {
        std::cout << "Failed to load private key." << std::endl;
        return -2;
    }
    
    BIO_free_all(BIO_file);

    srand(time(nullptr));
    GenerateActivationCode();

    time_t serverTime;
    time(&serverTime);
    
    char serverDate[32] = {};
    char activationDate[32] = {};
    char firstActivation[32] = {};
    //char nextActivation[32] = {};
    tm* serverTM = localtime(&serverTime);
    
    strftime(serverDate, sizeof(serverDate), "serverDate=%B %d, %Y", serverTM);
    strftime(activationDate, sizeof(activationDate), "activationDate=%B %d, %Y", serverTM);
    strftime(firstActivation, sizeof(firstActivation), "firstActivation=%B %d, %Y", serverTM);
    //strftime(nextActivation, sizeof(nextActivation), "nextActivation=%B %d, %Y", serverTM);
    
    std::string name;
    std::string license;
    std::string buffer;
    std::cout << "Input your name:";
    std::getline(std::cin, name);
    std::cout << "Input license name:";
    std::getline(std::cin, license);
    std::cout << "Input Activate Info:" << std::endl;
    std::getline(std::cin, buffer);
    
    char RegisterInfo[4096] = {};
    sprintf(RegisterInfo, "%1024s&errorCode=0&key_type=0&registed_name=%s&licenseName=%s&serverTime=%lld&%s&%s&%s",
            buffer.c_str(),
            name.c_str(),
            license.c_str(),
            serverTime,
            serverDate,
            activationDate,
            firstActivation);
    
    size_t RegisterInfoLength = strlen(RegisterInfo);
    BIO_file = BIO_new_file("license.bin", "wb");
    for (size_t i = 0; i < RegisterInfoLength; i += 200) {
        unsigned char enc_data[256] = { };
        RSA_private_encrypt(RegisterInfoLength - i > 200 ? 200 : RegisterInfoLength - i,
                            reinterpret_cast<unsigned char*>(RegisterInfo + i),
                            enc_data,
                            PrivateKey, RSA_PKCS1_PADDING);
        BIO_write(BIO_file, enc_data, 256);
    }
    BIO_free_all(BIO_file);
    RSA_free(PrivateKey);
    return 0;
}
