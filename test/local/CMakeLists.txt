foreach(test
    basic_endpoint
    connect_pair
    datagram_protocol
    stream_protocol
    )
  asio_test_link(${test})
endforeach()


