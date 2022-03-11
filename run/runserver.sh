#!/bin/bash

# pkill deepin-movie #dde-control-cen
# PROGRAM=/usr/bin/deepin-movie #dde-control-center  # /home/alex/Desktop/gamademo/auto/injector_test_demo
# ${PROGRAM} &
cd .. && cmake CMakeLists.txt && make -j8 && cd - || exit 1

pkill dde-control-cen
PROGRAM=/usr/bin/dde-control-center  # /home/alex/Desktop/gamademo/auto/injector_test_demo
${PROGRAM} -s &

# PID=$!
PID=$(pidof ${PROGRAM})
echo "run demo in background，pid:  ${PID}"
# GAMA_SO_LIB=/home/alex/Desktop/gamademo/auto/injector_test_demo/injector.so

# gdb -p $PID -batch                             \
# -ex "set logging on"                                \
# -ex "set confirm off"                               \
# -ex "sha dl"                                        \
# -ex "sha ${GAMA_SO_LIB}"                            \
# -ex "call (void) dlopen(\"${GAMA_SO_LIB}\", 2)"     \
# -ex "call (void) gammaray_probe_attach()"

# -ex detach                                
# -ex quit
# -ex continue                                      
sleep 1
gdb -p ${PID} -batch -x ./cmd.gdb
# gdb -ex=r ${PROGRAM}
# gdb attach ${PID}



# probecreator.cpp
# hooks.cpp
# preload.cpp

# probe.cpp
# 查找对象的入口：
# void Probe::createProbe(bool findExisting)
# void Probe::findExistingObjects()
# 加入到：m_validObjects
# 添加对象指针的地方：
# m_toolManager->objectAdded(obj);

# 执行信号操的入口：
# Probe::executeSignalCallback

# Probe::delayedInit() 进行了服务端的初始化

# metaobjectrepository.cpp  是解析元对象的地方，调用是在这里：void WidgetInspectorServer::registerWidgetMetaTypes()

# probe 的instance可以获取到 MetaObjectRegistry *metaObjectRegistry()



# selectTools看这里：bool WidgetInspectorServer::eventFilter(QObject *object, QEvent *event)

# 拿到Probe::instance后是否可以创建窗口？
#    GammaRay::Probe::instance()->setWindow(window);
#    GammaRay::Probe::instance()->setParent(window);


# 无法附加的问题：https://github.com/KDAB/GammaRay/wiki/Known-Issues

# 附加到客户程序的示例：void Probe::showInProcessUi()


# void MethodsExtension::activateMethod()

# server端是这样选中对象的：
# void WidgetInspectorServer::widgetSelected(QWidget *widget)


# 只有这里的fromCtor是true：
# extern "C" Q_DECL_EXPORT void gammaray_addObject(QObject *obj) {
#    Probe::objectAdded(obj, true);


# 在这两个地方安装的hook：
# gammaray_probe_inject
# gammaray_install_hooks

# toolmanager里面有这个添加tool的例子：
# emit toolEnabled(factory->id());

