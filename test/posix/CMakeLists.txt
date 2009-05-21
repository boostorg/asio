foreach(test
    basic_descriptor
    basic_stream_descriptor
    descriptor_base
    stream_descriptor
    stream_descriptor_service
    )
  asio_test_link(${test})
endforeach()


