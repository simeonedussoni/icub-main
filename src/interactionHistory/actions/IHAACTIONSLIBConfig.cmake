
MESSAGE(STATUS "IHAACTIONS Library")

IF (NESTED_BUILD)
  SET(IHAACTIONSLIB_LIBRARIES ihaActionsLib)
ELSE (NESTED_BUILD)
  FIND_LIBRARY(IHAACTIONSLIB_LIBRARIES ihaActionsLib ${IHAACTIONSLIB_DIR})
ENDIF (NESTED_BUILD)

SET(IHAACTIONSLIB_INCLUDE_DIRS ${IHAACTIONSLIB_DIR}/include)

