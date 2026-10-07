cmake_minimum_required(VERSION 3.21)

project(BMCMTrayLauncherQt VERSION 1.1.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui")

find_package(Qt6 6.4 REQUIRED COMPONENTS Network Widgets)
include(CTest)

configure_file(
    cmake/app_version.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/generated/app_version.h"
    @ONLY
)

set(LAUNCHER_SOURCES
    src/applicationinstance.cpp
    src/applicationinstance.h
    src/autostartmanager.cpp
    src/autostartmanager.h
    src/backendcontroller.cpp
    src/backendcontroller.h
    src/launcherconfig.cpp
    src/launcherconfig.h
    src/launcherupdater.cpp
    src/launcherupdater.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/readinesschecker.cpp
    src/readinesschecker.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/ui/mainwindow.ui
    src/ui/settingsdialog.ui
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${LAUNCHER_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${LAUNCHER_SOURCES})
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Network
)

if(BUILD_TESTING)
    add_executable(launcher_runtime_test
        tests/launcher_runtime_test.cpp
        src/applicationinstance.cpp
        src/applicationinstance.h
        src/autostartmanager.cpp
        src/autostartmanager.h
        src/backendcontroller.cpp
        src/backendcontroller.h
        src/launcherconfig.cpp
        src/launcherconfig.h
        src/launcherupdater.cpp
        src/launcherupdater.h
        src/readinesschecker.cpp
        src/readinesschecker.h
    )
    target_include_directories(launcher_runtime_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(launcher_runtime_test PRIVATE Qt6::Core Qt6::Network)
    target_compile_definitions(launcher_runtime_test
        PRIVATE TEST_CMAKE_EXECUTABLE="${CMAKE_COMMAND}"
    )
    add_test(NAME launcher_runtime COMMAND launcher_runtime_test)
    set_tests_properties(launcher_runtime PROPERTIES TIMEOUT 20)
endif()

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt6 windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
        REQUIRED
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --release
            --no-translations
            --compiler-runtime
            "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Deploying Qt and compiler runtime libraries"
        VERBATIM
    )

    if(MINGW)
        find_program(NTLDD_EXECUTABLE NAMES ntldd REQUIRED)
        get_filename_component(MINGW_COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                "-DAPP_FILE=$<TARGET_FILE:${PROJECT_NAME}>"
                "-DDEPLOY_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                "-DNTLDD_EXECUTABLE=${NTLDD_EXECUTABLE}"
                "-DMINGW_BIN_DIR=${MINGW_COMPILER_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployMingwDependencies.cmake"
            COMMENT "Deploying MinGW and Qt runtime dependencies"
            VERBATIM
        )
    endif()
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
