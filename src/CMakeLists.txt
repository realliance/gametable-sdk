set(DIR ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME}
  PRIVATE
  "${DIR}/network.cpp"
  "${DIR}/helper.cpp"
)
