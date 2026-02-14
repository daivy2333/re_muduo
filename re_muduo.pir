<pir>
<meta>
name: re_muduo
root: /home/daivy/projects/muduo_learn/re_muduo
profile: generic
lang: CPP,H
</meta>
<units>
u0: tests/test_timer_integration.cpp type=CPP role=lib module=tests
u1: tests/test_buffer.cpp type=CPP role=lib module=tests
u2: tests/test_eventloopthreadpool.cpp type=CPP role=lib module=tests
u3: tests/test_timer_class.cpp type=CPP role=lib module=tests
u4: tests/test_tcpconnection.cpp type=CPP role=lib module=tests
u5: tests/test_timer.cpp type=CPP role=lib module=tests
u6: tests/test_inetaddress.cpp type=CPP role=lib module=tests
u7: tests/test_async_logging.cpp type=CPP role=lib module=tests
u8: tests/test_channel.cpp type=CPP role=lib module=tests
u9: tests/test_logger.cpp type=CPP role=lib module=tests
u10: tests/test_timestamp.cpp type=CPP role=lib module=tests
u11: tests/test_eventloop.cpp type=CPP role=lib module=tests
u12: tests/test_thread.cpp type=CPP role=lib module=tests
u13: tests/test_currentthread.cpp type=CPP role=lib module=tests
u14: tests/test_eventloopthread.cpp type=CPP role=lib module=tests
u15: tests/run_all_tests.cpp type=CPP role=lib module=tests
u16: tests/test_poller.cpp type=CPP role=lib module=tests
u17: tests/test_tcpserver.cpp type=CPP role=lib module=tests
u18: tests/test_timer_queue.cpp type=CPP role=lib module=tests
u19: tests/test_socket.cpp type=CPP role=lib module=tests
u20: tests/test_acceptor.cpp type=CPP role=lib module=tests
u21: include/CurrentThread.h type=H role=lib module=include
u22: include/Thread.h type=H role=lib module=include
u23: include/EventLoopThread.h type=H role=lib module=include
u24: include/TcpConnection.h type=H role=lib module=include
u25: include/Socket.h type=H role=lib module=include
u26: include/EventLoopThreadPool.h type=H role=lib module=include
u27: include/noncopyable.h type=H role=lib module=include
u28: include/TcpServer.h type=H role=lib module=include
u29: include/LogStream.h type=H role=lib module=include
u30: include/Logger.h type=H role=lib module=include
u31: include/Buffer.h type=H role=lib module=include
u32: include/Channel.h type=H role=lib module=include
u33: include/Eventloop.h type=H role=lib module=include
u34: include/Acceptor.h type=H role=lib module=include
u35: include/ProcessInfo.h type=H role=lib module=include
u36: include/LogFile.h type=H role=lib module=include
u37: include/EpollPoller.h type=H role=lib module=include
u38: include/Timer.h type=H role=lib module=include
u39: include/FileUtil.h type=H role=lib module=include
u40: include/Poller.h type=H role=lib module=include
u41: include/Callbacks.h type=H role=lib module=include
u42: include/Timestamp.h type=H role=lib module=include
u43: include/TimerQueue.h type=H role=lib module=include
u44: include/InetAddress.h type=H role=lib module=include
u45: include/Condition.h type=H role=lib module=include
u46: include/AsyncLogging.h type=H role=lib module=include
u47: include/Mutex.h type=H role=lib module=include
u48: include/AsyncLoggingInit.h type=H role=lib module=include
u49: benchmark/echo_server_bench.cpp type=CPP role=lib module=benchmark
u50: benchmark/benchmark_base.cpp type=CPP role=lib module=benchmark
u51: benchmark/latency_test.cpp type=CPP role=lib module=benchmark
u52: benchmark/self_stress_test.cpp type=CPP role=lib module=benchmark
u53: benchmark/benchmark_base.h type=H role=lib module=benchmark
u54: benchmark/echo_server_bench.h type=H role=lib module=benchmark
u55: benchmark/throughput_test.cpp type=CPP role=lib module=benchmark
u56: src/base/Timer.cpp type=CPP role=lib module=base
u57: src/base/Logger_new.cpp type=CPP role=lib module=base
u58: src/base/Socket.cpp type=CPP role=lib module=base
u59: src/base/Acceptor.cpp type=CPP role=lib module=base
u60: src/base/AsyncLogging.cpp type=CPP role=lib module=base
u61: src/base/DefaultPoller.cpp type=CPP role=lib module=base
u62: src/base/Thread.cpp type=CPP role=lib module=base
u63: src/base/InetAddress.cpp type=CPP role=lib module=base
u64: src/base/TcpServer.cpp type=CPP role=lib module=base
u65: src/base/Logger.cpp type=CPP role=lib module=base
u66: src/base/CurrentThread.cpp type=CPP role=lib module=base
u67: src/base/LogFile.cpp type=CPP role=lib module=base
u68: src/base/LogStream.cpp type=CPP role=lib module=base
u69: src/base/EpollPoller.cpp type=CPP role=lib module=base
u70: src/base/EventLoopThread.cpp type=CPP role=lib module=base
u71: src/base/Eventloop.cpp type=CPP role=lib module=base
u72: src/base/AsyncLoggingInit.cpp type=CPP role=lib module=base
u73: src/base/Timestamp.cpp type=CPP role=lib module=base
u74: src/base/TimerQueue.cpp type=CPP role=lib module=base
u75: src/base/FileUtil.cpp type=CPP role=lib module=base
u76: src/base/EventLoopThreadPool.cpp type=CPP role=lib module=base
u77: src/base/Buffer.cpp type=CPP role=lib module=base
u78: src/base/Poller.cpp type=CPP role=lib module=base
u79: src/base/Channel.cpp type=CPP role=lib module=base
u80: src/base/TcpConnection.cpp type=CPP role=lib module=base
u81: src/base/ProcessInfo.cpp type=CPP role=lib module=base
</units>
<dependency-pool>
d0: include:[Acceptor.h]
d1: include:[AsyncLogging.h]
d2: include:[AsyncLoggingInit.h]
d3: include:[Buffer.h]
d4: include:[Callbacks.h]
d5: include:[Channel.h]
d6: include:[Condition.h]
d7: include:[CurrentThread.h]
d8: include:[EpollPoller.h]
d9: include:[EventLoopThread.h]
d10: include:[EventLoopThreadPool.h]
d11: include:[Eventloop.h]
d12: include:[FileUtil.h]
d13: include:[InetAddress.h]
d14: include:[LogFile.h]
d15: include:[LogStream.h]
d16: include:[Logger.h]
d17: include:[Mutex.h]
d18: include:[Poller.h]
d19: include:[ProcessInfo.h]
d20: include:[Socket.h]
d21: include:[TcpConnection.h]
d22: include:[TcpServer.h]
d23: include:[Thread.h]
d24: include:[Timer.h]
d25: include:[TimerQueue.h]
d26: include:[Timestamp.h]
d27: include:[algorithm]
d28: include:[arpa/inet.h]
d29: include:[atomic]
d30: include:[benchmark_base.h]
d31: include:[cassert]
d32: include:[cctype]
d33: include:[cerrno]
d34: include:[chrono]
d35: include:[condition_variable]
d36: include:[cstdio]
d37: include:[cstdlib]
d38: include:[cstring]
d39: include:[dirent.h]
d40: include:[echo_server_bench.h]
d41: include:[exception]
d42: include:[fstream]
d43: include:[functional]
d44: include:[future]
d45: include:[iomanip]
d46: include:[iostream]
d47: include:[libgen.h]
d48: include:[memory]
d49: include:[mutex]
d50: include:[netinet/in.h]
d51: include:[netinet/tcp.h]
d52: include:[noncopyable.h]
d53: include:[pthread.h]
d54: include:[pwd.h]
d55: include:[set]
d56: include:[sstream]
d57: include:[stdexcept]
d58: include:[stdlib:c]
d59: include:[stdlib:py]
d60: include:[strings.h]
d61: include:[sys/epoll.h]
d62: include:[sys/eventfd.h]
d63: include:[sys/resource.h]
d64: include:[sys/socket.h]
d65: include:[sys/stat.h]
d66: include:[sys/syscall.h]
d67: include:[sys/sysinfo.h]
d68: include:[sys/time.h]
d69: include:[sys/timerfd.h]
d70: include:[sys/types.h]
d71: include:[sys/uio.h]
d72: include:[thread]
d73: include:[unordered_map]
d74: include:[vector]
</dependency-pool>
<dependencies>
u0->refs:[d26 d11 d58 d74 d57 d16 d29 d23 d24 d46 d7 d48 d41]
u1->refs:[d59 d46 d3 d38 d31]
u2->refs:[d10 d11 d34 d29 d46 d72 d31]
u3->refs:[d26 d16 d46 d24 d31 d57 d41]
u4->refs:[d64 d21 d26 d11 d59 d58 d74 d34 d27 d44 d46 d3 d13 d72 d38 d28 d24 d31]
u5->refs:[d11 d29 d16 d24 d72 d46]
u6->refs:[d31 d46 d13]
u7->refs:[d56 d58 d34 d2 d16 d46 d23 d72 d65 d42 d70]
u8->refs:[d64 d31 d61 d11 d58 d16 d46 d62 d72 d5 d33 d48]
u9->refs:[d16 d46 d31]
u10->refs:[d26 d34 d46 d72 d31]
u11->refs:[d11 d58 d74 d57 d16 d29 d46 d72 d48 d5 d31 d62]
u12->refs:[d34 d74 d29 d46 d23 d7 d55 d31 d48]
u13->refs:[d34 d74 d29 d46 d72 d7 d55 d31]
u14->refs:[d9 d11 d34 d29 d46 d31]
u15->refs:[d37 d59 d46]
u16->refs:[d64 d9 d8 d11 d58 d74 d34 d16 d29 d46 d72 d44 d5 d31 d18 d62]
u17->refs:[d21 d26 d11 d34 d59 d46 d13 d72 d22 d31]
u18->refs:[d25 d11 d74 d16 d29 d24 d72 d46 d31 d48]
u19->refs:[d64 d20 d58 d60 d46 d13 d50 d28 d31 d51]
u20->refs:[d64 d11 d58 d74 d0 d49 d16 d29 d46 d13 d72 d28 d31 d35 d48]
u22->refs:[d52 d59 d58 d29 d72 d43 d48]
u23->refs:[d52 d11 d49 d23 d35 d43]
u24->refs:[d52 d11 d59 d3 d13 d4 d48]
u25->refs:[d52]
u26->refs:[d52 d11 d59 d74 d43 d48]
u28->refs:[d10 d52 d73 d11 d59 d0 d29 d13 d4 d43]
u29->refs:[d38 d27 d59]
u30->refs:[d52 d26 d59 d15 d46 d43]
u31->refs:[d38 d27 d59 d74]
u32->refs:[d26 d52 d48 d43]
u33->refs:[d52 d26 d58 d74 d49 d29 d48 d7 d4 d5 d43]
u34->refs:[d52 d20 d43 d5]
u35->refs:[d70 d59 d74]
u36->refs:[d17 d48 d59]
u37->refs:[d18 d61 d74]
u38->refs:[d26 d29 d4]
u39->refs:[d52 d70 d59]
u40->refs:[d52 d73 d26 d74]
u41->refs:[d43 d48]
u42->refs:[d59 d46]
u43->refs:[d52 d74 d24 d5 d55 d48]
u44->refs:[d60 d28 d59 d50]
u45->refs:[d17 d58 d53]
u46->refs:[d52 d17 d59 d74 d15 d29 d23 d6 d48]
u47->refs:[d31 d7 d52 d53]
u48->refs:[d1 d43 d59]
u49->refs:[d64 d21 d26 d34 d58 d2 d16 d46 d3 d38 d50 d28 d40]
u50->refs:[d45 d27 d46 d30]
u51->refs:[d11 d34 d21 d3 d38 d42 d48 d45 d30 d64 d58 d74 d2 d16 d46 d72 d50 d28 d22]
u52->refs:[d11 d34 d13 d21 d24 d3 d38 d42 d48 d45 d64 d26 d58 d74 d2 d16 d29 d46 d50 d72 d28 d22]
u53->refs:[d26 d74 d49 d29 d43]
u54->refs:[d11 d30 d29 d72 d22 d48]
u55->refs:[d45 d30 d2 d46 d42 d40]
u56->refs:[d24]
u57->refs:[d1 d16 d58]
u58->refs:[d64 d20 d58 d16 d13 d51 d70]
u59->refs:[d64 d11 d58 d0 d16 d13 d70]
u60->refs:[d1 d26 d58 d14]
u61->refs:[d8 d18 d58]
u62->refs:[d56 d36 d23 d66 d70]
u63->refs:[d38 d60 d46 d13]
u64->refs:[d21 d20 d11 d58 d0 d16 d22 d43]
u65->refs:[d16]
u66->refs:[d66 d7 d58]
u67->refs:[d68 d58 d14 d12 d19]
u68->refs:[d36 d27 d15 d32 d38 d43]
u69->refs:[d8 d11 d58 d16 d5]
u70->refs:[d9 d11]
u71->refs:[d27 d25 d11 d58 d16 d31 d18 d62]
u72->refs:[d37 d2 d16 d48]
u73->refs:[d68 d26 d59]
u74->refs:[d27 d25 d58 d11 d16 d69]
u75->refs:[d47 d58 d12 d65 d70]
u76->refs:[d10 d9 d58]
u77->refs:[d58 d71 d3]
u78->refs:[d18 d11]
u79->refs:[d11 d16 d46 d5 d61]
u80->refs:[d20 d21 d11 d58 d16 d3 d5 d43]
u81->refs:[d58 d67 d63 d39 d7 d19 d54 d70]
</dependencies>
<symbols>
timeout1:u0 func
timeout2:u0 func
timeout3:u0 func
test_eventloop_timer:u0 func
thread:u0 func
test_cross_thread_timer:u0 func
timerThread:u0 func
test_timer_callback_exception:u0 func
test_massive_timers:u0 func
test_past_expiration:u0 func
test_very_short_interval:u0 func
test_same_expiration:u0 func
test_cleanup_on_exit:u0 func
test_cancel_during_execution:u0 func
test_concurrent_operations:u0 func
main:u0 func entry=true
testBufferEmpty:u1 func
testBufferAppendRetrieve:u1 func
testBufferRetrieveAsString:u1 func
testBufferGrow:u1 func
testBufferAppendVoidPtr:u1 func
testBufferInsideGrow:u1 func
testBufferShrink:u1 func
testBufferPrepend:u1 func
testBufferReadInt:u1 func
testBufferFindCRLF:u1 func
main:u1 func entry=true
test_basic_creation:u2 func
test_get_all_loops:u2 func
test_get_next_loop:u2 func
test_init_callback:u2 func
test_run_in_multiple_loops:u2 func
test_concurrent_get_loop:u2 func
main:u2 func entry=true
test_timer_basic:u3 func
test_timer_sequence:u3 func
test_timer_expiration:u3 func
test_timer_callback_exception:u3 func
test_timer_invalid_timestamp:u3 func
test_timer_null_callback:u3 func
test_timer_very_short_interval:u3 func
test_timer_negative_interval:u3 func
test_timer_zero_interval:u3 func
main:u3 func entry=true
test_connection_lifecycle:u4 func
test_message_send_receive:u4 func
test_high_water_mark:u4 func
test_force_close:u4 func
test_multithread_send:u4 func
test_shutdown_write:u4 func
test_send_with_buffer:u4 func
test_write_complete_callback:u4 func
test_close_callback:u4 func
test_message_send_receive_improved:u4 func
test_edge_cases:u4 func
test_repeated_close:u4 func
test_operation_in_callback:u4 func
test_large_message:u4 func
test_high_water_mark_improved:u4 func
test_cross_thread_operations:u4 func
test_memory_leak:u4 func
main:u4 func entry=true
test_basic_timer:u5 func
test_timer_cancel:u5 func
main:u5 func entry=true
test_inetaddress_with_port:u6 func
test_inetaddress_with_ip_and_port:u6 func
test_inetaddress_get_sockaddr:u6 func
main:u6 func entry=true
testBasicLogging:u7 func
testMultiThreadLogging:u7 func
test1_BasicLogging:u7 func
test2_MultiThreadLogging:u7 func
test3_NonExistentPath:u7 func
test4_LargeVolumeLogging:u7 func
test5_LogFileRolling:u7 func
test6_EmptyLog:u7 func
test7_VeryLongMessage:u7 func
test8_RapidStartStop:u7 func
test9_HighConcurrencyStress:u7 func
test10_ZeroLengthMessage:u7 func
main:u7 func entry=true
test_channel_creation:u8 func
test_channel_events:u8 func
test_channel_callbacks:u8 func
test_channel_tie:u8 func
test_channel_all_events:u8 func
test_channel_edge_cases:u8 func
test_channel_multithread:u8 func
test_channel_remove:u8 func
main:u8 func entry=true
test_logger_instance:u9 func
test_logger_log_levels:u9 func
test_logger_set_log_level:u9 func
main:u9 func entry=true
test_timestamp_default_constructor:u10 func
test_timestamp_constructor_with_microseconds:u10 func
test_timestamp_now:u10 func
test_timestamp_to_string:u10 func
test_timestamp_comparison_operators:u10 func
test_timestamp_valid:u10 func
test_timestamp_invalid:u10 func
test_addTime:u10 func
test_timeDifference:u10 func
test_timestamp_boundary_values:u10 func
test_timestamp_precision:u10 func
test_timestamp_format:u10 func
main:u10 func entry=true
test_eventloop_basic:u11 func
test_eventloop_channel:u11 func
test_eventloop_runinloop:u11 func
test_eventloop_queueinloop:u11 func
test_eventloop_wakeup:u11 func
test_eventloop_pollreturntime:u11 func
test_channel_write_event:u11 func
test_channel_close_event:u11 func
test_channel_tie:u11 func
test_eventloop_haschannel:u11 func
test_eventloop_isinloopthread:u11 func
main:u11 func entry=true
test_thread_creation:u12 func
thread:u12 func
test_thread_id:u12 func
thread:u12 func
test_default_naming:u12 func
test_concurrent_threads:u12 func
test_thread_count:u12 func
t1:u12 func
t2:u12 func
test_multiple_join:u12 func
thread:u12 func
test_duplicate_start:u12 func
thread:u12 func
test_destructor_detach:u12 func
thread:u12 func
test_thread_exception:u12 func
thread:u12 func
test_currentthread_tid_isolation:u12 func
test_unstarted_thread_tid:u12 func
thread:u12 func
test_high_concurrent_thread_count:u12 func
test_thread_name_edge_cases:u12 func
t1:u12 func
t2:u12 func
t3:u12 func
test_join_thread_safety:u12 func
thread:u12 func
test_thread_resource_management:u12 func
test_join_without_start:u12 func
thread:u12 func
test_thread_self_access:u12 func
thread:u12 func
test_massive_thread_creation:u12 func
main:u12 func entry=true
test_current_thread_tid:u13 func
test_multiple_threads:u13 func
test_thread_local_storage:u13 func
test_nested_threads:u13 func
test_tid_stability:u13 func
test_massive_concurrent_threads:u13 func
test_thread_reuse:u13 func
test_main_thread:u13 func
test_tid_performance:u13 func
test_tid_edge_cases:u13 func
main:u13 func entry=true
test_basic_creation:u14 func
test_thread_naming:u14 func
test_init_callback:u14 func
test_run_in_loop:u14 func
test_multiple_creation:u14 func
test_concurrent_loops:u14 func
main:u14 func entry=true
main:u15 func entry=true
test_poller_creation:u16 func
test_epollpoller_creation:u16 func
test_epollpoller_channel_operations:u16 func
test_epollpoller_poll:u16 func
test_epollpoller_edge_cases:u16 func
test_epollpoller_special_events:u16 func
test_epollpoller_resource_management:u16 func
test_epollpoller_concurrent:u16 func
main:u16 func entry=true
test_basic_creation:u17 func
t:u17 func
test_multithread:u17 func
t:u17 func
test_connection_management:u17 func
t:u17 func
test_thread_init_callback:u17 func
t:u17 func
main:u17 func entry=true
test_timer_queue_basic:u18 func
test_timer_queue_cancel:u18 func
test_timer_queue_concurrent:u18 func
test_timer_queue_null_callback:u18 func
test_timer_queue_cancel_during_execution:u18 func
test_timer_queue_massive_timers:u18 func
test_timer_queue_past_expiration:u18 func
test_timer_queue_very_short_interval:u18 func
test_timer_queue_same_expiration:u18 func
test_timer_queue_cleanup_on_exit:u18 func
main:u18 func entry=true
test_socket_bind:u19 func
test_socket_listen:u19 func
test_socket_accept:u19 func
test_socket_tcp_nodelay:u19 func
test_socket_reuse_addr:u19 func
test_socket_reuse_port:u19 func
test_socket_keep_alive:u19 func
test_socket_shutdown_write:u19 func
test_socket_get_local_addr:u19 func
test_socket_noncopyable:u19 func
test_socket_destructor:u19 func
test_socket_accept_flags:u19 func
test_socket_tcp_nodelay_off:u19 func
test_socket_reuse_addr_off:u19 func
test_socket_reuse_port_off:u19 func
test_socket_keep_alive_off:u19 func
test_socket_shutdown_write_effect:u19 func
test_socket_multiple_bind:u19 func
test_socket_get_local_addr_after_connect:u19 func
main:u19 func entry=true
onNewConnection:u20 func
test_acceptor_creation:u20 func
test_acceptor_listen:u20 func
test_acceptor_multiple_connections:u20 func
main:u20 func entry=true
tid:u21 func
numCreated:u22 func
setConnectionCallback:u24 func
setMessageCallback:u24 func
setWriteCompleteCallback:u24 func
setCloseCallback:u24 func
setHighWaterMarkCallback:u24 func
setState:u24 func
Socket:u25 func
setThreadNUm:u26 func
setThreadInitCallback:u28 func
setConnectionCallback:u28 func
setMessageCallback:u28 func
setWriteCompleteCallback:u28 func
append:u29 func
reset:u29 func
bzero:u29 func
append:u29 func
add:u29 func
reset:u29 func
bzero:u29 func
Buffer:u31 func
retrieve:u31 func
retrieveUntil:u31 func
retrieveAll:u31 func
append:u31 func
append:u31 func
append:u31 func
prepend:u31 func
shrink:u31 func
makeSpace:u31 func
ensureWritableBytes:u31 func
setReadCallback:u32 func
setWriteCallback:u32 func
setCloseCallback:u32 func
setErrorCallback:u32 func
setRevents:u32 func
index:u32 func
setIndex:u32 func
enableReading:u32 func
disableReading:u32 func
enableWriting:u32 func
disableWriting:u32 func
disableAll:u32 func
assertInLoopThread:u33 func
setNewConnectionCallback:u34 func
run:u38 func
numCreated:u38 func
addTime:u42 func
timeDifference:u42 func
InetAddress:u44 func
setSockAddr:u44 func
Condition:u45 func
wait:u45 func
waitForSeconds:u45 func
notify:u45 func
notifyAll:u45 func
lock:u47 func
unlock:u47 func
MutexLockGuard:u47 func
run_latency_tests:u51 func
main:u51 func entry=true
run:u52 func
runServer:u52 func
runClients:u52 func
main:u52 func entry=true
run_throughput_benchmark:u55 func
main:u55 func entry=true
createNonblockingOrDie:u59 func
gettid:u62 func
main:u63 func entry=true
cacheTid:u66 func
convert:u68 func
createEventfd:u71 func
initAsyncLogging:u72 func
resetAsyncLogging:u72 func
setAsyncOutput:u72 func
enableTestMode:u72 func
hasErrorLog:u72 func
cleanupLogFiles:u72 func
main:u73 func entry=true
createTimerfd:u74 func
howMuchTimeFromNow:u74 func
resetTimerfd:u74 func
readTimerfd:u74 func
pid:u81 func
uid:u81 func
euid:u81 func
openFiles:u81 func
maxOpenFiles:u81 func
threads:u81 func
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