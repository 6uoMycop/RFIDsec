#define NONCE_LEN 1 // 16 * 8 = 128 bit

#define NODES_QUANTITY 1

#define ADDR_READER    -1

//
// Choose if adversary acts
//
//#undef ADVERSARY
#define ADVERSARY

#ifdef ADVERSARY
#define ADDR_ADVERSARY NODES_QUANTITY
#endif