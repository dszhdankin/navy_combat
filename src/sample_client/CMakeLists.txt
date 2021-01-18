set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(client main.cpp)

target_link_libraries(client PRIVATE Threads::Threads)