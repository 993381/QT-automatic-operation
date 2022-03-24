#!/bin/bash

DAEMON_CLIENT=/home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon

PID_LIST="`pidof dde-control-center` `pidof ${DAEMON_CLIENT}`"
[[ ! "${PID_LIST}" =~ ^[[:space:]]{0,}$ ]] && kill -9 ${PID_LIST}
sleep 1
PID_LIST="`pidof dde-control-center` `pidof ${DAEMON_CLIENT}`"
[[ ! "${PID_LIST}" =~ ^[[:space:]]{0,}$ ]] && kill -9 ${PID_LIST}
sleep 1

${DAEMON_CLIENT} -l "dde-control-center" -- -s || exit 1

LOOP_COUNT=1
[[ -n $1 ]] && LOOP_COUNT=$1
SLEEP_TIME=1
for index in `seq ${LOOP_COUNT}`
do
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('生物认证')" || exit 1
    
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('指纹')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('人脸')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('虹膜')"
    
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('帐户')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "点击('修改密码')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "输入('xxxxxxx',1)"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "输入('xxxxxxx',2)"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "输入('xxxxxxx',3)"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "输入('xxxxxxx',4)"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "点击('取消')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('Union ID')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('显示')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('默认程序')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('网页')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('邮件')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('文本')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('音乐')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('视频')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('图片')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('终端')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('个性化')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通用')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('图标主题')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('光标主题')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('字体')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('任务栏')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('网络')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('网络详情')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('应用代理')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('系统代理')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('VPN')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('DSL')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('有线网络')"


    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通知')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('声音')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('设备管理')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('系统音效')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('输入')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('输出')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('时间日期')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('时区列表')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('时间设置')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('格式设置')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('电源管理')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('使用电源')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通用')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('鼠标')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('鼠标', 1)"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通用')"
    # exit 0

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('键盘和语言')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通用')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('键盘布局')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('输入法')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('系统语言')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('快捷键')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('文本翻译')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('语音朗读')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('语音听写')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('桌面智能助手')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('更新')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('系统信息')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('备份还原')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('隐私政策')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('最终用户许可协议')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('版本协议')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('关于本机')"

    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('通用')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('域管理')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('用户体验计划')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('开发者模式')"
    sleep ${SLEEP_TIME}; ${DAEMON_CLIENT} -c "选择('启动菜单')"
done
