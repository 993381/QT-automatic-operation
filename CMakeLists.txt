cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# sudo apt install libqt5websockets5-dev qtwebengine5-dev
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets WebChannel DBus Network WebSockets)
set(QT_LIBS
    Qt5::Core
    Qt5::CorePrivate
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineCore
    Qt5::WebEngineWidgets
    Qt5::WebChannel
    Qt5::DBus
    Qt5::Network
    Qt5::WebSockets
)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x040800)

set(LOCAL_SOCKET
    local_socket/local_client.h
    # local_socket/local_client.cpp

    local_socket/local_server.h
    # local_socket/local_server.cpp

    # local_socket/message.h
    # local_socket/message.cpp
)

set(WEBSOKETS
    websocket/proto.h
    websocket/echoclient.cpp
    websocket/echoclient.h
    websocket/echoserver.cpp
    websocket/echoserver.h
    websocket/client_manager.h
    websocket/client_manager.cpp
)

set(SOURCES          # ${LOCAL_SOCKET}
    ${WEBSOKETS}
    # sop.h
    # sop.cpp

    daemon/client_d.h
    #daemon/client_d.cpp

    daemon/daemon_process.h
    # daemon/daemon_process.cpp



    # dbus_register.h
    # dbus_register.cpp

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

#set(SOURCES ${SOURCES}
#    # "${PROJECT_SOURCE_DIR}/js/index.html"
#    "${PROJECT_SOURCE_DIR}/js/testbase.js"
#    "${PROJECT_SOURCE_DIR}/js/testcase.js"
#    "${PROJECT_SOURCE_DIR}/js/qwebchannel.js"
#    "${PROJECT_SOURCE_DIR}/js/testapi.js"
#)

set(SOURCES ${SOURCES}
    js/javascript.qrc
)

add_definitions(
    # -DINDEX_HTML="${PROJECT_SOURCE_DIR}/js/index.html"
    -DTESTBASE_JS=":/javascript/testbase.js"
    -DTESTCASE_JS=":/javascript/testcase.js"       # 改成动态加载，用户手动指定比较好
    -DCHANNEL_JS=":/javascript/qwebchannel.js"
    -DTESTAPI_JS=":/javascript/testapi.js"
    -DINJECTOR_DLL="${CMAKE_BINARY_DIR}/libinjector.so"
)

set(INSTANCE
    QSingleInstance/clientinstance.cpp
    QSingleInstance/clientinstance.h
    QSingleInstance/qsingleinstance.cpp
    QSingleInstance/qsingleinstance.h
    QSingleInstance/qsingleinstance_p.cpp
    QSingleInstance/qsingleinstance_p.h
)

add_library(injector SHARED ${SOURCES} ${INSTANCE})
set_target_properties(injector PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(injector ${QT_LIBS})

add_executable(injector_test_demo main.cpp ${SOURCES} ${INSTANCE})
target_link_libraries(injector_test_demo ${QT_LIBS})



add_executable(daemon
    daemon/main.cpp
    daemon/client_d.h
    daemon/client_d.cpp
    daemon/daemon_process.h
    daemon/daemon_process.cpp
    ${INSTANCE}
    ${WEBSOKETS}
    # ${LOCAL_SOCKET}
    gdbinjector/gdb_injector.h
    #gdbinjector/gdb_injector.cpp

)
target_link_libraries(daemon Qt5::Core Qt5::DBus Qt5::Network Qt5::WebSockets)

