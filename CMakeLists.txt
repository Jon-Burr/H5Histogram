cmake_minimum_required(VERSION 3.12)
project(H5Histograms VERSION 0.0.0)

find_package(HDF5 COMPONENTS CXX REQUIRED)

add_library(H5Histograms SHARED)
target_sources(H5Histograms
PRIVATE
    src/CategoryAxis.cxx
    src/FixedBinAxis.cxx
    src/Histogram.cxx
    src/HistogramBase.cxx
    src/IAxis.cxx
    src/NumericAxis.cxx
    src/VariableBinAxis.cxx
)
target_include_directories(H5Histograms
    PUBLIC include ${HDF5_INCLUDE_DIRS}
)
target_link_libraries(H5Histograms
    PUBLIC ${HDF5_LIBRARIES} H5Composites
)