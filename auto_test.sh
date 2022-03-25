#!/bin/bash
export INJECTOR_PATH=/home/alex/Desktop/gamademo/build-auto-unknown-Default/libinjector.so
DAEMON_CLIENT=/home/alex/Desktop/gamademo/build-auto-unknown-Default/daemon

for i in `seq 5`
do
    PID_LIST="`pidof dde-control-center` `pidof ${DAEMON_CLIENT}`"
    [[ ! "${PID_LIST}" =~ ^[[:space:]]{0,}$ ]] && kill -9 ${PID_LIST} || break
    sleep 0.5
done 

${DAEMON_CLIENT} -l "dde-control-center" -- -s || exit 1
sleep 1

LOOP_COUNT=1
[[ -n $1 ]] && LOOP_COUNT=$1
SLEEP_TIME=0.8

function logRecord {
    echo -e "$*" >> test-result.log
}

for index in `seq ${LOOP_COUNT}`
do
    while read LINE
    do
        if [[ ! "${LINE}" =~ ^[[:space:]]{0,}$ ]] 
        then
            sleep ${SLEEP_TIME}; 
            ${DAEMON_CLIENT} -c "${LINE}" && logRecord "SUCCESS    ${LINE}" || logRecord "FAILED     ${LINE}"
        fi
    done < test.js
done
