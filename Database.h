#pragma once
#include "defines.h"
#include "Crypt.h"
#include <vector>

class Database
{
public:
    Database(int iNodesQuantity);
    ~Database();

    struct stData
    {
        uint8_t xi[NONCE_LEN * 2]    = { 0 };
        uint8_t yi[NONCE_LEN * 2]    = { 0 };

        uint8_t xiNEW[NONCE_LEN * 2] = { 0 };
        uint8_t yiNEW[NONCE_LEN * 2] = { 0 };

        uint8_t yiOLD[NONCE_LEN * 2] = { 0 };

        //

        uint8_t r1[NONCE_LEN * 2] = { 0 };
        uint8_t r2[NONCE_LEN * 2] = { 0 };

        uint8_t M1[NONCE_LEN * 2] = { 0 };
        uint8_t M2[NONCE_LEN * 2] = { 0 };

        uint8_t M3[NONCE_LEN * 2] = { 0 };
        uint8_t M4[NONCE_LEN * 2] = { 0 };
    };

    void Get_xi(int iTagNum, uint8_t* xi);
    void Get_yi(int iTagNum, uint8_t* yi);
    void Get_hk(int iTagNum, uint8_t* hk);
    void Get_M3(int iTagNum, uint8_t* M3);
    void Get_M4(int iTagNum, uint8_t* M4);

    void Set_r1(int iTagNum, uint8_t* r1);
    void Set_r2(int iTagNum, uint8_t* r2);
    void Set_M1(int iTagNum, uint8_t* M1);
    void Set_M2(int iTagNum, uint8_t* M2);
    void Set_new_yi(int iTagNum);

    void Compute_xi(int iTagNum);
    void Compute_yi(int iTagNum);
    void Compute_xiNEW(int iTagNum);
    void Compute_yiNEW(int iTagNum);
    void Compute_M3(int iTagNum);
    void Compute_M4(int iTagNum);


    bool Check_M2(int iTagNum);


private:
    std::vector<stData> vData;
    uint8_t k [NONCE_LEN * 2] = { 0 }; // Secret key of the database
    uint8_t hk[NONCE_LEN * 2] = { 0 }; // h(k)
};

