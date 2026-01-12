FILE(GLOB_RECURSE app *.*)
FILE(GLOB_RECURSE libs ../lib/*.c)
idf_component_register(
  SRCS ${app} ${libs}
  INCLUDE_DIRS "." ".." "../lib/"
)