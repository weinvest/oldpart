INCLUDE(CollectSrcs)
SET(compress_SUBDIRS )
GET_SRCS(compress_SRCLISTS compress_SUBDIRS)
FILE(GLOB_RECURSE ${TARGET}_HEADER_LISTS "*.h" "*.hpp")
INSTALL_FILES(include ${TARGET}_HEADER_LISTS)
