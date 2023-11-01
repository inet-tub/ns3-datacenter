#build_example(
#  NAME queueing-logs
#  SOURCE_FILES 
#    queueing-logs.cc
#    cdf.c
#  HEADER_FILES
#    cdf.h
#  LIBRARIES_TO_LINK
#    ${libpoint-to-point}
#    ${libtraffic-control}
#    ${libinternet}
#    ${libapplications}
#    ${libpybind11}
#)
#set_source_files_properties(cdf.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
