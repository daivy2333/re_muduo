<pir>
<meta>
name: re_muduo
root: /home/daivy/projects/muduo_learn/re_muduo
profile: generic
lang: CPP,H
</meta>
<units>
u0: tests/test_buffer.cpp type=CPP role=lib module=tests
u1: tests/test_eventloopthreadpool.cpp type=CPP role=lib module=tests
u2: tests/test_tcpconnection.cpp type=CPP role=lib module=tests
u3: tests/test_timer.cpp type=CPP role=lib module=tests
u4: tests/test_inetaddress.cpp type=CPP role=lib module=tests
u5: tests/test_channel.cpp type=CPP role=lib module=tests
u6: tests/test_logger.cpp type=CPP role=lib module=tests
u7: tests/test_timestamp.cpp type=CPP role=lib module=tests
u8: tests/test_eventloop.cpp type=CPP role=lib module=tests
u9: tests/test_thread.cpp type=CPP role=lib module=tests
u10: tests/test_currentthread.cpp type=CPP role=lib module=tests
u11: tests/test_eventloopthread.cpp type=CPP role=lib module=tests
u12: tests/run_all_tests.cpp type=CPP role=lib module=tests
u13: tests/test_poller.cpp type=CPP role=lib module=tests
u14: tests/test_tcpserver.cpp type=CPP role=lib module=tests
u15: tests/test_socket.cpp type=CPP role=lib module=tests
u16: tests/test_acceptor.cpp type=CPP role=lib module=tests
u17: include/CurrentThread.h type=H role=lib module=include
u18: include/Thread.h type=H role=lib module=include
u19: include/EventLoopThread.h type=H role=lib module=include
u20: include/TcpConnection.h type=H role=lib module=include
u21: include/Socket.h type=H role=lib module=include
u22: include/EventLoopThreadPool.h type=H role=lib module=include
u23: include/noncopyable.h type=H role=lib module=include
u24: include/TcpServer.h type=H role=lib module=include
u25: include/Logger.h type=H role=lib module=include
u26: include/Buffer.h type=H role=lib module=include
u27: include/Channel.h type=H role=lib module=include
u28: include/Eventloop.h type=H role=lib module=include
u29: include/Acceptor.h type=H role=lib module=include
u30: include/EpollPoller.h type=H role=lib module=include
u31: include/Timer.h type=H role=lib module=include
u32: include/Poller.h type=H role=lib module=include
u33: include/Callbacks.h type=H role=lib module=include
u34: include/Timestamp.h type=H role=lib module=include
u35: include/TimerQueue.h type=H role=lib module=include
u36: include/InetAddress.h type=H role=lib module=include
u37: benchmark/echo_server_bench.cpp type=CPP role=lib module=benchmark
u38: benchmark/benchmark_base.cpp type=CPP role=lib module=benchmark
u39: benchmark/latency_test.cpp type=CPP role=lib module=benchmark
u40: benchmark/self_stress_test.cpp type=CPP role=lib module=benchmark
u41: benchmark/benchmark_base.h type=H role=lib module=benchmark
u42: benchmark/echo_server_bench.h type=H role=lib module=benchmark
u43: benchmark/throughput_test.cpp type=CPP role=lib module=benchmark
u44: src/base/Timer.cpp type=CPP role=lib module=base
u45: src/base/Socket.cpp type=CPP role=lib module=base
u46: src/base/Acceptor.cpp type=CPP role=lib module=base
u47: src/base/DefaultPoller.cpp type=CPP role=lib module=base
u48: src/base/Thread.cpp type=CPP role=lib module=base
u49: src/base/InetAddress.cpp type=CPP role=lib module=base
u50: src/base/TcpServer.cpp type=CPP role=lib module=base
u51: src/base/Logger.cpp type=CPP role=lib module=base
u52: src/base/CurrentThread.cpp type=CPP role=lib module=base
u53: src/base/EpollPoller.cpp type=CPP role=lib module=base
u54: src/base/EventLoopThread.cpp type=CPP role=lib module=base
u55: src/base/Eventloop.cpp type=CPP role=lib module=base
u56: src/base/Timestamp.cpp type=CPP role=lib module=base
u57: src/base/TimerQueue.cpp type=CPP role=lib module=base
u58: src/base/EventLoopThreadPool.cpp type=CPP role=lib module=base
u59: src/base/Buffer.cpp type=CPP role=lib module=base
u60: src/base/Poller.cpp type=CPP role=lib module=base
u61: src/base/Channel.cpp type=CPP role=lib module=base
u62: src/base/TcpConnection.cpp type=CPP role=lib module=base
</units>
<dependency-pool>
d0: include:[Acceptor.h]
d1: include:[Buffer.h]
d2: include:[Callbacks.h]
d3: include:[Channel.h]
d4: include:[CurrentThread.h]
d5: include:[EpollPoller.h]
d6: include:[EventLoopThread.h]
d7: include:[EventLoopThreadPool.h]
d8: include:[Eventloop.h]
d9: include:[InetAddress.h]
d10: include:[Logger.h]
d11: include:[Poller.h]
d12: include:[Socket.h]
d13: include:[TcpConnection.h]
d14: include:[TcpServer.h]
d15: include:[Thread.h]
d16: include:[Timer.h]
d17: include:[TimerQueue.h]
d18: include:[Timestamp.h]
d19: include:[algorithm]
d20: include:[arpa/inet.h]
d21: include:[atomic]
d22: include:[benchmark_base.h]
d23: include:[cassert]
d24: include:[cerrno]
d25: include:[chrono]
d26: include:[condition_variable]
d27: include:[cstdio]
d28: include:[cstdlib]
d29: include:[cstring]
d30: include:[echo_server_bench.h]
d31: include:[fstream]
d32: include:[functional]
d33: include:[iomanip]
d34: include:[iostream]
d35: include:[memory]
d36: include:[mutex]
d37: include:[netinet/in.h]
d38: include:[netinet/tcp.h]
d39: include:[noncopyable.h]
d40: include:[set]
d41: include:[sstream]
d42: include:[stdlib:c]
d43: include:[stdlib:py]
d44: include:[strings.h]
d45: include:[sys/epoll.h]
d46: include:[sys/eventfd.h]
d47: include:[sys/socket.h]
d48: include:[sys/syscall.h]
d49: include:[sys/time.h]
d50: include:[sys/timerfd.h]
d51: include:[sys/types.h]
d52: include:[sys/uio.h]
d53: include:[thread]
d54: include:[unordered_map]
d55: include:[vector]
</dependency-pool>
<dependencies>
u0->refs:[d34 d23 d43 d1]
u1->refs:[d21 d25 d8 d7 d23 d34 d53]
u2->refs:[d18 d42 d8 d13 d20 d9 d43 d34 d47]
u3->refs:[d21 d16 d8 d10 d34 d53]
u4->refs:[d34 d23 d9]
u5->refs:[d46 d8 d42 d10 d24 d23 d34 d53 d3]
u6->refs:[d10 d34 d23]
u7->refs:[d34 d18 d23]
u8->refs:[d42 d8 d46 d10 d23 d34 d53 d3]
u9->refs:[d4 d21 d25 d15 d55 d23 d34]
u10->refs:[d4 d55 d23 d34 d53]
u11->refs:[d21 d25 d8 d6 d23 d34]
u12->refs:[d34 d28 d43]
u13->refs:[d42 d8 d46 d10 d11 d5 d23 d34 d53 d3]
u14->refs:[d18 d25 d8 d13 d9 d43 d23 d14 d34 d53]
u15->refs:[d42 d20 d9 d38 d37 d23 d44 d34 d47 d12]
u16->refs:[d0 d8 d42 d10 d20 d9 d23 d34 d53 d47]
u18->refs:[d21 d35 d42 d32 d39 d43 d53]
u19->refs:[d36 d8 d32 d39 d15 d26]
u20->refs:[d2 d35 d8 d39 d9 d43 d1]
u21->refs:[d39]
u22->refs:[d35 d8 d32 d39 d55 d43]
u24->refs:[d2 d21 d0 d8 d54 d32 d39 d7 d9 d43]
u25->refs:[d34 d39 d43]
u26->refs:[d19 d55 d29 d43]
u27->refs:[d18 d39 d35 d32]
u28->refs:[d4 d2 d18 d36 d21 d35 d42 d32 d39 d55 d3]
u29->refs:[d32 d39 d3 d12]
u30->refs:[d55 d45 d11]
u31->refs:[d2 d18 d21]
u32->refs:[d39 d55 d54 d18]
u33->refs:[d35 d32]
u34->refs:[d34 d43]
u35->refs:[d35 d16 d55 d39 d40 d3]
u36->refs:[d44 d37 d43 d20]
u37->refs:[d18 d29 d25 d42 d13 d30 d20 d37 d1 d34 d47]
u38->refs:[d22 d19 d33 d34]
u39->refs:[d29 d42 d8 d35 d13 d25 d55 d20 d31 d37 d33 d14 d1 d22 d34 d53 d47]
u40->refs:[d21 d29 d13 d10 d31 d34 d53 d47 d35 d16 d1 d18 d25 d8 d42 d55 d9 d37 d33 d14 d20]
u41->refs:[d18 d36 d21 d32 d55]
u42->refs:[d21 d35 d8 d14 d22 d53]
u43->refs:[d30 d31 d33 d22 d34]
u44->refs:[d16]
u45->refs:[d51 d42 d10 d9 d38 d47 d12]
u46->refs:[d51 d42 d0 d8 d10 d9 d47]
u47->refs:[d42 d11 d5]
u48->refs:[d51 d15 d48 d27 d41]
u49->refs:[d44 d34 d29 d9]
u50->refs:[d0 d8 d42 d13 d32 d14 d12]
u51->refs:[d10]
u52->refs:[d4 d48 d42]
u53->refs:[d42 d8 d10 d5 d3]
u54->refs:[d6 d8]
u55->refs:[d46 d8 d42 d17 d10 d19 d11 d23]
u56->refs:[d49 d18 d43]
u57->refs:[d42 d8 d17 d10 d19 d50]
u58->refs:[d7 d6 d42]
u59->refs:[d42 d1 d52]
u60->refs:[d11 d8]
u61->refs:[d34 d45 d3 d8]
u62->refs:[d42 d8 d13 d32 d1 d3 d12]
</dependencies>
<symbols>
testBufferAppendRetrieve:u0 func
testBufferGrow:u0 func
testBufferInsideGrow:u0 func
testBufferShrink:u0 func
testBufferPrepend:u0 func
testBufferReadInt:u0 func
testBufferFindCRLF:u0 func
main:u0 func entry=true
test_basic_creation:u1 func
test_get_all_loops:u1 func
test_get_next_loop:u1 func
test_init_callback:u1 func
test_run_in_multiple_loops:u1 func
test_concurrent_get_loop:u1 func
main:u1 func entry=true
onConnection:u2 func
onMessage:u2 func
onWriteComplete:u2 func
createSocketPair:u2 func
main:u2 func entry=true
test_basic_timer:u3 func
test_timer_cancel:u3 func
main:u3 func entry=true
test_inetaddress_with_port:u4 func
test_inetaddress_with_ip_and_port:u4 func
test_inetaddress_get_sockaddr:u4 func
main:u4 func entry=true
test_channel_creation:u5 func
test_channel_events:u5 func
test_channel_callbacks:u5 func
main:u5 func entry=true
test_logger_instance:u6 func
test_logger_log_levels:u6 func
test_logger_set_log_level:u6 func
main:u6 func entry=true
test_timestamp_default_constructor:u7 func
test_timestamp_constructor_with_microseconds:u7 func
test_timestamp_now:u7 func
test_timestamp_to_string:u7 func
main:u7 func entry=true
test_eventloop_basic:u8 func
test_eventloop_channel:u8 func
test_eventloop_runinloop:u8 func
main:u8 func entry=true
test_thread_creation:u9 func
thread:u9 func
test_thread_id:u9 func
thread:u9 func
test_default_naming:u9 func
test_concurrent_threads:u9 func
test_thread_count:u9 func
t1:u9 func
t2:u9 func
test_multiple_join:u9 func
thread:u9 func
main:u9 func entry=true
test_current_thread_tid:u10 func
test_multiple_threads:u10 func
main:u10 func entry=true
test_basic_creation:u11 func
test_thread_naming:u11 func
test_init_callback:u11 func
test_run_in_loop:u11 func
test_multiple_creation:u11 func
test_concurrent_loops:u11 func
main:u11 func entry=true
main:u12 func entry=true
test_poller_creation:u13 func
test_epollpoller_creation:u13 func
test_epollpoller_channel_operations:u13 func
test_epollpoller_poll:u13 func
main:u13 func entry=true
test_basic_creation:u14 func
t:u14 func
test_multithread:u14 func
t:u14 func
test_connection_management:u14 func
t:u14 func
test_thread_init_callback:u14 func
t:u14 func
main:u14 func entry=true
test_socket_bind:u15 func
test_socket_listen:u15 func
test_socket_accept:u15 func
test_socket_tcp_nodelay:u15 func
test_socket_reuse_addr:u15 func
test_socket_reuse_port:u15 func
test_socket_keep_alive:u15 func
test_socket_shutdown_write:u15 func
main:u15 func entry=true
onNewConnection:u16 func
test_acceptor_creation:u16 func
test_acceptor_listen:u16 func
test_acceptor_multiple_connections:u16 func
main:u16 func entry=true
tid:u17 func
numCreated:u18 func
setConnectionCallback:u20 func
setMessageCallback:u20 func
setWriteCompleteCallback:u20 func
setCloseCallback:u20 func
setHighWaterMarkCallback:u20 func
setState:u20 func
Socket:u21 func
setThreadNUm:u22 func
setThreadInitCallback:u24 func
setConnectionCallback:u24 func
setMessageCallback:u24 func
setWriteCompleteCallback:u24 func
Buffer:u26 func
retrieve:u26 func
retrieveUntil:u26 func
retrieveAll:u26 func
append:u26 func
append:u26 func
append:u26 func
prepend:u26 func
shrink:u26 func
makeSpace:u26 func
ensureWritableBytes:u26 func
setReadCallback:u27 func
setWriteCallback:u27 func
setCloseCallback:u27 func
setErrorCallback:u27 func
setRevents:u27 func
index:u27 func
setIndex:u27 func
enableReading:u27 func
disableReading:u27 func
enableWriting:u27 func
disableWriting:u27 func
disableAll:u27 func
assertInLoopThread:u28 func
setNewConnectionCallback:u29 func
numCreated:u31 func
addTime:u34 func
timeDifference:u34 func
InetAddress:u36 func
setSockAddr:u36 func
teardown:u39 func
run_latency_tests:u39 func
main:u39 func entry=true
run:u40 func
runServer:u40 func
runClients:u40 func
main:u40 func entry=true
run_throughput_benchmark:u43 func
main:u43 func entry=true
createNonblockingOrDie:u46 func
gettid:u48 func
main:u49 func entry=true
cacheTid:u52 func
createEventfd:u55 func
main:u56 func entry=true
createTimerfd:u57 func
howMuchTimeFromNow:u57 func
resetTimerfd:u57 func
readTimerfd:u57 func
</symbols>
<profiles>
  active: system-c
  c-framework:
    confidence: 0.5
    tags:
      - domain:language-tooling
      - runtime:native
      - stack:c-framework
  system-c:
    confidence: 0.55
    tags:
      - domain:system
      - lang:c
      - runtime:native
    signals:
      - multi-unit
</profiles>
</pir>