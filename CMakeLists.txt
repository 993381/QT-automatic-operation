cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x040800)

set(SOURCES
    #    sop.h
    #    sop.cpp

        util.h
        util.cpp

        hooks.h
        hooks.cpp

        probe.h
        probe.cpp

        sigspy.h
        sigspy.cpp

        scopeguard.h

        objectpath.h
        objectpath.cpp

        probeguard.h
        probeguard.cpp

        entry_unix.cpp

        probecreator.h
        probecreator.cpp

    #    objecthelper.h
    #    objecthelper.cpp

        uiacontroller.h
        uiacontroller.cpp

        operationmanager.h
        operationmanager.cpp

        # 记录一些对象的黑白名单
        objectlistmanager.h
        objectlistmanager.cpp

        objectpathresolver.h
        objectpathresolver.cpp

        signalspycallbackset.h
        signalspycallbackset.cpp
)
add_library(injector SHARED ${SOURCES})
set_target_properties(injector PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(injector Qt5::Core Qt5::CorePrivate Qt5::Gui Qt5::Widgets)

add_executable(injector_test_demo main.cpp ${SOURCES})
target_link_libraries(injector_test_demo Qt5::Core Qt5::CorePrivate Qt5::Gui Qt5::Widgets)
