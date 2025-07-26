/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SLORY
#define NODEPP_SLORY
#define PAYLOAD "GET / HTTP/1.1 \r\n Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7 \r\n Accept-Encoding: gzip, deflate \r\n Accept-Language: es-VE,es;q=0.9,en;q=0.8 \r\n Cache-Control: max-age=0 \r\n Connection: keep-alive \r\n Host: www.gstatic.com \r\n If-Modified-Since: Mon, 29 Aug 2022 20:42:49 GMT \r\n If-None-Match: 1cfa9-5e7674e97dfe0-gzip \r\n Upgrade-Insecure-Requests: 1 \r\n User-Agent: Mozilla/5.0 (X11; CrOS x86_64 14541.0.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 \r\n \r\n"

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/limit.h>
#include <nodepp/tcp.h>

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_SLORY_CONFIG
#define NODEPP_SLORY_CONFIG
namespace nodepp { struct slory_config_t {
    string_t host    = "localhost";
    string_t payload = PAYLOAD;
    ulong    delay   = 0 ;
    int      state   = 0 ;
    uint     port    = 80;
};}
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class slory_tcp_t {
protected:

    ptr_t<slory_config_t> obj;

public:

    event_t<>           onOpen ;
    event_t<>           onDrain;
    event_t<>           onClose;
    event_t<except_t>   onError;
    event_t<uint,ulong> onProgress;
    
    /*─······································································─*/

    slory_tcp_t ( slory_config_t args ) noexcept : obj( type::bind(args) ){ obj->state = 1; }

    virtual ~slory_tcp_t () noexcept  { if( obj.count()> 1 ) { return; } free(); }

    slory_tcp_t () noexcept : obj( new slory_config_t() ){}
    
    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state<=0; }

    void free() const noexcept { 
         if( obj->state < 0 ){ return; }
         close(); obj->state = -1; onClose.emit();
    }

    void close() const noexcept {
         if( obj->state <= 0 ){ return; }
         onDrain.emit(); obj->state = 0;
    }

    void pipe() const noexcept { if( is_closed() ){ return; }
    auto self = type::bind( this ); onOpen.emit();

        process::add( coroutine::add( COROUTINE(){
            if( self->is_closed() ){ self->close(); return -1; }
        coBegin ; coWait( !limit::fileno_ready() ); do {

            auto skt = tcp::client(); skt.onConnect.once([=]( socket_t cli ){
            auto time= type::bind( process::now() );
            auto fd  = cli.get_fd();

                cli.onDrain.once([=](){
                    self->onProgress.emit( fd, process::now() -*time );
                }); cli.write( self->obj->payload ); stream::pipe(cli);

            }); 
            
            skt.onError([=]( ... ){ self->close(); });
            skt.connect( self->obj->host, self->obj->port );

        } while(0); coDelay( self->obj->delay ) ; coGoto(0) ; coFinish
        }));
         
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace slory {

    slory_tcp_t tcp( const slory_config_t& args ){ 
    slory_tcp_t pid( args ); pid.pipe(); return pid;
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#undef PAYLOAD
#endif
