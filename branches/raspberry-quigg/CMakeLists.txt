cmake_minimum_required (VERSION 2.6)
project (RCSwitch)
# The version number.
set (RCSwitch_VERSION_MAJOR 1)
set (RCSwitch_VERSION_MINOR 0)

option (RASPBERRY "Compile for RaspberryPi" ON)

if (RASPBERRY)
  set (EXTRA_LIBS ${EXTRA_LIBS} wiringPi)
  add_definitions(-DRASPBERRY)
endif (RASPBERRY)

add_library (RCSwitch SHARED RCSwitch.cpp)
target_link_libraries(RCSwitch ${EXTRA_LIBS})

install (TARGETS RCSwitch DESTINATION lib)
install (FILES RCSwitch.h DESTINATION include)

