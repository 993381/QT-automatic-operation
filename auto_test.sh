#!/bin/bash

kill -9 `pidof dde-control-center` `pidof /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon`
/home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -l "dde-control-center" -- -s
sleep 3

LOOP_COUNT=3
[[ -n $1 ]] && LOOP_COUNT=$1
SLEEP_TIME=1
for index in `seq ${LOOP_COUNT}`
do
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('帐户')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "点击('修改密码')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "输入('xxxxxxx',1)"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "输入('xxxxxxx',2)"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "输入('xxxxxxx',3)"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "输入('xxxxxxx',4)"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "点击('取消')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('Union ID')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('显示')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('默认程序')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('个性化')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('网络')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('通知')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('声音')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('时间日期')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('电源管理')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('鼠标')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('键盘和语言')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('辅助功能')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('更新')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('系统信息')"
    sleep ${SLEEP_TIME}
    /home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon -c "选择('通用')"
    sleep ${SLEEP_TIME}
done
