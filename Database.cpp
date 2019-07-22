#include "Database.h"

#include <iostream>
#include <bitset>

//vv
////

typedef struct
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} prng_t;

static prng_t prng_ctx;

static uint32_t prng_rotate(uint32_t x, uint32_t k)
{
    return (x << k) | (x >> (32 - k));
}

static uint32_t prng_next(void)
{
    uint32_t e = prng_ctx.a - prng_rotate(prng_ctx.b, 27);
    prng_ctx.a = prng_ctx.b ^ prng_rotate(prng_ctx.c, 17);
    prng_ctx.b = prng_ctx.c + prng_ctx.d;
    prng_ctx.c = prng_ctx.d + e;
    prng_ctx.d = e + prng_ctx.a;
    return prng_ctx.d;
}

static void prng_init(uint32_t seed)
{
    uint32_t i;
    prng_ctx.a = 0xf1ea5eed;
    prng_ctx.b = seed;
    prng_ctx.c = seed;
    prng_ctx.d = seed;

    for (i = 0; i < 31; ++i)
    {
        (void)prng_next();
    }
}

////
//^^

Database::Database(int iNodesQuantity)
{
    prng_init((0xbad ^ 0xc0ffee ^ 42) | 0xcafebabe ^ 666 ^ 92);

    for (int i = 0; i < NONCE_LEN; i++)
    {
        k[i] = prng_next();
    }

    h(k, hk);

    for (int i = 0; i < iNodesQuantity; i++)
    {
        stData tmp;
        for (int i = 0; i < NONCE_LEN; i++)
        {
            tmp.xi[i] = prng_next();
        }
        f(k, tmp.xi, tmp.yi);
        vData.push_back(tmp);
    }

#ifdef ADVERSARY
    stData adv_tmp;
    vData.push_back(adv_tmp);
#endif
}

Database::~Database()
{
}

void Database::Get_xi(int iTagNum, uint8_t* xi)
{
    memcpy(xi, vData[iTagNum].xi, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Get_yi(int iTagNum, uint8_t* yi)
{
    memcpy(yi, vData[iTagNum].yi, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Get_hk(uint8_t* _hk)
{
    memcpy(_hk, hk, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Get_M3(int iTagNum, uint8_t* M3)
{
    memcpy(M3, vData[iTagNum].M3, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Get_M4(int iTagNum, uint8_t* M4)
{
    memcpy(M4, vData[iTagNum].M4, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Set_r1(int iTagNum, uint8_t* r1)
{
    memcpy(vData[iTagNum].r1, r1, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Set_r2(int iTagNum, uint8_t* r2)
{
    memcpy(vData[iTagNum].r2, r2, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Set_M1(int iTagNum, uint8_t* M1)
{
    memcpy(vData[iTagNum].M1, M1, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Set_M2(int iTagNum, uint8_t* M2)
{
    memcpy(vData[iTagNum].M2, M2, NONCE_LEN * 2 * sizeof uint8_t);
}

void Database::Set_new_yi(int iTagNum)
{
    memcpy(vData[iTagNum].yiOLD, vData[iTagNum].yi,    NONCE_LEN * 2 * sizeof(uint8_t));
    memcpy(vData[iTagNum].yi,    vData[iTagNum].yiNEW, NONCE_LEN * 2 * sizeof(uint8_t));
}

void Database::Compute_xi(int iTagNum)
{
    uint8_t in[NONCE_LEN * 2];
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = hk[j] ^ vData[iTagNum].r2[j];
    }
    h(in, vData[iTagNum].xi);
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        vData[iTagNum].xi[j] ^= vData[iTagNum].M1[j];
    }
}

void Database::Compute_yi(int iTagNum)
{
    f(k, vData[iTagNum].xi, vData[iTagNum].yi);
}

bool Database::Check_M2(int iTagNum)
{
    uint8_t in[NONCE_LEN * 2];
    uint8_t rez[NONCE_LEN * 2];

    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = vData[iTagNum].yi[j] ^ vData[iTagNum].r1[j] ^ vData[iTagNum].r2[j];
    }
    h(in, rez);

    if (memcmp(vData[iTagNum].M2, rez, NONCE_LEN * 2 * sizeof(uint8_t)) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Database::Compute_xiNEW(int iTagNum)
{
    uint8_t in[NONCE_LEN * 2];
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = vData[iTagNum].xi[j] ^ vData[iTagNum].yi[j] ^ vData[iTagNum].r1[j] ^ vData[iTagNum].r2[j];
    }
    h(in, vData[iTagNum].xiNEW);
}

void Database::Compute_yiNEW(int iTagNum)
{
    f(k, vData[iTagNum].xiNEW, vData[iTagNum].yiNEW);
}

void Database::Compute_M3(int iTagNum)
{
    uint8_t hash[NONCE_LEN * 2];
    uint8_t in[NONCE_LEN * 2];
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = vData[iTagNum].xiNEW[j] ^ vData[iTagNum].yi[j];
    }
    h(in, hash);

    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        vData[iTagNum].M3[j] = vData[iTagNum].yiNEW[j] ^ hash[j];
    }
}

void Database::Compute_M4(int iTagNum)
{
    uint8_t in[NONCE_LEN * 2];
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = vData[iTagNum].xiNEW[j] ^ vData[iTagNum].yiNEW[j];
    }
    h(in, vData[iTagNum].M4);
}

