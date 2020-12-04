link_directories(
        ${GTKMM_LIBRARY_DIRS}  )

include_directories(
        ${GTKMM_INCLUDE_DIRS}  )

add_executable(navy_combat
        main.cpp)

target_link_libraries(navy_combat
        ${GTKMM_LIBRARIES}  )
