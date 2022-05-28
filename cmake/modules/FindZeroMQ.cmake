# Try to find the ZeroMQ librairies
# ZMQ_FOUND - system has ZeroMQ lib
# ZMQ_INCLUDE_DIR - the ZeroMQ include directory
# ZMQ_LIBRARY - Libraries needed to use ZeroMQ

if(ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY)
	# Already in cache, be silent
	set(ZMQ_FIND_QUIETLY TRUE)
endif()

find_path(ZMQ_INCLUDE_DIR NAMES zmq.h)

if (MSVC)
  find_library(ZMQ_LIBRARY NAMES libzmq-mt-s-4_3_2 libzmq-mt-s-4_3_3)
else()
  find_library(ZMQ_LIBRARY NAMES zmq libzmq)
  find_library(PGM_LIBRARY NAMES pgm)
  find_library(SODIUM_LIBRARY NAMES sodium)
  find_library(NORM_LIBRARY NAMES norm)
  find_library(PROTOLIB_LIBRARY NAMES protolib)
  find_library(COMERR_LIBRARY NAMES com_err)
  find_library(GSSAPI_LIBRARY NAMES gssapi_krb5)
  find_library(K5CRYPTO_LIBRARY NAMES k5crypto)
  find_library(KRB5_LIBRARY NAMES krb5)
  find_library(KRB5SUPPORT_LIBRARY NAMES krb5support)
  find_library(KEYUTILS_LIBRARY NAMES keyutils)
  find_library(RESOLV_LIBRARY NAMES resolv)
endif()

message(STATUS "ZeroMQ lib: " ${ZMQ_LIBRARY})
message(STATUS "       lib: " ${PGM_LIBRARY})
message(STATUS "       lib: " ${SODIUM_LIBRARY})
message(STATUS "       lib: " ${NORM_LIBRARY})
message(STATUS "       lib: " ${PROTOLIB_LIBRARY})
message(STATUS "       lib: " ${GSSAPI_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZeroMQ DEFAULT_MSG ZMQ_INCLUDE_DIR ZMQ_LIBRARY)

mark_as_advanced(ZMQ_INCLUDE_DIR ZMQ_LIBRARY)

set(ZeroMQ_LIBRARIES ${ZMQ_LIBRARY})
set(ZeroMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})
