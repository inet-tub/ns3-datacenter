build_example(
  NAME tutorial-3
  SOURCE_FILES 
    tutorial-3.cc
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)
build_example(
  NAME tutorial-4_5
  SOURCE_FILES 
    tutorial-4_5.cc
  LIBRARIES_TO_LINK
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libinternet}
    ${libapplications}
)