#define NONCE_LEN 16 // 16 * 8 = 128 bit


#define ADDR_READER -1

//
// Choose if adversary acts
//
#undef ADVERSARY
//#define ADVERSARY

#ifdef ADVERSARY
#define NODES_QUANTITY 1 // MUST BE 1
#define ADDR_ADVERSARY NODES_QUANTITY
#else
#define NODES_QUANTITY 10 // any number
#endif