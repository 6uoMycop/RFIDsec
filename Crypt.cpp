#include "Crypt.h"
#include <functional>

std::hash<uint8_t> hashFunc;



void h(uint8_t* data, uint8_t* rez)
{
    // size is NONCE_LEN
    uint8_t zero[NONCE_LEN] = { 0 };
    int sz = NONCE_LEN;
    if (memcmp(&data[NONCE_LEN], zero, NONCE_LEN) != 0)
    {
        sz *= 2;
    }

    for (int i = 0; i < sz; i++)
    {
        rez[i] = hashFunc(data[i]);
    }
    
}

void f(uint8_t* key, uint8_t* data, uint8_t* rez)
{
    // size is NONCE_LEN * 2

    uint8_t buf[NONCE_LEN * 2] = { 0 };
    memcpy(buf, data, NONCE_LEN * sizeof(uint8_t));
    memcpy(&buf[NONCE_LEN], key, NONCE_LEN * sizeof(uint8_t));

    for (int i = 0; i < NONCE_LEN * 2; i++)
    {
        rez[i] = hashFunc(buf[i]);
    }
}
