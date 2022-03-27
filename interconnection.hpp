#ifndef INTERCONNECTION_H_
#define INTERCONNECTION_H_

enum interconnection : unsigned char {
    A_READY = 1u,
    B_READY = 1u << 1,
    C_READY = 1u << 2,
    ALL_READY = A_READY | B_READY | C_READY,
    A_CLOSED = 1u << 3,
    B_CLOSED = 1u << 4,
    C_CLOSED = 1u << 5,
    SOMEONE_CLOSED = A_CLOSED | B_CLOSED | C_CLOSED,
};
struct memory {
    char line[ 1024 ];
    unsigned char status;
    int AtoB;
    int CtoB;
};



#endif // INTERCONNECTION_H_
