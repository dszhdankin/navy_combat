set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server main.cpp)

target_link_libraries(server PRIVATE Threads::Threads)