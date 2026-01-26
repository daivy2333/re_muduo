<pir>
<meta>
name: re_muduo
root: /home/daivy/projects/muduo_learn/re_muduo
profile: generic
lang: CPP,H
</meta>
<units>
u0: CurrentThread.h type=H role=lib module=re_muduo
u1: Socket.cpp type=CPP role=lib module=re_muduo
u2: Acceptor.cpp type=CPP role=lib module=re_muduo
u3: Thread.h type=H role=lib module=re_muduo
u4: EventLoopThread.h type=H role=lib module=re_muduo
u5: TcpConnection.h type=H role=lib module=re_muduo
u6: Socket.h type=H role=lib module=re_muduo
u7: DefaultPoller.cpp type=CPP role=lib module=re_muduo
u8: EventLoopThreadPool.h type=H role=lib module=re_muduo
u9: noncopyable.h type=H role=lib module=re_muduo
u10: TcpServer.h type=H role=lib module=re_muduo
u11: Thread.cpp type=CPP role=lib module=re_muduo
u12: InetAddress.cpp type=CPP role=lib module=re_muduo
u13: Logger.h type=H role=lib module=re_muduo
u14: Buffer.h type=H role=lib module=re_muduo
u15: Channel.h type=H role=lib module=re_muduo
u16: TcpServer.cpp type=CPP role=lib module=re_muduo
u17: Logger.cpp type=CPP role=lib module=re_muduo
u18: Eventloop.h type=H role=lib module=re_muduo
u19: Acceptor.h type=H role=lib module=re_muduo
u20: EpollPoller.h type=H role=lib module=re_muduo
u21: CurrentThread.cpp type=CPP role=lib module=re_muduo
u22: Poller.h type=H role=lib module=re_muduo
u23: Callbacks.h type=H role=lib module=re_muduo
u24: Timestamp.h type=H role=lib module=re_muduo
u25: EpollPoller.cpp type=CPP role=lib module=re_muduo
u26: EventLoopThread.cpp type=CPP role=lib module=re_muduo
u27: Eventloop.cpp type=CPP role=lib module=re_muduo
u28: InetAddress.h type=H role=lib module=re_muduo
u29: Timestamp.cpp type=CPP role=lib module=re_muduo
u30: EventLoopThreadPool.cpp type=CPP role=lib module=re_muduo
u31: Buffer.cpp type=CPP role=lib module=re_muduo
u32: Poller.cpp type=CPP role=lib module=re_muduo
u33: Channel.cpp type=CPP role=lib module=re_muduo
u34: TcpConnection.cpp type=CPP role=lib module=re_muduo
u35: tests/test_buffer.cpp type=CPP role=lib module=tests
u36: tests/test_eventloopthreadpool.cpp type=CPP role=lib module=tests
u37: tests/test_tcpconnection.cpp type=CPP role=lib module=tests
u38: tests/test_inetaddress.cpp type=CPP role=lib module=tests
u39: tests/test_channel.cpp type=CPP role=lib module=tests
u40: tests/test_logger.cpp type=CPP role=lib module=tests
u41: tests/test_timestamp.cpp type=CPP role=lib module=tests
u42: tests/test_eventloop.cpp type=CPP role=lib module=tests
u43: tests/test_thread.cpp type=CPP role=lib module=tests
u44: tests/test_currentthread.cpp type=CPP role=lib module=tests
u45: tests/test_eventloopthread.cpp type=CPP role=lib module=tests
u46: tests/run_all_tests.cpp type=CPP role=lib module=tests
u47: tests/test_poller.cpp type=CPP role=lib module=tests
u48: tests/test_tcpserver.cpp type=CPP role=lib module=tests
u49: tests/test_socket.cpp type=CPP role=lib module=tests
u50: tests/test_acceptor.cpp type=CPP role=lib module=tests
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
d16: include:[Timestamp.h]
d17: include:[algorithm]
d18: include:[arpa/inet.h]
d19: include:[atomic]
d20: include:[cassert]
d21: include:[cerrno]
d22: include:[chrono]
d23: include:[condition_variable]
d24: include:[cstdio]
d25: include:[cstdlib]
d26: include:[cstring]
d27: include:[functional]
d28: include:[iostream]
d29: include:[memory]
d30: include:[mutex]
d31: include:[netinet/in.h]
d32: include:[netinet/tcp.h]
d33: include:[noncopyable.h]
d34: include:[sstream]
d35: include:[stdlib:c]
d36: include:[stdlib:py]
d37: include:[strings.h]
d38: include:[sys/epoll.h]
d39: include:[sys/eventfd.h]
d40: include:[sys/socket.h]
d41: include:[sys/syscall.h]
d42: include:[sys/types.h]
d43: include:[sys/uio.h]
d44: include:[thread]
d45: include:[unordered_map]
d46: include:[vector]
</dependency-pool>
<dependencies>
u1->refs:[d42 d9 d40 d32 d12 d10 d35]
u2->refs:[d8 d42 d9 d40 d0 d10 d35]
u3->refs:[d19 d44 d29 d27 d36 d33 d35]
u4->refs:[d8 d27 d15 d30 d33 d23]
u5->refs:[d29 d1 d8 d9 d36 d2 d33]
u6->refs:[d33]
u7->refs:[d11 d5 d35]
u8->refs:[d29 d8 d46 d27 d36 d33]
u10->refs:[d19 d8 d9 d45 d27 d0 d7 d36 d33 d2]
u11->refs:[d41 d42 d15 d34 d24]
u12->refs:[d9 d37 d26 d28]
u13->refs:[d28 d36 d33]
u14->refs:[d26 d36 d17 d46]
u15->refs:[d33 d27 d16 d29]
u16->refs:[d14 d8 d13 d27 d0 d12 d35]
u17->refs:[d10]
u18->refs:[d19 d3 d29 d46 d33 d27 d30 d16 d4]
u19->refs:[d12 d27 d3 d33]
u20->refs:[d11 d46 d38]
u21->refs:[d41 d4 d35]
u22->refs:[d33 d16 d46 d45]
u23->refs:[d27 d29]
u24->refs:[d28 d36]
u25->refs:[d3 d8 d5 d10 d35]
u26->refs:[d8 d6]
u27->refs:[d39 d8 d17 d20 d10 d11 d35]
u28->refs:[d18 d31 d36 d37]
u29->refs:[d16 d36]
u30->refs:[d7 d35 d6]
u31->refs:[d1 d43 d35]
u32->refs:[d8 d11]
u33->refs:[d38 d3 d8 d28]
u34->refs:[d3 d1 d8 d13 d27 d12 d35]
u35->refs:[d1 d36 d28 d20]
u36->refs:[d19 d44 d8 d22 d7 d20 d28]
u37->refs:[d18 d8 d13 d9 d40 d28 d16 d36 d35]
u38->refs:[d9 d28 d20]
u39->refs:[d3 d39 d44 d8 d21 d20 d28 d10 d35]
u40->refs:[d28 d10 d20]
u41->refs:[d28 d16 d20]
u42->refs:[d3 d44 d39 d8 d20 d28 d10 d35]
u43->refs:[d19 d46 d15 d22 d20 d28 d4]
u44->refs:[d44 d46 d20 d28 d4]
u45->refs:[d19 d8 d22 d20 d28 d6]
u46->refs:[d28 d25 d36]
u47->refs:[d3 d44 d39 d8 d5 d20 d28 d10 d11 d35]
u48->refs:[d44 d14 d8 d9 d13 d22 d20 d28 d16 d36]
u49->refs:[d18 d37 d31 d9 d40 d32 d20 d28 d12 d35]
u50->refs:[d18 d44 d8 d9 d40 d0 d20 d28 d10 d35]
</dependencies>
<symbols>
tid:u0 func
createNonblockingOrDie:u2 func
numCreated:u3 func
setConnectionCallback:u5 func
setMessageCallback:u5 func
setWriteCompleteCallback:u5 func
setCloseCallback:u5 func
setHighWaterMarkCallback:u5 func
setState:u5 func
Socket:u6 func
setThreadNUm:u8 func
setThreadInitCallback:u10 func
setConnectionCallback:u10 func
setMessageCallback:u10 func
setWriteCompleteCallback:u10 func
gettid:u11 func
main:u12 func entry=true
Buffer:u14 func
retrieve:u14 func
retrieveUntil:u14 func
retrieveAll:u14 func
append:u14 func
append:u14 func
append:u14 func
prepend:u14 func
shrink:u14 func
makeSpace:u14 func
ensureWritableBytes:u14 func
setReadCallback:u15 func
setWriteCallback:u15 func
setCloseCallback:u15 func
setErrorCallback:u15 func
setRevents:u15 func
index:u15 func
setIndex:u15 func
enableReading:u15 func
disableReading:u15 func
enableWriting:u15 func
disableWriting:u15 func
disableAll:u15 func
setNewConnectionCallback:u19 func
cacheTid:u21 func
createEventfd:u27 func
InetAddress:u28 func
setSockAddr:u28 func
main:u29 func entry=true
testBufferAppendRetrieve:u35 func
testBufferGrow:u35 func
testBufferInsideGrow:u35 func
testBufferShrink:u35 func
testBufferPrepend:u35 func
testBufferReadInt:u35 func
testBufferFindCRLF:u35 func
main:u35 func entry=true
test_basic_creation:u36 func
test_get_all_loops:u36 func
test_get_next_loop:u36 func
test_init_callback:u36 func
test_run_in_multiple_loops:u36 func
test_concurrent_get_loop:u36 func
main:u36 func entry=true
onConnection:u37 func
onMessage:u37 func
onWriteComplete:u37 func
createSocketPair:u37 func
main:u37 func entry=true
test_inetaddress_with_port:u38 func
test_inetaddress_with_ip_and_port:u38 func
test_inetaddress_get_sockaddr:u38 func
main:u38 func entry=true
test_channel_creation:u39 func
test_channel_events:u39 func
test_channel_callbacks:u39 func
main:u39 func entry=true
test_logger_instance:u40 func
test_logger_log_levels:u40 func
test_logger_set_log_level:u40 func
main:u40 func entry=true
test_timestamp_default_constructor:u41 func
test_timestamp_constructor_with_microseconds:u41 func
test_timestamp_now:u41 func
test_timestamp_to_string:u41 func
main:u41 func entry=true
test_eventloop_basic:u42 func
test_eventloop_channel:u42 func
test_eventloop_runinloop:u42 func
main:u42 func entry=true
test_thread_creation:u43 func
thread:u43 func
test_thread_id:u43 func
thread:u43 func
test_default_naming:u43 func
test_concurrent_threads:u43 func
test_thread_count:u43 func
t1:u43 func
t2:u43 func
test_multiple_join:u43 func
thread:u43 func
main:u43 func entry=true
test_current_thread_tid:u44 func
test_multiple_threads:u44 func
main:u44 func entry=true
test_basic_creation:u45 func
test_thread_naming:u45 func
test_init_callback:u45 func
test_run_in_loop:u45 func
test_multiple_creation:u45 func
test_concurrent_loops:u45 func
main:u45 func entry=true
main:u46 func entry=true
test_poller_creation:u47 func
test_epollpoller_creation:u47 func
test_epollpoller_channel_operations:u47 func
test_epollpoller_poll:u47 func
main:u47 func entry=true
test_basic_creation:u48 func
t:u48 func
test_multithread:u48 func
t:u48 func
test_connection_management:u48 func
t:u48 func
test_thread_init_callback:u48 func
t:u48 func
main:u48 func entry=true
test_socket_bind:u49 func
test_socket_listen:u49 func
test_socket_accept:u49 func
test_socket_tcp_nodelay:u49 func
test_socket_reuse_addr:u49 func
test_socket_reuse_port:u49 func
test_socket_keep_alive:u49 func
test_socket_shutdown_write:u49 func
main:u49 func entry=true
onNewConnection:u50 func
test_acceptor_creation:u50 func
test_acceptor_listen:u50 func
test_acceptor_multiple_connections:u50 func
main:u50 func entry=true
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