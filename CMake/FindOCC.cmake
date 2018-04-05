# Try to find OCC
# Once done this will define
#
# OCC_FOUND          - system has OCC - OpenCASCADE
# OCC_INCLUDE_DIR    - where the OCC include directory can be found
# OCC_LIBRARY_DIR    - where the OCC library directory can be found
# OCC_LIBRARIES      - Link this to use OCC
# OCC_OCAF_LIBRARIES - Link this to use OCC OCAF framework

find_package(OCE)
if (OCE_FOUND)
 set(OCC_LIBRARIES ${OCE_LIBRARIES})
 set(OCC_INCLUDE_DIRS ${OCE_INCLUDE_DIR} ${OCE_INCLUDE_DIRS})
 set(OCC_INCLUDE_DIR ${OCC_INCLUDE_DIRS})
else()
message(STATUS "OCE not found, looking for OpenCASCADE")
 find_package(OpenCASCADE)
 set(OCC_FOUND ${OpenCASCADE_FOUND})
 set(OCC_LIBRARIES ${OpenCASCADE_LIBRARIES})
 set(OCC_INCLUDE_DIRS ${OpenCASCADE_INCLUDE_DIR})
 set(OCC_INCLUDE_DIR ${OpenCASCADE_INCLUDE_DIR})
 
endif()
