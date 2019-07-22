#include "Adversary.h"

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


int adv_worker(
    int                      iNodesNumber,
    Messenger*               messenger,
    std::mutex*              pMutStdout,
    SYNCHRONIZATION_BARRIER* pBar
)
{
    uint8_t r1_[NONCE_LEN * 2] = { 0 };
    uint8_t r2_[NONCE_LEN * 2] = { 0 };
    uint8_t M1_[NONCE_LEN * 2] = { 0 };
    uint8_t M2_[NONCE_LEN * 2] = { 0 };

    uint8_t hk [NONCE_LEN * 2] = { 0 };

    uint8_t xi [NONCE_LEN * 2] = { 0 };


    uint8_t r1 [NONCE_LEN * 2] = { 0 };
    uint8_t r2 [NONCE_LEN * 2] = { 0 };
    uint8_t M1 [NONCE_LEN * 2] = { 0 };
    uint8_t M2 [NONCE_LEN * 2] = { 0 };

    prng_init((0xbad ^ 0xc0ffee ^ 42) | 0xcafebabe | 666 | 523);

    std::list<int> accepted(3, 0); // list for checking accepted messages in step 3. When a message accepted, number is erased
    std::list<int>::iterator iter = accepted.begin();
    
#ifdef ADVERSARY

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 0
    ///--------------------------------------------------------------------------//
    ///

    Message msg_hk;
    messenger->recv(ADDR_READER, &msg_hk);
    if (msg_hk.GetID() == 12)
    {
        memcpy(hk, msg_hk.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
    }
    else
    {
        std::cout << "Wrong ID" << std::endl;
        return -1;
    }

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 1
    ///--------------------------------------------------------------------------//
    ///



    //Phase 1: The adversary queries a tag T_i as a reader.
    //    1.1 A transmits some random nonce r_1^' to T_i.
    for (int i = 0; i < NONCE_LEN; i++)
    {
        prng_next();
        prng_next();
        r1_[i] = prng_next();
    }
    Message msg_r1_(ADDR_ADVERSARY, 0, 0, r1_, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 0
    messenger->send(&msg_r1_);

    ///   1.2 T_i generates another random nonce r_2 ^ ' 
    ///       and computes 
    ///   M_1^' = x_i xor h(h(k) xor r_2^'),
    ///   M_2^' = h(yi xor r_1^'xor r_2^').

    //    1.3 A recieves { r_2^', M_1^', M_2^'} from T_i.
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
            memcpy(r2_, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 2:
        {
            memcpy(M1_, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 3:
        {
            memcpy(M2_, msg.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
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
    //    1.4 A computes h(h(k) xor r_2^') and obtains 
    //            x_i = M_1^' xor h(h(k) xor r_2^').
    uint8_t in[NONCE_LEN * 2];
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        in[j] = hk[j] ^ r2_[j];
    }
    h(in, xi);
    for (int j = 0; j < NONCE_LEN * 2; j++)
    {
        xi[j] ^= M1_[j];
    }

    //    1.5 A keeps {x_i, r_1^', r_2^', M_2^'}.
    

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 2
    ///--------------------------------------------------------------------------//
    ///


    //Phase 2: The adversary impersonates the tag T_i to the valid server.
    //    2.1 A initiates a session with R.

    //    2.2 A recieves some random nonce r_1 from R.
    Message msg_r1;
    messenger->recv(ADDR_READER, &msg_r1);
    memcpy(r1, msg_r1.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));

    //    2.3 A sends response {r_2, M_1, M_2} to R, where
    //            r_2 = r_1^' xor r_2^' xor r_1,
    for (int i = 0; i < NONCE_LEN * 2; i++)
    {
        r2[i] = r1_[i] ^ r2_[i] ^ r1[i];
    }
    //            M_1 = x_i xor h(h(k) xor r_2),
    uint8_t tmp1[NONCE_LEN * 2] = { 0 };
    uint8_t in1 [NONCE_LEN * 2] = { 0 };
    for (int i = 0; i < NONCE_LEN * 2; i++)
    {
        in1[i] = hk[i] ^ r2[i];
    }
    h(in1, tmp1);
    for (int i = 0; i < NONCE_LEN * 2; i++)
    {
        M1[i] = xi[i] ^ tmp1[i];
    }
    //            M_2 = M_2^'.

    Message msg_r2(ADDR_ADVERSARY, ADDR_READER, 1, r2, NONCE_LEN * 2 * sizeof(uint8_t));  // ID 1
    Message msg_M1(ADDR_ADVERSARY, ADDR_READER, 2, M1, NONCE_LEN * 2 * sizeof(uint8_t));  // ID 2
    Message msg_M2(ADDR_ADVERSARY, ADDR_READER, 3, M2_, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 3
    messenger->send(&msg_r2);
    messenger->send(&msg_M1);
    messenger->send(&msg_M2);

    ///   2.4 R delivers {r_2, M_1, M_2} to DB.
    ///   2.5 DB computes
    ///           x_i = M_1 xor h(h(k) xor r_2),
    ///           y_i = f_k(x_i).
    ///   2.6 M_2 = h(y_i xor r_2 xor r_1) is satisfied and DB authenticates A as T_i, because
    ///           h(y_i xor r_2 xor r_1) = h(y_i xor r_1^' xor r_2^' xor r_1 xor r_1) =
    ///                                  = h(y_i xor r_1^' xor r_2^') =
    ///                                  = M_2 = M_2^'.

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // LL
    ///--------------------------------------------------------------------------//
    ///

#endif

    return 0;
}

Adversary::Adversary(int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    mutStdout = pMutStdout;

    messengerInstance = new Messenger(cPipeName, iNodesNumber);

    pAdversaryThread = new std::thread(adv_worker, iNodesNumber, messengerInstance, mutStdout, pBar);
}

Adversary::~Adversary()
{
}

void Adversary::start()
{
    pAdversaryThread->join();
}
