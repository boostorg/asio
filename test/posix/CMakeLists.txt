foreach(test
    posix_basic_descriptor
    posix_basic_stream_descriptor
    posix_descriptor_base
    posix_stream_descriptor
    posix_stream_descriptor_service
    )
  boost_test_link(${test})
endforeach()


