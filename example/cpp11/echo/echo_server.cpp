///
/// Copyright (C) 2016 Xiaoshuang LU (luxiaoshuang at qiyi dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///

# include <cstdlib>
# include <cstring>

# include <chrono>
# include <iostream>
# include <memory>
# include <string>
# include <thread>

# include <boost/asio.hpp>
# include <boost/lockfree/queue.hpp>

# include "byte_buffer.hpp"

class server_socket
:
public std::enable_shared_from_this<server_socket>
{
    public:
        server_socket(boost::asio::io_service & io_service)
        :
        socket_(io_service),
        write_queue_(1024),
        handle_write_flag_(false)
        {
        }

        boost::asio::ip::tcp::socket & get_socket()
        {
            return this->socket_;
        }

        void read()
        {
            byte_buffer * byte_buffer_pointer = new byte_buffer;

            byte_buffer_pointer->header_length_ = 4;
            byte_buffer_pointer->body_length_ = 0;
            byte_buffer_pointer->data_ = nullptr;

            std::shared_ptr<server_socket> self(shared_from_this());

            boost::asio::async_read
            (
                this->socket_,
                boost::asio::buffer(& (byte_buffer_pointer->body_length_), byte_buffer_pointer->header_length_),
                [this, self, byte_buffer_pointer](boost::system::error_code const & error_code, std::size_t length)
                {
# if defined(DEBUG_MODE)
                    std::cout << "async_read header complete." << std::endl;
# endif
                    if (! error_code)
                    {
                        if (byte_buffer_pointer->header_length_ != length)
                        {
                            delete byte_buffer_pointer;

                            std::cerr << "async_read header failed. Number of bytes read successfully does not equal expectation." << std::endl;

                            boost::system::error_code temporary;

                            this->socket_.close(temporary);
                        }
                        else
                        {
                            this->handle_header_read(byte_buffer_pointer);
                        }
                    }
                    else
                    {
                        delete byte_buffer_pointer;

                        std::cerr << "async_read header failed. (error_code = " << error_code.value() << ", error_message = " << error_code.message() << ", error_category = " << error_code.category().name() << ")" << std::endl;

                        boost::system::error_code temporary;

                        this->socket_.close(temporary);
                    }
                }
            );
        }

        void write(byte_buffer * byte_buffer_pointer)
        {
            this->write_queue_.push(byte_buffer_pointer);

            this->handle_write();
        }

    private:
        void handle_header_read(byte_buffer * byte_buffer_pointer)
        {
            byte_buffer_pointer->data_ = (uint8_t *)malloc(byte_buffer_pointer->header_length_ + byte_buffer_pointer->body_length_);
# if defined(DEBUG_MODE)
            std::cout << "(header_length = " << byte_buffer_pointer->header_length_ << ", body_length = " << byte_buffer_pointer->body_length_ << ", data = " << (uintmax_t)(byte_buffer_pointer->data_) << ")" << std::endl;
# endif
            memcpy(byte_buffer_pointer->data_, & (byte_buffer_pointer->body_length_), byte_buffer_pointer->header_length_);

            std::shared_ptr<server_socket> self(shared_from_this());

            boost::asio::async_read
            (
                this->socket_,
                boost::asio::buffer(byte_buffer_pointer->data_ + byte_buffer_pointer->header_length_, byte_buffer_pointer->body_length_),
                [this, self, byte_buffer_pointer](boost::system::error_code const & error_code, std::size_t length)
                {
# if defined(DEBUG_MODE)
                    std::cout << "async_read body complete." << std::endl;
# endif
                    if (! error_code)
                    {
                        if (byte_buffer_pointer->body_length_ != length)
                        {
                            free(byte_buffer_pointer->data_);
                            delete byte_buffer_pointer;

                            std::cerr << "async_read body failed. Number of bytes read successfully does not equal expectation." << std::endl;

                            boost::system::error_code temporary;

                            this->socket_.close(temporary);
                        }
                        else
                        {
                            this->handle_body_read(byte_buffer_pointer);
                        }
                    }
                    else
                    {
                        free(byte_buffer_pointer->data_);
                        delete byte_buffer_pointer;

                        std::cerr << "async_read body failed. (error_code = " << error_code.value() << ", error_message = " << error_code.message() << ", error_category = " << error_code.category().name() << ")" << std::endl;

                        boost::system::error_code temporary;

                        this->socket_.close(temporary);
                    }
                }
            );
        }

        void handle_body_read(byte_buffer * byte_buffer_pointer)
        {
            std::string string((char *)(byte_buffer_pointer->data_ + byte_buffer_pointer->header_length_), byte_buffer_pointer->body_length_);

            std::cout << "client: " << string << std::endl;

            this->write(byte_buffer_pointer);

            this->read();
        }

        void handle_write()
        {
            if (! this->write_queue_.empty())
            {
                bool expected = false;
                bool desired = true;

                if (!this->handle_write_flag_.compare_exchange_weak(expected, desired))
                {
                    return;
                }

                byte_buffer * byte_buffer_pointer;

                if (this->write_queue_.pop(byte_buffer_pointer))
                {
                    std::shared_ptr<server_socket> self(shared_from_this());

                    boost::asio::async_write
                    (
                        this->socket_,
                        boost::asio::buffer(byte_buffer_pointer->data_, byte_buffer_pointer->header_length_ + byte_buffer_pointer->body_length_),
                        [this, self, byte_buffer_pointer](boost::system::error_code const & error_code, std::size_t length)
                        {
# if defined(DEBUG_MODE)
                            std::cout << "async_write complete." << std::endl;
# endif
                            this->handle_write_flag_.store(false);

                            if (! error_code)
                            {
                                if (byte_buffer_pointer->header_length_ + byte_buffer_pointer->body_length_ != length)
                                {
                                    std::cerr << "async_write failed. Number of bytes written successfully does not equal expectation." << std::endl;

                                    boost::system::error_code temporary;

                                    this->socket_.close(temporary);
                                }
                                else
                                {
                                    this->handle_write();
                                }
                            }
                            else
                            {
                                std::cerr << "async_write failed. (error_code = " << error_code.value() << ", error_message = " << error_code.message() << ", error_category = " << error_code.category().name() << ")" << std::endl;

                                boost::system::error_code temporary;

                                this->socket_.close(temporary);
                            }

                            free(byte_buffer_pointer->data_);
                            delete byte_buffer_pointer;
                        }
                    );
                }
                else
                {
                    std::cerr << "It is impossible to reach here." << std::endl;;

                    this->handle_write_flag_.store(false);
                }
            }
        }

        boost::asio::ip::tcp::socket socket_;
        // high performance concurrent queue is required
        boost::lockfree::queue<byte_buffer *> write_queue_;
        std::atomic<bool> handle_write_flag_;
};

class echo_server
{
    public:
        echo_server(boost::asio::io_service & io_service, short port)
        :
        io_service_(io_service),
        acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        {
            this->do_accept();
        }

    private:
        void do_accept()
        {
            std::shared_ptr<server_socket> serverSocket{ new server_socket(this->io_service_) };

            this->acceptor_.async_accept
            (
                serverSocket->get_socket(),
                [this, serverSocket](boost::system::error_code const & error_code)
                {
# if defined(DEBUG_MODE)
                    std::cout << "async_accept complete." << std::endl;
# endif
                    if (! error_code)
                    {
                        serverSocket->read();

                        this->do_accept();
                    }
                    else
                    {
                        std::cerr << "async_accept failed. (error_code = " << error_code.value() << ", error_message = " << error_code.message() << ", error_category = " << error_code.category().name() << ")" << std::endl;
                    }
                }
            );
        }

        boost::asio::io_service & io_service_;
        boost::asio::ip::tcp::acceptor acceptor_;
};

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "SYNOPSIS: echo_server <port>" << std::endl;

        return -1;
    }

    try
    {
        boost::asio::io_service io_service;

        boost::asio::io_service::work work(io_service);

        echo_server server(io_service, std::atoi(argv[1]));

        int io_service_thread_number = 8;

        std::vector<std::thread *> io_service_thread_vector;

        for (int i = 0; i < io_service_thread_number; ++ i)
        {
            std::thread * thread =
            new std::thread
            (
                [& io_service]
                {
                    io_service.run();
                }
            );

            io_service_thread_vector.push_back(thread);
        }

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::hours(INTMAX_MAX));
        }

        for (int i = 0; i < io_service_thread_number; ++ i)
        {
            io_service_thread_vector[i]->join();
        }
    }
    catch (std::exception & exception)
    {
        std::cerr << "Exception: " << exception.what() << std::endl;
    }

    return 0;
}
