# Target boundary only. The observed file codec belongs to Foundation 4.
add_library(tora_compat INTERFACE)
add_library(tora::compat ALIAS tora_compat)
target_link_libraries(tora_compat INTERFACE tora::core)
