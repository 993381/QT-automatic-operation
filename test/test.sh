#!/bin/bash
DAEMON_CLIENT=/home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon
CONTROL_CENTER=/usr/bin/control-center

function 控制中心::启动初始化 {
    # kill -9 `pidof ${DAEMON_CLIENT} dde-control-center`
    kill -9 `pidof dde-control-center`
    ${DAEMON_CLIENT} -l dde-control-center -- -s
    sleep 2
}

function 控制中心::一二级菜单 {
    ${DAEMON_CLIENT} -f `pwd`/auto_click.js
}

function 控制中心::修改密码 {
    ${DAEMON_CLIENT} -c "设置速度('慢')"
    ${DAEMON_CLIENT} -c "选择('帐户')"
    ${DAEMON_CLIENT} -c "点击('修改密码')"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',1)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',2)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',3)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',4)"
    ${DAEMON_CLIENT} -c "点击('取消')"
}

function 控制中心::创建账户 {
    ${DAEMON_CLIENT} -f `pwd`/create_account.js
    sleep 2      # 等待dbus通信，pkexec完全弹出鉴权窗口
    ${DAEMON_CLIENT} -j `pidof dde-polkit-agent`
    sleep 2      # 等待注入成功，程序登陆服务器
    ${DAEMON_CLIENT} -f `pwd`/confirm.js
}

function 控制中心::查询账户 {
    ${DAEMON_CLIENT} -j `pidof ${CONTROL_CENTER}`
    sleep 1
    ${DAEMON_CLIENT} -c "选择('帐户')"
    sleep 1
    ${DAEMON_CLIENT} -c "选择('demostrate')" && return 0 || return 1
}

function 控制中心::删除账户 {
    ${DAEMON_CLIENT} -j `pidof ${CONTROL_CENTER}`
    sleep 1
    ${DAEMON_CLIENT} -c "选择('帐户')"
    sleep 1
    ${DAEMON_CLIENT} -c "选择('demostrate')"
    sleep 1
    ${DAEMON_CLIENT} -c "点击('删除帐户')"
    sleep 1
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', '删除')"
    sleep 2
    ${DAEMON_CLIENT} -j `pidof dde-polkit-agent`
    sleep 2
    ${DAEMON_CLIENT} -c "输入('a')"
    ${DAEMON_CLIENT} -c "点击('确 定')"
}

function 锁屏界面::自动解锁 {
    # 锁屏
    /usr/bin/setxkbmap -option grab:break_actions&&/usr/bin/xdotool key XF86Ungrab&&dbus-send --print-reply --dest=com.deepin.dde.lockFront /com/deepin/dde/lockFront com.deepin.dde.lockFront.Show
    # 注入
    ${DAEMON_CLIENT} -j `pidof dde-lock`
    sleep 1
    ${DAEMON_CLIENT} -c "输入('a')"
    sleep 1
    ${DAEMON_CLIENT} -c "点击图形按钮(1)"
    sleep 1
}

function 启动器::全屏切换 {
    kill -9 `pidof ${DAEMON_CLIENT}` `pidof dde-launcher`
    sleep 3
    ${DAEMON_CLIENT} -l dde-launcher -- -s
    sleep 1
    ${DAEMON_CLIENT} -c "选择('计算器')"
    sleep 1
    dde-launcher -s
    sleep 1
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', 'modeToggleBtn')"
    sleep 1
    ${DAEMON_CLIENT} -c "选择('音乐')"
    sleep 1
    dde-launcher -s
    sleep 1
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', 'Btn-ToggleMode')"
    sleep 1
    kill -9 `pidof deepin-music deepin-calculator`
}

function 启动器::点击左侧工具 {
    kill -9 `pidof ${DAEMON_CLIENT}` `pidof dde-launcher`
    sleep 3
    ${DAEMON_CLIENT} -l dde-launcher -- -s
    sleep 1
    ${DAEMON_CLIENT} -f `pwd`/launcher_1.js
    sleep 1
    kill -9 `pidof dde-file-manager dde-control-center`
}

function 测试应用启动状态 {
    # dde-daemon 搜索 print-reply 就行
    dbus-send --print-reply --dest=com.deepin.dde.osd /org/freedesktop/Notifications com.deepin.dde.Notification.Toggle
    dbus-send --print-reply --dest=com.deepin.dde.Launcher /com/deepin/dde/Launcher com.deepin.dde.Launcher.Toggle
    dbus-send --print-reply --dest=com.deepin.dde.Clipboard /com/deepin/dde/Clipboard com.deepin.dde.Clipboard.Toggle
    # logout
    dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Show
    # 锁屏
    /usr/bin/setxkbmap -option grab:break_actions&&/usr/bin/xdotool key XF86Ungrab&&dbus-send --print-reply --dest=com.deepin.dde.lockFront /com/deepin/dde/lockFront com.deepin.dde.lockFront.Show
    # 截屏
    dbus-send --print-reply --dest=com.deepin.ScreenRecorder /com/deepin/ScreenRecorder com.deepin.ScreenRecorder.stopRecord
    # 控制中心
    dbus-send --session --dest=com.deepin.dde.ControlCenter  --print-reply /com/deepin/dde/ControlCenter com.deepin.dde.ControlCenter.Show

    # 然后再查询 pid 或者注入即可
    pidof /usr/bin/dde-clipboard
}

# function AppLaunchStateTest { }

# TestAutoUnlock 

# TestDDELLauncherSwitch
# TestDDELLauncherTool

# TestControlCenter && TestCreateAccount 
# TestControlCenter && TestRemoveAccount

# 查询窗口标题：
# wmctrl -l -p


# 控制中心::启动初始化 
# 控制中心::一二级菜单
# 控制中心::修改密码

# 控制中心::创建账户

控制中心::启动初始化
if 控制中心::查询账户 
then
    控制中心::删除账户
else
    控制中心::创建账户
fi
