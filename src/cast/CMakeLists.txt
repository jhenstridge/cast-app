
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(
  generated_sources generated_headers
  proto/cast_channel.proto)
add_library(cast-qml MODULE
  plugin.cpp
  caster.cpp
  channel.cpp
  interface.cpp
  connection-interface.cpp
  heartbeat-interface.cpp
  receiver-interface.cpp
  media-interface.cpp
  ${generated_sources}
  ${generated_headers}
  )
set_target_properties(cast-qml PROPERTIES
  AUTOMOC TRUE
  NO_SONAME TRUE
  LINK_FLAGS "-Wl,--no-undefined"
  COMPILE_FLAGS "-fvisibility=hidden")
target_include_directories(cast-qml PRIVATE
  ${PROTOBUF_INCLUDE_DIRS}
  )
target_compile_options(cast-qml PRIVATE
  -DQT_NO_KEYWORDS)
target_link_libraries(cast-qml PRIVATE
  Qt5::Core
  Qt5::Qml
  ${PROTOBUF_LITE_LIBRARIES})

add_custom_target(cast-qmldir ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/qmldir
  DEPENDS qmldir cast-qml
  )
