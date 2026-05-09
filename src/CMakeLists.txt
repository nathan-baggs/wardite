add_library(dinput8 SHARED
  dinput8.def
  dinput8.cpp
)

install(TARGETS dinput8 DESTINATION bin)

target_compile_options(dinput8 PRIVATE
  -Wall
  -Wextra
  -pedantic
  -Werror
  -Wconversion-null
  -Wmissing-declarations
  -Woverlength-strings
  -Wpointer-arith
  -Wunused-local-typedefs
  -Wunused-result
  -Wvarargs
  -Wvla
  -Wwrite-strings
  -Wno-cast-function-type
)
target_compile_features(dinput8 PUBLIC cxx_std_26)
target_compile_definitions(dinput8 PUBLIC -DNOMINMAX)
target_link_libraries(dinput8 PUBLIC stdc++exp)
set_target_properties(dinput8 PROPERTIES PREFIX "")
