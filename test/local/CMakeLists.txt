foreach(test
    local_basic_endpoint
    local_connect_pair
    local_datagram_protocol
    local_stream_protocol
    )
  boost_test_link(${test})
endforeach()


