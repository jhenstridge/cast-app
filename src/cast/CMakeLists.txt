
include(FindPkgConfig)
find_package(Qt5Core REQUIRED)
find_package(Protobuf REQUIRED)
pkg_check_modules(AVAHI REQUIRED glib-2.0 avahi-core avahi-glib)

include_directories(${PROTOBUF_INCLUDE_DIRS})
add_definitions(-DQT_NO_KEYWORDS ${AVAHI_CFLAGS})

protobuf_generate_cpp(
  generated_sources generated_headers
  proto/cast_channel.proto)
add_library(cast-qml MODULE
  plugin.cpp
  browser.cpp
  caster.cpp
  channel.cpp
  interface.cpp
  connection-interface.cpp
  heartbeat-interface.cpp
  receiver-interface.cpp
  ${generated_sources}
  ${generated_headers}
  )
set_target_properties(cast-qml PROPERTIES
  AUTOMOC TRUE
  NO_SONAME TRUE
  LINK_FLAGS "-Wl,--no-undefined"
  COMPILE_FLAGS "-fvisibility=hidden")
qt5_use_modules(cast-qml Core Qml)
target_link_libraries(cast-qml ${AVAHI_LDFLAGS} ${PROTOBUF_LITE_LIBRARIES})

add_custom_target(cast-qmldir ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/qmldir
  DEPENDS qmldir cast-qml
  )
