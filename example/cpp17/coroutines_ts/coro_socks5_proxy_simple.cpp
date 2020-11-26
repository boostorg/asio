//
// coro_socks5_proxy_simple.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// gamefunc/32686647
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// 
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <cstdio>
#include <string>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

static bool print_request_bytes = false;
static bool print_response_bytes = false;


void print_bytes(uint8_t *str, uint64_t str_len){
    for(uint64_t i = 0; i < str_len; i++){
        if (str[i] == 9) {
            std::printf("\\t");
        }else if(str[i] == 10){
            std::printf("\\n");
        }else if(str[i] == 13){
            std::printf("\\r");
        }else if (str[i] < 32 || str[i] > 126){
            std::printf("\\x%02x", str[i]);
        }else{
            std::printf("%c", str[i]);
        }
    }
    std::printf("\n\n");
}

awaitable<void> dst_trans(tcp::socket &front_s, tcp::socket &back_s){
    // read destination data -> broswer;
    uint8_t recv_buff[4096];
    uint64_t recv_len;
    try{
        for(;;){
            recv_len = co_await back_s.async_read_some(
                boost::asio::buffer(recv_buff, 4096), use_awaitable
            );
            if (print_response_bytes){ print_bytes(recv_buff, recv_len); }
            co_await front_s.async_write_some(
                boost::asio::buffer(recv_buff, recv_len), use_awaitable
            );
        }
    }catch (std::exception& e){ front_s.close(); back_s.close(); }

}

awaitable<void> local_trans(tcp::socket front_s){
    // socks5 handshake; then read broswer data -> destination ;
    auto loop = co_await this_coro::executor;
    tcp::socket back_s(loop);
    tcp::endpoint back_ep;
    uint8_t state = 0;
    uint64_t recv_len;
    uint8_t recv_buff[1024];
    uint8_t send_buff[10];
    try{
        for(;;){
            recv_len = co_await front_s.async_read_some(
                boost::asio::buffer(recv_buff, 1024), use_awaitable);
            if (state == 0 && recv_buff[0] == 5 && recv_buff[1] == 1){
                send_buff[0] = 5;
                send_buff[1] = 0;
                state = 1;
                co_await front_s.async_write_some(
                    boost::asio::buffer(send_buff, 2), use_awaitable);
            }else if(state == 1){
                uint8_t atype = (uint8_t)recv_buff[3];
                uint8_t domainLen = (uint8_t)recv_buff[4];
                std::string dst_host;
                dst_host.reserve(15);
                uint16_t dst_port;
                uint8_t *pp = (uint8_t*)&dst_port;
                if(atype == 1){ // ipv4;
                    for(uint8_t i = 0; i < 4; i++){
                        dst_host += std::to_string(recv_buff[i+4]);
                        if (i != 3){ dst_host += '.'; } }
                    pp[0] = recv_buff[9];
                    pp[1] = recv_buff[8];
                    back_ep = tcp::endpoint(
                        boost::asio::ip::make_address(dst_host), dst_port);
                }else if(atype == 3){ // domain.com
                    dst_host.append((char*)&recv_buff[5], domainLen);
                    pp[0] = recv_buff[recv_len-1];
                    pp[1] = recv_buff[recv_len-2];
                    tcp::resolver resolver(loop);
                    tcp::resolver::iterator back_ep_iter;
                    back_ep_iter = co_await resolver.async_resolve(
                        {dst_host, std::to_string(dst_port)}, use_awaitable);
                    back_ep = *back_ep_iter;
                }else{
                    printf("other lazy@@\n");
                    front_s.close();
                }
                co_await back_s.async_connect(back_ep, use_awaitable);

                // create a new task;
                co_spawn(loop, dst_trans(front_s, back_s), detached);

                // reply_msg;
                send_buff[0] = 5;
                send_buff[1] = 0;
                send_buff[2] = 0;
                send_buff[3] = 1;
                state = 2;
                co_await front_s.async_write_some(
                    boost::asio::buffer(send_buff, 10), use_awaitable);
            }else if(state == 2){
                if (print_request_bytes){ print_bytes(recv_buff, recv_len); }
                co_await back_s.async_write_some(
                    boost::asio::buffer(recv_buff, recv_len), use_awaitable);
            }
        }
    }catch (std::exception& e){front_s.close(); back_s.close();}
}


awaitable<void> listener(uint16_t listen_port){
    auto loop = co_await this_coro::executor;
    tcp::acceptor acceptor(loop, {tcp::v4(), listen_port});
    for(;;){
        tcp::socket socket = co_await acceptor.async_accept(
            use_awaitable);
        co_spawn(loop, local_trans(std::move(socket)), detached);
    }
}


int main(){
    uint16_t listen_port = 7777;
    boost::asio::io_context io_context(1);
    boost::asio::signal_set signals(
        io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });
    co_spawn(io_context, listener(listen_port), detached);
    io_context.run();
}
