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
    write( fd, "\0", sizeof( "\0" ) );
    memory* mem = reinterpret_cast< memory* >( mmap( 0, sizeof( memory ), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 ) );
    if( !mem ) return 2;
    mem->status = mem->status | B_READY;
    if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
        std::cerr << errno << '\n';
        return errno;
    }
    while( true/* ( mem->status & ALL_READY ) != ALL_READY  */) {
        if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
            std::cerr << errno << '\n';
            return errno;
        }
    }
    while( true ) {
        if( mem->status & SOMEONE_CLOSED ) {
            break;
        }
        while( !mem->AtoB );
        std::cout << "B: A sent " << mem->AtoB << " signs\n";
        mem->AtoB = 0;
        while( !mem->CtoB );
        std::cout << "B: C got " << mem->CtoB << " signs\n";
        mem->CtoB = 0;
    }
    mem->status |= B_CLOSED;
    close( fd );
    return 0;
}
