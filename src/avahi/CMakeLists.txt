include(FindPkgConfig)
find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
pkg_check_modules(AVAHI REQUIRED glib-2.0 avahi-core avahi-glib)

add_library(avahi-qml MODULE
  plugin.cpp
  browser.cpp
  )
set_target_properties(avahi-qml PROPERTIES
  AUTOMOC TRUE
  NO_SONAME TRUE
  LINK_FLAGS "-Wl,--no-undefined"
  COMPILE_FLAGS "-fvisibility=hidden")
target_compile_options(avahi-qml PRIVATE
  -DQT_NO_KEYWORDS
  ${AVAHI_CFLAGS}
  )
target_link_libraries(avahi-qml PRIVATE
  Qt5::Core
  Qt5::Qml
  ${AVAHI_LDFLAGS}
  )

add_custom_target(avahi-qmldir ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/qmldir
  DEPENDS qmldir cast-qml
  )
