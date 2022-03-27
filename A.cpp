#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <errno.h>

#include "interconnection.hpp"
//1024 byte - string; 1 byte - A status or not; 4 byte - B from A; 4 byte - C from A

int main() {
    int fd = open( "./.file", O_RDWR | O_CREAT | O_TRUNC , S_IRWXU );
    write( fd, "", sizeof( "" ) );
    memory* mem = ( memory* )mmap( 0, sizeof( memory ), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if( !mem ) return 2;
    for( int i = 0; i < 1024; ++i ) {
        mem->line[ i ] = 0;
    }
    mem->status |= A_READY | A_TURN;
    mem->AtoB = 0;
    mem->CtoB = 0;

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
        while( !( mem->status & A_TURN ) ) {
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
        char ch;
        int count = 0;
        std::putc( '>', stdout );
        while( ( ( ch = std::getc( stdin ) ) != '\n' ) && ( ch != EOF ) && ( count < 1024 ) ) {
            mem->line[ count++ ] = ch;
        }
        if( ch == EOF ) break;
        mem->line[ count ] = '\0';
        mem->AtoB = count + 1; //all chars = count + \0
        mem->status &= ~A_TURN;
        mem->status |= C_TURN;
        if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
            std::cerr << errno << '\n';
            return errno;
        }
    }
    mem->status |= A_CLOSED;
    if( msync( mem, sizeof( memory ), MS_INVALIDATE ) ) {
        std::cerr << errno << '\n';
        return errno;
    }
    munmap( &mem, sizeof( memory ) );
    close( fd );
    return 0;
}
