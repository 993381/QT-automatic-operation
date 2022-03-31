#!/bin/bash

export INJECTOR_PATH=/home/alex/Desktop/gamademo/build-auto-unknown-Default/libinjector.so
TEST_CLIENT=/home/alex/Desktop/gamademo/build-auto-unknown-Default/test-cli

# 清除客户端进程，为确保能够正常启动反复确认
for i in `seq 5`
do
    PID_LIST="`pidof dde-control-center` `pidof ${TEST_CLIENT}`"
    [[ ! "${PID_LIST}" =~ ^[[:space:]]{0,}$ ]] && kill -9 ${PID_LIST} || break
    sleep 0.5
done

${TEST_CLIENT} -l "dde-control-center" -- -s || exit 1

# 启动成功后要稍微等待，等待控制中心初始化完成
sleep 1

LOOP_COUNT=1
[[ -n $1 ]] && LOOP_COUNT=$1
SLEEP_TIME=0.8

# 将成功、失败的结果分别显示到终端并记录到文件
function logRecordP {
    echo -e "\033[42;30mPASS\033[0m      $*"
    echo "PASS    $*" >> test-result.log
}
function logRecordF {
    echo -e "\e[41;30mFAILED\e[0m    $*"
    echo "FAILED  $*" >> test-result.log
}

# 从 js 文件 auto_click.js 按行读取命令并执行
for index in `seq ${LOOP_COUNT}`
do
    while read LINE
    do
        # 排除空行和注释
        [[ "${LINE}" =~ ^[[:space:]]{0,}//.*$ ]] && continue
        if [[ ! "${LINE}" =~ ^[[:space:]]{0,}$ ]]
        then
            sleep ${SLEEP_TIME};
            # 执行测试命令
            ${TEST_CLIENT} -c "${LINE}" && logRecordP "${LINE}" || logRecordF "${LINE}"
        fi
    done < ./auto_click.js
done
