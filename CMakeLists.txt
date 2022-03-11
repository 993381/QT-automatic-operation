cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets WebChannel)
set(QT_LIBS
    Qt5::Core
    Qt5::CorePrivate
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineCore
    Qt5::WebEngineWidgets
    Qt5::WebChannel
)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x040800)

set(SOURCES
    #    sop.h
    #    sop.cpp

    gdbinjector/gdb_injector.h
    gdbinjector/gdb_injector.cpp

        scriptengine/scriptengine.h
        scriptengine/scriptengine.cpp

        scriptengine/js_cpp_interface.h
        scriptengine/js_cpp_interface.cpp

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

set(SOURCES ${SOURCES}
    "${PROJECT_SOURCE_DIR}/js/index.html"
    "${PROJECT_SOURCE_DIR}/js/tester.js"
    "${PROJECT_SOURCE_DIR}/js/testcase.js"
    "${PROJECT_SOURCE_DIR}/js/qwebchannel.js"
)

add_definitions(
    # -DINDEX_HTML="${PROJECT_SOURCE_DIR}/js/index.html"
    -DTESTER_JS="${PROJECT_SOURCE_DIR}/js/tester.js"
    -DTESTCASE_JS="${PROJECT_SOURCE_DIR}/js/testcase.js"       # 改成动态加载，用户手动指定比较好
    -DCHANNEL_JS="${PROJECT_SOURCE_DIR}/js/qwebchannel.js"
    -DINJECTOR_DLL="${CMAKE_BINARY_DIR}/libinjector.so"
)

add_library(injector SHARED ${SOURCES})
set_target_properties(injector PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(injector ${QT_LIBS})

add_executable(injector_test_demo main.cpp ${SOURCES})
target_link_libraries(injector_test_demo ${QT_LIBS})
