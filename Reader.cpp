#include "Reader.h"

#include "Crypt.h"
#include "Database.h"

//#include "openssl\hmac.h"
#include <stdint.h> 

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

//typedef unsigned char*(__stdcall* _HMAC)(const EVP_MD*, const void*, int, const unsigned char*, int, unsigned char*, unsigned int*);
//
//HINSTANCE hDLL;               // Handle to DLL  
//_HMAC __HMAC;    // Function pointer  
//
//
//int hmac_it(const byte* msg, size_t mlen, byte* val, size_t* vlen, EVP_PKEY* pkey)
//{
//    /* Returned to caller */
//    int result = -1;
//
//    if (!msg || !mlen || !val || !pkey) {
//        //assert(0);
//        return -1;
//    }
//
//    if (*val)
//        OPENSSL_free(val);
//
//    //*val = NULL;
//    *vlen = 0;
//
//    EVP_MD_CTX* ctx = NULL;
//
//    do
//    {
//        ctx = EVP_MD_CTX_create();
//        //assert(ctx != NULL);
//        if (ctx == NULL) {
//            printf("EVP_MD_CTX_create failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        const EVP_MD* md = EVP_get_digestbyname("SHA256");
//        //assert(md != NULL);
//        if (md == NULL) {
//            printf("EVP_get_digestbyname failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        int rc = EVP_DigestInit_ex(ctx, md, NULL);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestInit_ex failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        rc = EVP_DigestSignInit(ctx, NULL, md, NULL, pkey);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignInit failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        rc = EVP_DigestSignUpdate(ctx, msg, mlen);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignUpdate failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        size_t req = 0;
//        rc = EVP_DigestSignFinal(ctx, NULL, &req);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignFinal failed (1), error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        //assert(req > 0);
//        if (!(req > 0)) {
//            printf("EVP_DigestSignFinal failed (2), error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        val = (byte*)OPENSSL_malloc(req);
//        //assert(*val != NULL);
//        if (val == NULL) {
//            printf("OPENSSL_malloc failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        *vlen = req;
//        rc = EVP_DigestSignFinal(ctx, val, vlen);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignFinal failed (3), return code %d, error 0x%lx\n", rc, GetLastError());
//            break; /* failed */
//        }
//
//        //assert(req == *vlen);
//        if (req != *vlen) {
//            printf("EVP_DigestSignFinal failed, mismatched signature sizes %ld, %ld", req, *vlen);
//            break; /* failed */
//        }
//
//        result = 0;
//
//    } while (0);
//
//    if (ctx) {
//        EVP_MD_CTX_destroy(ctx);
//        ctx = NULL;
//    }
//
//    /* Convert to 0/1 result */
//    return !!result;
//}
//
//int verify_it(const byte* msg, size_t mlen, const byte* val, size_t vlen, EVP_PKEY* pkey)
//{
//    /* Returned to caller */
//    int result = -1;
//
//    if (!msg || !mlen || !val || !vlen || !pkey) {
//        //assert(0);
//        return -1;
//    }
//
//    EVP_MD_CTX* ctx = NULL;
//
//    do
//    {
//        ctx = EVP_MD_CTX_create();
//        //assert(ctx != NULL);
//        if (ctx == NULL) {
//            printf("EVP_MD_CTX_create failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        const EVP_MD* md = EVP_get_digestbyname("SHA256");
//        //assert(md != NULL);
//        if (md == NULL) {
//            printf("EVP_get_digestbyname failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        int rc = EVP_DigestInit_ex(ctx, md, NULL);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestInit_ex failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        rc = EVP_DigestSignInit(ctx, NULL, md, NULL, pkey);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignInit failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        rc = EVP_DigestSignUpdate(ctx, msg, mlen);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestSignUpdate failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        byte buff[EVP_MAX_MD_SIZE];
//        size_t size = sizeof(buff);
//
//        rc = EVP_DigestSignFinal(ctx, buff, &size);
//        //assert(rc == 1);
//        if (rc != 1) {
//            printf("EVP_DigestVerifyFinal failed, error 0x%lx\n", GetLastError());
//            break; /* failed */
//        }
//
//        //assert(size > 0);
//        if (!(size > 0)) {
//            printf("EVP_DigestSignFinal failed (2)\n");
//            break; /* failed */
//        }
//
//        const size_t m = (vlen < size ? vlen : size);
//        result = !!CRYPTO_memcmp(val, buff, m);
//
//        OPENSSL_cleanse(buff, sizeof(buff));
//
//    } while (0);
//
//    if (ctx) {
//        EVP_MD_CTX_destroy(ctx);
//        ctx = NULL;
//    }
//
//    /* Convert to 0/1 result */
//    return !!result;
//}

int worker(
    int                      iNodesNumber,
    Messenger*               messenger,
    std::mutex*              pMutStdout,
    SYNCHRONIZATION_BARRIER* pBar
)
{
    prng_init((0xbad ^ 0xc0ffee ^ 42) | 0xcafebabe | 666);

    std::list<int> accepted; // list for checking accepted messages in step 3. When a message accepted, number is erased
#ifndef ADVERSARY
    for (int i = 0; i < iNodesNumber; i++)
    {
        accepted.push_back(i);
        accepted.push_back(i);
        accepted.push_back(i);
    }
#else
    accepted.push_back(ADDR_ADVERSARY);
    accepted.push_back(ADDR_ADVERSARY);
    accepted.push_back(ADDR_ADVERSARY);
#endif
    std::list<int>::iterator iter = accepted.begin();

    struct stTagData
    {
        // from tag
        uint8_t r2[NONCE_LEN * 2] = { 0 };
        uint8_t M1[NONCE_LEN * 2] = { 0 };
        uint8_t M2[NONCE_LEN * 2] = { 0 };

        // to tag
        uint8_t r1[NONCE_LEN * 2] = { 0 };
        uint8_t M3[NONCE_LEN * 2] = { 0 };
        uint8_t M4[NONCE_LEN * 2] = { 0 };

    } tmpData;
    std::vector<stTagData> vTagData(iNodesNumber, tmpData);

    Database DB(iNodesNumber);

    // initialization

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 0
    ///--------------------------------------------------------------------------//
    ///
    
    // init nodes
    for (int i = 0; i < iNodesNumber; i++)
    {
        uint8_t hk[NONCE_LEN * 2];
        uint8_t xi[NONCE_LEN * 2];
        uint8_t yi[NONCE_LEN * 2];
        DB.Get_hk(hk);
        DB.Get_xi(i, xi);
        DB.Get_yi(i, yi);

        ////char cHello[] = "Hello";
        ////Message hello(-1, i, -100, cHello, strlen(cHello));
        ////messenger->send(&hello);

        Message msg_init_xi(-1, i, 10, xi, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 10
        Message msg_init_yi(-1, i, 11, yi, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 11
        Message msg_init_hk(-1, i, 12, hk, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 12

        messenger->send(&msg_init_xi);
        messenger->send(&msg_init_yi);
        messenger->send(&msg_init_hk);
    }

#ifdef ADVERSARY
    uint8_t a_hk[NONCE_LEN * 2];
    DB.Get_hk(a_hk);
    Message msg_adv_hk(ADDR_READER, ADDR_ADVERSARY, 12, a_hk, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 12
    messenger->send(&msg_adv_hk);
#endif

    pMutStdout->lock();
    std::cout << "Reader: init sent" << std::endl;
    pMutStdout->unlock();

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 1
    ///--------------------------------------------------------------------------//
    ///

#ifndef ADVERSARY

    // 1.
    // 1.1 R generates a random nonce r_1
    for (int i = 0; i < iNodesNumber; i++)
    {
        for (int j = 0; j < NONCE_LEN; j++)
        {
            prng_next();
            vTagData[i].r1[j] = prng_next();
        }
    }
    //  1.2 R sends r_1 to T_i
    for(int i = 0; i < iNodesNumber; i++)
    {
        Message msg(-1, i, 0, vTagData[i].r1, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 0
        messenger->send(&msg);
    }

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 2
    ///--------------------------------------------------------------------------//
    ///

    /// 2. T_i generates another random nonce r_2 and computes
    ///        M_1 = x_i xor h(h(k)) xor r_2),
    ///        M_2 = h(y_i xor r_1 xor r_2)

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 3
    ///--------------------------------------------------------------------------//
    ///

    //  3. R accepts {r_2, M_1, M_2} from T_i
    while (!accepted.empty())
    {
        Message msg;
        if (messenger->recv(*iter, &msg) == -1) // ID: 1 - r_2, 2 - M_1, 3 - M_2
        {
            continue; // message was not recieved
        }

        // Handling
        switch (msg.GetID())
        {
        case 1:
        {
            memcpy(vTagData[*iter].r2, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 2:
        {
            memcpy(vTagData[*iter].M1, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 3:
        {
            memcpy(vTagData[*iter].M2, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        default:
        {
            pMutStdout->lock();
            printf("ERROR\n");
            pMutStdout->unlock();
            return -1;
        }
        }
        
        // Deletion
        iter = accepted.erase(iter);
        if (iter == accepted.end())
        {
            iter = accepted.begin();
        }
    }

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 4
    ///--------------------------------------------------------------------------//
    ///

    //  4. R transmits {r_1, r_2, M_1, M_2} to DB
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Set_r1(i, vTagData[i].r1);
        DB.Set_r2(i, vTagData[i].r2);
        DB.Set_M1(i, vTagData[i].M1);
        DB.Set_M2(i, vTagData[i].M2);
    }
    
    //  4' DB computes 
    //         x_i = M_1 xor h(h(k) xor r2)
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Compute_xi(i);
    }
    //     and
    //         y_i = f_k(x_i).
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Compute_yi(i);
    }
    //     Then checks whether 
    //         M_2 = h(y_i xor r1 xor r2).
    //     If it holds, DB authenticates T_i. 
    //     Otherwise it sends an error to R and terminates the session.
    for (int i = 0; i < iNodesNumber; i++)
    {
        if (!DB.Check_M2(i))
        {
            pMutStdout->lock();
            std::cout << "Tag " << i << ": DB check M2 error" << std::endl;
            pMutStdout->unlock();
        }
    }

    //  5. DB computes 
    //         x_i^* = h(x_i xor y_i xor r_1 xor r_2)
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Compute_xiNEW(i);
    }
    //     and new key 
    //         y_i^* = f_k(x_i^*)
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Compute_yiNEW(i);
    }

    //  6. DB calculates 
    //         M_3 = y_i^* xor h(x_i^* xor y_i),
    //         M_4 = h(x_i^* xor y_i^*),
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Compute_M3(i);
        DB.Compute_M4(i);
    //     sends {M_3, M_4} with related data of T_i to R 
        DB.Get_M3(i, vTagData[i].M3);
        DB.Get_M4(i, vTagData[i].M4);
    }
    //     and sets
    //         y_i^{old} <- y_i, 
    //         y_i       <- y_i^*
    for (int i = 0; i < iNodesNumber; i++)
    {
        DB.Set_new_yi(i);
    }

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 7
    ///--------------------------------------------------------------------------//
    ///

    //  7. R forwards M_3, M_4 to T_i
    for (int i = 0; i < iNodesNumber; i++)
    {
        ////char cHello[] = "Hello";
        ////Message hello(-1, i, -100, cHello, strlen(cHello));
        ////messenger->send(&hello);

        Message msg1(-1, i, 4, vTagData[i].M3, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 4
        Message msg2(-1, i, 5, vTagData[i].M4, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 5
        messenger->send(&msg1);
        messenger->send(&msg2);
    }

    /// 8. T_i calculates 
    ///        x_i^* = h(x_i xor y^i xor r_1 xor r_2)
    ///    and obtains
    ///        y_i^* = M_3 xor h(x_i^* xor y_i). 
    ///    After, it checks whether
    ///        M_4 = h(x_i xor y_i). 
    ///    If the check succeeds, T_i authenticates the server and sets:
    ///        x_i <- x_i^*,
    ///        y_i <- y_i^*. 
    ///    Otherwise, it keeps x_i, y_i unchanged.

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // L
    ///--------------------------------------------------------------------------//
    ///

#else // If ADVERSARY defined
    uint8_t adv_r1[NONCE_LEN * 2];
    uint8_t adv_r2[NONCE_LEN * 2];
    uint8_t adv_M1[NONCE_LEN * 2];
    uint8_t adv_M2[NONCE_LEN * 2];

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 2
    ///--------------------------------------------------------------------------//
    ///

    //Phase 2: The adversary impersonates the tag T_i to the valid server.
    ///2.1 A initiates a session with R.
    //2.2 R transmits some random nonce r_1 to A.
    for (int i = 0; i < NONCE_LEN; i++)
    {
        prng_next();
        adv_r1[i] = prng_next();
    }
    Message msg_adv_r1(ADDR_READER, ADDR_ADVERSARY, 0, adv_r1, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 0
    messenger->send(&msg_adv_r1);

    // ? to DB
    DB.Set_r1(iNodesNumber, adv_r1);

    //2.3 R recieves response {r_2, M_1, M_2} from A
    while (!accepted.empty())
    {
        Message msg;
        if (messenger->recv(*iter, &msg) == -1) // ID: 1 - r_2, 2 - M_1, 3 - M_2
        {
            continue; // message was not recieved
        }

        // Handling
        switch (msg.GetID())
        {
        case 1:
        {
            memcpy(adv_r2, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 2:
        {
            memcpy(adv_M1, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 3:
        {
            memcpy(adv_M2, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        default:
        {
            pMutStdout->lock();
            printf("ERROR\n");
            pMutStdout->unlock();
            return -1;
        }
        }

        // Deletion
        iter = accepted.erase(iter);
        if (iter == accepted.end())
        {
            iter = accepted.begin();
        }
    }

    //2.4 R delivers {r_2, M_1, M_2} to DB.
    DB.Set_r2(iNodesNumber, adv_r2);
    DB.Set_M1(iNodesNumber, adv_M1);
    DB.Set_M2(iNodesNumber, adv_M2);

    //2.5 DB computes 
    //        x_i = M_1 xor h(h(k) xor r_2),
    DB.Compute_xi(iNodesNumber);
    //        y_i = f_k(x_i).
    DB.Compute_yi(iNodesNumber);

    //2.6 M_2 = h(y_i xor r_2 xor r_1) is satisfied and DB authenticates A as T_i, because
    //        h(y_i xor r_2 xor r_1) = h(y_i xor r_1^' xor r_2^' xor r_1 xor r_1) = 
    //                               = h(y_i xor r_1^' xor r_2^') =
    //                               = M_2 = M_2^'.
    if (!DB.Check_M2(iNodesNumber))
    {
        pMutStdout->lock();
        std::cout << "Adversary: DB check M2 error" << std::endl;
        pMutStdout->unlock();
    }
    else
    {
        pMutStdout->lock();
        std::cout << "Adversary: DB check M2 success!" << std::endl;
        pMutStdout->unlock();
    }


    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // LL
    ///--------------------------------------------------------------------------//
    ///

#endif

    pMutStdout->lock();
    std::cout << "Reader off" << std::endl;
    pMutStdout->unlock();

    return 0;

    //char text[128] = "Hello, i\'m reader. ";
    //for (int i = iNodesNumber - 1; i >= 0; i--)
    //{
    //    _itoa(i, &text[19], 10);
    //    Message msg(-1, i, text, 128);
    //    messenger->send(&msg);
    //}
    //return 0;
}

Reader::Reader(int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    mutStdout = pMutStdout;

    messengerInstance = new Messenger(cPipeName, iNodesNumber);

    pReaderThread = new std::thread(worker, iNodesNumber, messengerInstance, mutStdout, pBar);
}

Reader::~Reader()
{
}

void Reader::start()
{
    pReaderThread->join();
}