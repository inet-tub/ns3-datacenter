build_example(
  NAME powertcp-evaluation-burst
  SOURCE_FILES 
    powertcp-evaluation-burst.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
build_example(
  NAME powertcp-evaluation-fairness
  SOURCE_FILES 
    powertcp-evaluation-fairness.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
build_example(
  NAME powertcp-evaluation-workload
  SOURCE_FILES 
    powertcp-evaluation-workload.cc
    cdf.c
  HEADER_FILES
    cdf.h
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
set_source_files_properties(cdf.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)