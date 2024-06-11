build_example(
  NAME dcn-congestion-control-simple
  SOURCE_FILES 
    dcn-congestion-control-simple.cc
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)