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
//1024 byte - string; 1 byte - A wait or not; 4 byte - B from A; 4 byte - C from A


#include "interconnection.hpp"
int main() {
    int fd = open( "./.file", O_RDWR /* | O_CREAT | O_TRUNC */, S_IRWXU );
    write( fd, "", sizeof( "" ) );
    memory* mem = reinterpret_cast< memory* >( mmap( 0, sizeof( memory ), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 ) );
    close( fd );
    if( !mem ) return 2;
    for( int i = 0; i < sizeof( memory ); ++i ) {
        ( int )( ( ( char* )mem )[ i ] );
    }
    mem->status = mem->status | C_READY;
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
        while( !( mem->status & C_TURN ) ) {
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
        std::cout << "C: " << mem->line << '\n';
        int count = 0;
        while( mem->line[ count++ ] && ( count < 1024 ) );
        mem->CtoB = count;

        mem->status &= ~C_TURN;
        mem->status |= B_TURN;
        if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
            std::cerr << errno << '\n';
            return errno;
        }
    }
    mem->status |= C_CLOSED;
    if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
        std::cerr << errno << '\n';
        return errno;
    }
    munmap( &mem, sizeof( memory ) );
    return 0;
}
