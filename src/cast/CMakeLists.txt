
add_definitions(-DQT_NO_KEYWORDS ${AVAHI_CFLAGS})

add_library(cast-qml MODULE
  plugin.cpp
  browser.cpp
  )
set_target_properties(cast-qml PROPERTIES
  AUTOMOC TRUE
  LINK_FLAGS "-Wl,--no-undefined")
qt5_use_modules(cast-qml Core Qml)
target_link_libraries(cast-qml ${AVAHI_LDFLAGS})

add_custom_target(cast-qmldir ALL
  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/qmldir
  DEPENDS qmldir cast-qml
  )
