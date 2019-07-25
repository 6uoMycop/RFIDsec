#include "Node.h"
#include "Messenger.h"

#include "Crypt.h"

#include <bitset>

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


int worker(
    int                      iNodeNum, 
    int                      iNodesQuantity, 
    Messenger*               messenger, 
    SYNCHRONIZATION_BARRIER* pBar, 
    std::mutex*              pMutStdout
)
{
    // initial
    uint8_t xi[NONCE_LEN * 2]    = { 0 };
    uint8_t yi[NONCE_LEN * 2]    = { 0 }; //
    uint8_t hk[NONCE_LEN * 2]    = { 0 };

#ifndef ADVERSARY
    uint8_t r1[NONCE_LEN * 2]    = { 0 };
    uint8_t r2[NONCE_LEN * 2]    = { 0 };
    uint8_t M1[NONCE_LEN * 2]    = { 0 };
    uint8_t M2[NONCE_LEN * 2]    = { 0 };
    uint8_t M3[NONCE_LEN * 2]    = { 0 };
    uint8_t M4[NONCE_LEN * 2]    = { 0 };
    uint8_t xiNEW[NONCE_LEN * 2] = { 0 };
    uint8_t yiNEW[NONCE_LEN * 2] = { 0 }; //
#else
    uint8_t r1_[NONCE_LEN * 2]   = { 0 };
    uint8_t r2_[NONCE_LEN * 2]   = { 0 };
    uint8_t M1_[NONCE_LEN * 2]   = { 0 };
    uint8_t M2_[NONCE_LEN * 2]   = { 0 };
#endif


    prng_init((0xbad ^ 0xc0ffee ^ 42) | 666 | ((iNodeNum + 1) ^ 167));

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 0
    ///--------------------------------------------------------------------------//
    ///

    // INIT
    std::vector<Message> vInitMsg;
    while (vInitMsg.size() != 3)
    {
        Message msg;
        if (messenger->recv(-1, &msg) == 0)
        {
            vInitMsg.push_back(msg);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        switch (vInitMsg[i].GetID())
        {
        case 10: // x_i
        {
            memcpy(xi, vInitMsg[i].GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 11: // y_i
        {
            memcpy(yi, vInitMsg[i].GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        case 12: // hk
        {
            memcpy(hk, vInitMsg[i].GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
            break;
        }
        default:
        {
            pMutStdout->lock();
            std::cout << "Tag " << iNodeNum << " init error: wrong msg ID" << std::endl;
            pMutStdout->unlock();
            break;
        }
        }
    }

    /////pMutStdout->lock();
    /////std::cout << "Tag " << iNodeNum << " init OK" << std::endl;
    /////pMutStdout->unlock();

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 1
    ///--------------------------------------------------------------------------//
    ///

#ifndef ADVERSARY
        
    for (int k = 0; k < NUM_TESTS; k++)
    {

        //  1.
        /// 1.1 R generates a random nonce r_1
        //  1.2 T_i recieves r_1 from R
        Message msg_1;
        while (1)
        {
            if (messenger->recv(-1, &msg_1) == 0)
            {
                if (msg_1.GetID() == 0)
                {
                    memcpy(r1, msg_1.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
                    break;
                }
                else
                {
                    pMutStdout->lock();
                    std::cout << "Tag " << iNodeNum << " error: wrong msg ID" << std::endl;
                    pMutStdout->unlock();
                }
            }
        }

        ///
        ///--------------------------------------------------------------------------//
        EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 2
        ///--------------------------------------------------------------------------//
        ///

        //  2. T_i generates another random nonce r_2
        for (int i = 0; i < NONCE_LEN; i++)
        {
            r2[i] = prng_next();
        }
        //     and computes
        //         M_1 = x_i xor h(h(k) xor r_2),
        uint8_t tmp1_[NONCE_LEN * 2];
        uint8_t tmp1[NONCE_LEN * 2];
        for (int i = 0; i < NONCE_LEN; i++)
        {
            tmp1_[i] = hk[i] ^ r2[i];
        }
        h(tmp1_, tmp1);
        for (int i = 0; i < NONCE_LEN; i++)
        {
            M1[i] = xi[i] ^ tmp1[i];
        }
        //         M_2 = h(y_i xor r_1 xor r_2)
        uint8_t tmp2[NONCE_LEN * 2];
        for (int i = 0; i < NONCE_LEN * 2; i++)
        {
            tmp2[i] = yi[i] ^ r1[i] ^ r2[i];
        }
        h(tmp2, M2);

        /////std::cout << "tag r1= " << std::bitset<8>(r1[0]) << " " << std::bitset<8>(r1[1]) << std::endl
        /////          << "tag r2= " << std::bitset<8>(r2[0]) << " " << std::bitset<8>(r2[1]) << std::endl
        /////          << "tag yi= " << std::bitset<8>(yi[0]) << " " << std::bitset<8>(yi[1]) << std::endl
        /////          << "tag M2= " << std::bitset<8>(M2[0]) << " " << std::bitset<8>(M2[1]) << std::endl;

        ///
        ///--------------------------------------------------------------------------//
        EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 3
        ///--------------------------------------------------------------------------//
        ///

        //  3. T_i sends {r_2, M_1, M_2} to R
        Message msg_r2(iNodeNum, -1, 1, r2, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 1
        Message msg_M1(iNodeNum, -1, 2, M1, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 2
        Message msg_M2(iNodeNum, -1, 3, M2, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 3

        //////pMutStdout->lock();
        //////std::cout << "tag" << std::endl
        //////          << "yi " << std::bitset<8>(yi[0]) << " " << std::bitset<8>(yi[1]) << std::endl
        //////          << "r1 " << std::bitset<8>(r1[0]) << " " << std::bitset<8>(r1[1]) << std::endl
        //////          << "r2 " << std::bitset<8>(r2[0]) << " " << std::bitset<8>(r2[1]) << std::endl
        //////          << "M1 " << std::bitset<8>(M1[0]) << " " << std::bitset<8>(M1[1]) << std::endl
        //////          << "M2 " << std::bitset<8>(M2[0]) << " " << std::bitset<8>(M2[1]) << std::endl;
        //////pMutStdout->unlock();


        //messenger->send(&hello);
        messenger->send(&msg_r2);
        messenger->send(&msg_M1);
        messenger->send(&msg_M2);

        ///
        ///--------------------------------------------------------------------------//
        EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 4
        ///--------------------------------------------------------------------------//
        ///

        /// 4. R transmits {r_1, r_2, M_1, M_2} to DB

        /// 5. DB computes 
        ///        x_i^* = h(x_i xor y_i xor r_1 xor r_2)
        ///    and new key 
        ///        y_i^* = f_k(x_i^*)

        /// 6. DB calculates 
        ///        M_3 = y_i^* xor h(x_i^* xor y_i),
        ///        M_4 = h(x_i^* xor y_i^*),
        ///    sends {M_3, M_4} with related data of T_i to R 
        ///    and sets
        ///        y_i^{old} <- y_i, 
        ///        y_i       <- y_i^*


        ///
        ///--------------------------------------------------------------------------//
        EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 7
        ///--------------------------------------------------------------------------//
        ///

        //  7. T_i accepts M_3, M_4 from R
        std::vector<Message> vMsgPart7;
        while (vMsgPart7.size() != 2)
        {
            Message msg;
            if (messenger->recv(-1, &msg) == 0)
            {
                vMsgPart7.push_back(msg);
            }
        }

        for (int i = 0; i < 2; i++)
        {
            switch (vMsgPart7[i].GetID())
            {
            case 4: // M_3
            {
                memcpy(M3, vMsgPart7[i].GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
                break;
            }
            case 5: // M_4
            {
                memcpy(M4, vMsgPart7[i].GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
                break;
            }
            default:
            {
                pMutStdout->lock();
                std::cout << "Tag " << iNodeNum << " error: wrong msg ID" << std::endl;
                pMutStdout->unlock();
                break;
            }
            }
        }

        //  8. T_i calculates 
        //         x_i^* = h(x_i xor y^i xor r_1 xor r_2)
        uint8_t tmp3[NONCE_LEN * 2];
        for (int i = 0; i < NONCE_LEN * 2; i++)
        {
            tmp3[i] = xi[i] ^ yi[i] ^ r1[i] ^ r2[i];
        }
        h(tmp3, xiNEW);

        
        /////std::cout << "tag r1 = " << std::bitset<8>(r1[0])    << " " << std::bitset<8>(r1[1])    << std::endl
        /////          << "tag r2 = " << std::bitset<8>(r2[0])    << " " << std::bitset<8>(r2[1])    << std::endl
        /////          << "tag xi = " << std::bitset<8>(xi[0])    << " " << std::bitset<8>(xi[1])    << std::endl
        /////          << "tag yi = " << std::bitset<8>(yi[0])    << " " << std::bitset<8>(yi[1])    << std::endl
        /////          << "tag xi*= " << std::bitset<8>(xiNEW[0]) << " " << std::bitset<8>(xiNEW[1]) << std::endl
        /////          << "tag M3 = " << std::bitset<8>(M3[0])    << " " << std::bitset<8>(M3[1])    << std::endl;

        //     and obtains
        //         y_i^* = M_3 xor h(x_i^* xor y_i). 
        uint8_t tmp4[NONCE_LEN * 2];
        for (int i = 0; i < NONCE_LEN * 2; i++)
        {
            tmp4[i] = xiNEW[i] ^ yi[i];
        }
        uint8_t tmp5[NONCE_LEN * 2];
        h(tmp4, tmp5);
        for (int i = 0; i < NONCE_LEN * 2; i++)
        {
            yiNEW[i] = M3[i] ^ tmp5[i];
        }

        //     After, it checks whether
        //         M_4 = h(x_i^* xor y_i^*). 
        uint8_t tmp6[NONCE_LEN * 2];
        for (int i = 0; i < NONCE_LEN * 2; i++)
        {
            tmp6[i] = xiNEW[i] ^ yiNEW[i];
        }
        uint8_t tmp7[NONCE_LEN * 2];
        h(tmp6, tmp7);

        /////pMutStdout->lock();
        /////std::cout << "r1  " << std::bitset<8>(r1[0])    << " " << std::bitset<8>(r1[1])    << std::endl
        /////          << "r2  " << std::bitset<8>(r2[0])    << " " << std::bitset<8>(r2[1])    << std::endl
        /////          << "M1  " << std::bitset<8>(M1[0])    << " " << std::bitset<8>(M1[1])    << std::endl
        /////          << "M2  " << std::bitset<8>(M2[0])    << " " << std::bitset<8>(M2[1])    << std::endl
        /////          << "M3  " << std::bitset<8>(M3[0])    << " " << std::bitset<8>(M3[1])    << std::endl
        /////          << "M4  " << std::bitset<8>(M4[0])    << " " << std::bitset<8>(M4[1])    << std::endl
        /////          << "xi  " << std::bitset<8>(xi[0])    << " " << std::bitset<8>(xi[1])    << std::endl
        /////          << "yi  " << std::bitset<8>(yi[0])    << " " << std::bitset<8>(yi[1])    << std::endl
        /////          << "xi* " << std::bitset<8>(xiNEW[0]) << " " << std::bitset<8>(xiNEW[1]) << std::endl
        /////          << "yi* " << std::bitset<8>(yiNEW[0]) << " " << std::bitset<8>(yiNEW[1]) << std::endl
        /////          << "hk  " << std::bitset<8>(hk[0])    << " " << std::bitset<8>(hk[1])    << std::endl;
        /////pMutStdout->unlock();

        if (memcmp(M4, tmp7, NONCE_LEN * 2 * sizeof(uint8_t)) == 0)
        {
            //     If the check succeeds, T_i authenticates the server
            /////pMutStdout->lock();
            /////std::cout << "Tag " << iNodeNum << "\tsuccess with key y_i= ";
            /////for (int i = 0; i < NONCE_LEN * 2; i++)
            /////{
            /////    std::cout << std::bitset<8>(yiNEW[i]) << " ";
            /////}
            /////std::cout << std::endl;
            /////pMutStdout->unlock();
            //     and sets:
            //         x_i <- x_i^*,
            memcpy(xi, xiNEW, NONCE_LEN * 2 * sizeof(uint8_t));
            //         y_i <- y_i^*. 
            memcpy(yi, yiNEW, NONCE_LEN * 2 * sizeof(uint8_t));
        }
        else
        {
            //     Otherwise, it keeps x_i, y_i unchanged.
            pMutStdout->lock();
            std::cout << "Tag " << iNodeNum << " ERROR!" << std::endl;
            pMutStdout->unlock();
        }

        /////pMutStdout->lock();
        /////std::cout << "r1  " << std::bitset<8>(r1[0]) << " " << std::bitset<8>(r1[1]) << std::endl
        /////    << "r2  " << std::bitset<8>(r2[0]) << " " << std::bitset<8>(r2[1]) << std::endl
        /////    << "M1  " << std::bitset<8>(M1[0]) << " " << std::bitset<8>(M1[1]) << std::endl
        /////    << "M2  " << std::bitset<8>(M2[0]) << " " << std::bitset<8>(M2[1]) << std::endl
        /////    << "M3  " << std::bitset<8>(M3[0]) << " " << std::bitset<8>(M3[1]) << std::endl
        /////    << "M4  " << std::bitset<8>(M4[0]) << " " << std::bitset<8>(M4[1]) << std::endl
        /////    << "xi  " << std::bitset<8>(xi[0]) << " " << std::bitset<8>(xi[1]) << std::endl
        /////    << "yi  " << std::bitset<8>(yi[0]) << " " << std::bitset<8>(yi[1]) << std::endl
        /////    << "xi* " << std::bitset<8>(xiNEW[0]) << " " << std::bitset<8>(xiNEW[1]) << std::endl
        /////    << "yi* " << std::bitset<8>(yiNEW[0]) << " " << std::bitset<8>(yiNEW[1]) << std::endl
        /////    << "hk  " << std::bitset<8>(hk[0]) << " " << std::bitset<8>(hk[1]) << std::endl;
        /////pMutStdout->unlock();

        ///
        ///--------------------------------------------------------------------------//
        EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // L
        ///--------------------------------------------------------------------------//
        ///

    }

#else // If ADVERSARY defined
    //Phase 1: The adversary queries a tag T_i as a reader.

    //    1.1 T_i recieves random nonce r_1^' from A.
    Message msg_1;
    while (1)
    {
        if (messenger->recv(ADDR_ADVERSARY, &msg_1) == 0)
        {
            if (msg_1.GetID() == 0)
            {
                memcpy(r1_, msg_1.GetData(), NONCE_LEN * 2 * sizeof(uint8_t));
                break;
            }
            else
            {
                pMutStdout->lock();
                std::cout << "Tag " << iNodeNum << " error: wrong msg ID" << std::endl;
                pMutStdout->unlock();
            }
        }
    }

    //    1.2 T_i generates another random nonce r_2^'
    for (int i = 0; i < NONCE_LEN; i++)
    {
        r2_[i] = prng_next();
    }

    //        and computes 
    //            M_1^' = x_i xor h(h(k) xor r_2^'),
    uint8_t tmp1_[NONCE_LEN * 2];
    uint8_t tmp1[NONCE_LEN * 2];
    for (int i = 0; i < NONCE_LEN; i++)
    {
        tmp1_[i] = hk[i] ^ r2_[i];
    }
    h(tmp1_, tmp1);
    for (int i = 0; i < NONCE_LEN; i++)
    {
        M1_[i] = xi[i] ^ tmp1[i];
    }
    //         M_2 = h(y_i xor r_1^' xor r_2^')
    uint8_t tmp2[NONCE_LEN * 2];
    for (int i = 0; i < NONCE_LEN * 2; i++)
    {
        tmp2[i] = yi[i] ^ r1_[i] ^ r2_[i];
    }
    h(tmp2, M2_);

    //    1.3 T_i sends {r_2^', M_1^', M_2^'} to A.
    Message msg_r2_(iNodeNum, ADDR_ADVERSARY, 1, r2_, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 1
    Message msg_M1_(iNodeNum, ADDR_ADVERSARY, 2, M1_, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 2
    Message msg_M2_(iNodeNum, ADDR_ADVERSARY, 3, M2_, NONCE_LEN * 2 * sizeof(uint8_t)); // ID 3

    messenger->send(&msg_r2_);
    messenger->send(&msg_M1_);
    messenger->send(&msg_M2_);

    ///    1.4 A computes h(h(k) xor r_2^') and obtains 
    ///            x_i = M_1^' xor h(h(k) xor r_2^').

    ///    1.5 A keeps {x_i, r_1^', r_2^', M_2^'}.

    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // 2
    ///--------------------------------------------------------------------------//
    ///


    ///
    ///--------------------------------------------------------------------------//
    EnterSynchronizationBarrier(pBar, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY); // LL
    ///--------------------------------------------------------------------------//
    ///

#endif


    return 0;
}

Node::Node(int iNodeNum, int iNodesNumber, std::mutex* pMutStdout, SYNCHRONIZATION_BARRIER* pBar)
{
    iNodeNumber = iNodeNum;
    _itoa(iNodeNumber, &cPipeName[20], 10);

    mutStdout = pMutStdout;

    pBarrier = pBar;

    iNodesQuantity = iNodesNumber;

    messengerInstance = new Messenger(cPipeName, iNodesNumber);
    
    pNodeThread = new std::thread(worker, iNodeNumber, iNodesNumber, messengerInstance, pBarrier, mutStdout);
}


Node::~Node()
{
}

void Node::start()
{
    //pNodeThread->join();
}

void Node::GetPipeName(char* pipeName)
{
    if (pipeName != NULL)
    {
        memcpy(pipeName, cPipeName, sizeof(cPipeName));
    }
    else
    {
        exit(-3);
    }
}
