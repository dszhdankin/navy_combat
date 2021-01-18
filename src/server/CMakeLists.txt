set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server main.cpp
        ../utils/StraightforwardLogger.cpp
        ../utils/Reader.cpp
        ../utils/Writer.cpp
        Player.cpp)

target_link_libraries(server PRIVATE Threads::Threads)