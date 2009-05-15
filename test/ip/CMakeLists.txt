foreach(test
    address
    address_v4
    address_v6
    basic_endpoint
    basic_resolver
    basic_resolver_entry
    basic_resolver_iterator
    basic_resolver_query
    resolver_query_base
    resolver_service
    )
  asio_test_link(${test})
endforeach()

foreach(test
    host_name
    multicast
    tcp
    udp
    unicast
    v6_only)
  asio_test_run(${test})
endforeach()


