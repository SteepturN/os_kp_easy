#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>


#include "interconnection.hpp"
//1024 byte - string; 1 byte - A wait or not; 4 byte - B from A; 4 byte - C from A
int main() {
    int fd = open( "./.file", O_RDWR /* | O_CREAT | O_TRUNC  */, S_IRWXU );
    write( fd, "", sizeof( "" ) );
    memory* mem = reinterpret_cast< memory* >( mmap( 0, sizeof( memory ), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 ) );
    close( fd );
    if( !mem ) return 2;
    mem->status = mem->status | B_READY;

    if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
        std::cerr << errno << '\n';
        return errno;
    }
    while( ( mem->status & ALL_READY ) != ALL_READY ) {
        if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
            std::cerr << errno << '\n';
            return errno;
        }
    }
    bool all_good = true;
    while( true ) {
        while( !( mem->status & B_TURN ) ) {
            if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
                std::cerr << errno << '\n';
                return errno;
            }
            if( mem->status & SOMEONE_CLOSED ) {
                all_good = false;
                break;
            }
        }
        if( !all_good ) break;
        std::cout << "B: A sent " << mem->AtoB << " signs\n";
        mem->AtoB = 0;
        std::cout << "B: C got " << mem->CtoB << " signs\n";
        mem->CtoB = 0;

        mem->status &= ~B_TURN;
        mem->status |= A_TURN;
        if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
            std::cerr << errno << '\n';
            return errno;
        }
    }
    mem->status |= B_CLOSED;
    if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
        std::cerr << errno << '\n';
        return errno;
    }
    munmap( &mem, sizeof( memory ) );
    return 0;
}
