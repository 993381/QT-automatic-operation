#!/bin/bash
DAEMON_CLIENT=/home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon

function TestSellectItem {
    ${DAEMON_CLIENT} -c "选择('生物认证')"
    ${DAEMON_CLIENT} -c "选择('虹膜')"
    ${DAEMON_CLIENT} -c "选择('人脸')"
    ${DAEMON_CLIENT} -c "选择('指纹')"
}

function TestModifyPasswd {
    ${DAEMON_CLIENT} -c "设置速度('慢')"
    ${DAEMON_CLIENT} -c "选择('帐户')"
    ${DAEMON_CLIENT} -c "点击('修改密码')"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',1)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',2)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',3)"
    ${DAEMON_CLIENT} -c "输入('xxxxxxx',4)"
    ${DAEMON_CLIENT} -c "点击('取消')"
}

function TestCreateAccount {
    ${DAEMON_CLIENT} -f `pwd`/create_account.js
    ${DAEMON_CLIENT} -j `pidof dde-polkit-agent`
    sleep 1
    ${DAEMON_CLIENT} -f `pwd`/confirm.js
}

function TestRemoveAccount {
    ${DAEMON_CLIENT} -j `pidof dde-control-center`
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

function TestControlCenter {
    kill -9 `pidof ${DAEMON_CLIENT}` `pidof dde-control-center`
    ${DAEMON_CLIENT} -l dde-control-center -- -s
    sleep 1


    return 0
    ${DAEMON_CLIENT} -j `pidof dde-control-center`
    ${DAEMON_CLIENT} -c "选择('帐户')"
    ${DAEMON_CLIENT} -c "选择('demostrate')"
    ${DAEMON_CLIENT} -c '点击("删除帐户")'
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', '删除')"
    ${DAEMON_CLIENT} -j `pidof dde-polkit-agent`
    ${DAEMON_CLIENT} -f `pwd`/2.创建账户确认.js
    
    # ${DAEMON_CLIENT} -j `pidof dde-control-center`
    # ${DAEMON_CLIENT} -c "选择('帐户')"
    # ${DAEMON_CLIENT} -c "选择('demostrate')"
    # ${DAEMON_CLIENT} -c '点击("删除帐户")'
    # ${DAEMON_CLIENT} -c '点击按钮("byAccName", "取消")'
    # ${DAEMON_CLIENT} -j `pidof dde-polkit-agent`
    # ${DAEMON_CLIENT} -f `pwd`/2.创建账户确认.js
}

function TestDDELLauncher {
    kill -9 `pidof ${DAEMON_CLIENT}` `pidof dde-launcher`
    sleep 3
    ${DAEMON_CLIENT} -l dde-launcher -- -s
    sleep 1
    ${DAEMON_CLIENT} -c "选择('系统监视器')"
    sleep 1
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', 'modeToggleBtn')"
    sleep 1
    ${DAEMON_CLIENT} -c "点击按钮('byAccName', 'Btn-ToggleMode')"
    sleep 1
}

# TestDDELLauncher

TestControlCenter && TestCreateAccount 
TestControlCenter && TestRemoveAccount

