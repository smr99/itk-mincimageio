PROJECT(mincTests)

cmake_minimum_required(VERSION 2.6)

FIND_PACKAGE(ITK REQUIRED)

INCLUDE(${ITK_USE_FILE})

SET( common_LIBS
  ITKCommon
  ITKBasicFilters
  ITKIO
  minc2
  gtest gtest_main
)


ADD_EXECUTABLE( testMINCImageIO testMINCImageIO.cxx itkMINCImageIO.cxx )
TARGET_LINK_LIBRARIES( testMINCImageIO ${common_LIBS} )

ENABLE_TESTING()
ADD_TEST( testMINCImageIO testMINCImageIO )
