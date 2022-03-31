#!/bin/bash

# 导入动态库路径
export INJECTOR_PATH=`pwd`/libinjector.so

# 用 -l 命令启动控制中心，-- 后面的参数会传给控制中心
./test-cli -l dde-control-center -- -s

# 等启动完毕再执行测试
sleep 1

# 按行读取已经写好的脚本，调用 ./test-cli -c "xxxx"，一句一句执行，根据返回值获取执行结果并记录到文件
while read LINE
do
    if [[ ! "${LINE}" =~ ^[[:space:]]{0,}$ ]]    # 排除空行
    then
        sleep 0.5
        ./test-cli -c "${LINE}" && echo "SUCCESS    ${LINE}" >> result.log || echo "FAILED     ${LINE}" >> result.log
    fi
done < ./test.js

