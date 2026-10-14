cmake_minimum_required(VERSION 3.21)

project(BMCMTrayLauncherQt VERSION 1.1.15 LANGUAGES CXX)

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
    src/launcherreleaseupdater.cpp
    src/launcherreleaseupdater.h
    src/launcherupdater.cpp
    src/launcherupdater.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/readinesschecker.cpp
    src/readinesschecker.h
    src/systemrepositoryupdater.cpp
    src/systemrepositoryupdater.h
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

target_sources(${PROJECT_NAME} PRIVATE recursos.qrc)
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE recurso.rc)
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
        src/launcherreleaseupdater.cpp
        src/launcherreleaseupdater.h
        src/launcherupdater.cpp
        src/launcherupdater.h
        src/readinesschecker.cpp
        src/readinesschecker.h
        src/systemrepositoryupdater.cpp
        src/systemrepositoryupdater.h
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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/bmcm-tray-launcher-qt.desktop"
        DESTINATION share/applications)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app.png"
        DESTINATION share/pixmaps
        RENAME bmcm-tray-launcher-qt.png)
    install(CODE [[
        file(CHMOD
            "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}"
            "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin"
            "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share"
            "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/applications"
            "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/pixmaps"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE)
    ]])

    set(CPACK_PACKAGE_NAME "bmcm-tray-launcher-qt")
    set(CPACK_PACKAGE_VENDOR "BMCM")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Launcher para execução e monitoramento do BMCM")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "BMCM")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-qpa-plugins")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}")
    include(CPack)
endif()
