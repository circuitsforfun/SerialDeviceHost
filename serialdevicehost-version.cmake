set(SERIAL_DEVICE_HOST_VERSION 1.0.0)
set(PACKAGE_VERSION ${SERIAL_DEVICE_HOST_VERSION})

set(PACKAGE_VERSION_EXACT False)
set(PACKAGE_VERSION_COMPATIBLE False)

if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
  set(PACKAGE_VERSION_EXACT True)
  set(PACKAGE_VERSION_COMPATIBLE True)
endif()

if(PACKAGE_FIND_VERSION_MAJOR EQUAL 1
   AND PACKAGE_FIND_VERSION VERSION_LESS PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE True)
endif()
