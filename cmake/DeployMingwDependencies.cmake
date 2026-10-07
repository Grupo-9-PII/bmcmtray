foreach(REQUIRED_VARIABLE IN ITEMS APP_FILE DEPLOY_DIR NTLDD_EXECUTABLE MINGW_BIN_DIR)
    if(NOT DEFINED ${REQUIRED_VARIABLE})
        message(FATAL_ERROR "${REQUIRED_VARIABLE} was not provided to the MinGW deployment script")
    endif()
endforeach()

execute_process(
    COMMAND "${NTLDD_EXECUTABLE}" -R "${APP_FILE}"
    RESULT_VARIABLE NTLDD_RESULT
    OUTPUT_VARIABLE NTLDD_OUTPUT
    ERROR_VARIABLE NTLDD_ERROR
)

if(NOT NTLDD_RESULT EQUAL 0)
    message(FATAL_ERROR "ntldd failed to inspect ${APP_FILE}: ${NTLDD_ERROR}")
endif()

file(TO_CMAKE_PATH "${MINGW_BIN_DIR}" MINGW_BIN_DIR_NORMALIZED)
string(TOLOWER "${MINGW_BIN_DIR_NORMALIZED}/" MINGW_BIN_PREFIX)
string(REPLACE "\r\n" "\n" NTLDD_OUTPUT "${NTLDD_OUTPUT}")
string(REPLACE "\n" ";" NTLDD_LINES "${NTLDD_OUTPUT}")

set(DEPLOYED_DLLS)
foreach(NTLDD_LINE IN LISTS NTLDD_LINES)
    if(NTLDD_LINE MATCHES "=> ([A-Za-z]:.*[.]dll) [(]")
        set(DEPENDENCY_PATH "${CMAKE_MATCH_1}")
        file(TO_CMAKE_PATH "${DEPENDENCY_PATH}" DEPENDENCY_PATH_NORMALIZED)
        string(TOLOWER "${DEPENDENCY_PATH_NORMALIZED}" DEPENDENCY_PATH_LOWER)
        string(FIND "${DEPENDENCY_PATH_LOWER}" "${MINGW_BIN_PREFIX}" MINGW_PATH_INDEX)

        if(MINGW_PATH_INDEX EQUAL 0)
            get_filename_component(DEPENDENCY_NAME "${DEPENDENCY_PATH_NORMALIZED}" NAME)
            list(FIND DEPLOYED_DLLS "${DEPENDENCY_NAME}" DLL_INDEX)
            if(DLL_INDEX EQUAL -1)
                file(COPY_FILE
                    "${DEPENDENCY_PATH_NORMALIZED}"
                    "${DEPLOY_DIR}/${DEPENDENCY_NAME}"
                    ONLY_IF_DIFFERENT
                )
                list(APPEND DEPLOYED_DLLS "${DEPENDENCY_NAME}")
            endif()
        endif()
    endif()
endforeach()

list(LENGTH DEPLOYED_DLLS DEPLOYED_DLL_COUNT)
message(STATUS "Deployed ${DEPLOYED_DLL_COUNT} MinGW runtime dependencies")