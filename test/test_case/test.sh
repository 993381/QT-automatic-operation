#!/bin/bash
#set -e

export INJECTOR_PATH=`pwd`/libinjector.so
TEST_CLIENT=`pwd`/test-cli
CONTROL_CENTER=/usr/bin/dde-control-center

# root账户修改密码，改完再改回去
ROOT_PASSWD='a'
ROOT_NEW_PASSWD='b'

function 控制中心::启动初始化 {
    kill -9 `pidof dde-control-center`
    ${TEST_CLIENT} -l dde-control-center -- -s
    sleep 2
}

function 控制中心::一二级菜单 {
    ${TEST_CLIENT} -f `pwd`/auto_click.js
}

function 控制中心::修改密码 {
    local PSWD_OLD=$1
    local PSWD_NEW=$2
    sleep 1
    ${TEST_CLIENT} -c "选择('帐户')"
    sleep 1
    ${TEST_CLIENT} -c "点击('修改密码')"
    sleep 1
    ${TEST_CLIENT} -c "输入('${PSWD_OLD}',1)"
    sleep 1
    ${TEST_CLIENT} -c "输入('${PSWD_NEW}',2)"
    sleep 1
    ${TEST_CLIENT} -c "输入('${PSWD_NEW}',3)"
    sleep 1
    ${TEST_CLIENT} -c "输入('tips', 4)"
    sleep 1
    ${TEST_CLIENT} -c "点击('保存')"
    sleep 1
    # 验证结果
    if ${TEST_CLIENT} -c "点击('保存')"
    then
        return 1
    fi
    return 0
}

function 控制中心::重设密码 {
    sleep 2
    ${TEST_CLIENT} -j `pidof ${CONTROL_CENTER}`
    sleep 2

    local PSWD_NEW=$1
    sleep 1
    ${TEST_CLIENT} -c "选择('帐户')"
    sleep 1
    ${TEST_CLIENT} -c "选择('demostrate')"
    sleep 1
    ${TEST_CLIENT} -c "点击('重设密码')"
    sleep 1
    ${TEST_CLIENT} -c "输入('${PSWD_NEW}',1)"
    sleep 1
    ${TEST_CLIENT} -c "输入('${PSWD_NEW}',2)"
    sleep 1
    # ${TEST_CLIENT} -c "输入('xxxxxxx',3)"
    ${TEST_CLIENT} -c "点击('保存')"
    sleep 1
    # 验证结果
    if ${TEST_CLIENT} -c "点击('保存')"  
    then
        return 1
    fi
    return 0
}

function 控制中心::创建账户 {
    ${TEST_CLIENT} -f `pwd`/create_account.js

    sleep 2      # 等待dbus通信，pkexec完全弹出鉴权窗口
    ${TEST_CLIENT} -j `pidof dde-polkit-agent`
    sleep 2      # 等待注入成功，App 登陆服务端

    # 这里要输入密码所以不要放在 js 脚本中
    ${TEST_CLIENT} -c "输入('${ROOT_PASSWD}')"
    ${TEST_CLIENT} -c "点击('确 定')"
}

function 控制中心::查询账户 {
    ${TEST_CLIENT} -j `pidof ${CONTROL_CENTER}`
    sleep 1
    ${TEST_CLIENT} -c "选择('帐户')"
    sleep 1
    ${TEST_CLIENT} -c "选择('demostrate')" && return 0 || return 1
}

function 控制中心::删除账户 {
    ${TEST_CLIENT} -j `pidof ${CONTROL_CENTER}`
    sleep 1
    ${TEST_CLIENT} -c "选择('帐户')"
    sleep 1
    ${TEST_CLIENT} -c "选择('demostrate')"
    sleep 1
    ${TEST_CLIENT} -c "点击('删除帐户')"
    sleep 1
    ${TEST_CLIENT} -c "点击按钮('byAccName', '删除')"
    sleep 2

    if 控制中心::查询账户
    then
        ${TEST_CLIENT} -j `pidof dde-polkit-agent`
        sleep 2
        ${TEST_CLIENT} -c "输入('${ROOT_PASSWD}')"
        ${TEST_CLIENT} -c "点击('确 定')"
    fi
}

function 锁屏界面::自动解锁 {
    # 锁屏
    /usr/bin/setxkbmap -option grab:break_actions&&/usr/bin/xdotool key XF86Ungrab&&dbus-send --print-reply --dest=com.deepin.dde.lockFront /com/deepin/dde/lockFront com.deepin.dde.lockFront.Show
    # 注入
    ${TEST_CLIENT} -j `pidof dde-lock`
    sleep 1
    ${TEST_CLIENT} -c "输入('${ROOT_PASSWD}')" || return 1
    sleep 1
    ${TEST_CLIENT} -c "点击图形按钮(1)"
    sleep 1
}

function 启动器::全屏切换 {
    kill -9 `pidof  /usr/bin/dde-launcher`
    sleep 3
    ${TEST_CLIENT} -l dde-launcher -- -s
    sleep 1
    ${TEST_CLIENT} -c "选择('计算器')"
    sleep 1
    dde-launcher -s
    sleep 1
    ${TEST_CLIENT} -c "点击按钮('byAccName', 'modeToggleBtn')"
    sleep 1
    ${TEST_CLIENT} -c "选择('音乐')"
    sleep 1
    dde-launcher -s
    sleep 1
    ${TEST_CLIENT} -c "点击按钮('byAccName', 'Btn-ToggleMode')"
    sleep 1
    kill -9 `pidof deepin-music deepin-calculator`
}

function 启动器::点击左侧工具 {
    kill -9 `pidof ${TEST_CLIENT}` `pidof dde-launcher`
    sleep 3
    ${TEST_CLIENT} -l dde-launcher -- -s
    sleep 2
    ${TEST_CLIENT} -f `pwd`/launcher.js
    sleep 1
    kill -9 `pidof dde-file-manager dde-control-center`
}

# 应用是否启动成功可以用 pid 以及窗口查询
# 或者用命令行 test-cli -j pid 查询注入状态，能成功注入或者在线就证明应用启动是OK的,用-l 参数能成功启动也是OK的
# 查询窗口标题：
# wmctrl -l -p

控制中心::启动初始化 && 控制中心::一二级菜单

控制中心::启动初始化
# 改完密码后再改回来
控制中心::修改密码 ${ROOT_PASSWD} ${ROOT_NEW_PASSWD} && 控制中心::修改密码 ${ROOT_NEW_PASSWD} ${ROOT_PASSWD}
if 控制中心::查询账户 
then
    控制中心::删除账户
else
    控制中心::创建账户 && 控制中心::重设密码 passwd2 && 控制中心::删除账户 && echo "ALL FINISHED, SUCCESS"
    killall dde-control-center && sleep 1
fi

启动器::全屏切换

# kill 掉 dde-lock 的 logout 界面后就会进入锁屏解密，直接解锁
启动器::点击左侧工具 && killall dde-lock && sleep 1 && 锁屏界面::自动解锁

