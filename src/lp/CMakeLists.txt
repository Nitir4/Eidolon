# Target boundary only. LP model/validation belongs to Foundation 3.
add_library(or_lp INTERFACE)
add_library(tora::lp ALIAS or_lp)
target_link_libraries(or_lp INTERFACE tora::core)
