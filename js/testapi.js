// 该文件对外提供中英文的 API

//!TODO add setEnvironment function

var count = 0;
var inteval = 500;
var sleeptime = 0;

// 每次执行脚本前要resetTimer, 重新设置定时器才能马上执行
function resetTimer() {
    count = 0;
    inteval = 500;
    sleeptime = 0;
}

function getSleepTime() {
    sleeptime = inteval*count;
    count++;
    return sleeptime;
}
function setSleepTime(time) {
    sleeptime = time;
}

function 选择(item) {
    timedExec(getSleepTime(), function () {
        TestMethod.selectListItem(item);
    });
}

function 点击(btnText) {
    timedExec(getSleepTime(), function () {
        TestMethod.clickButtonByText(btnText);
    });
}

function 输入() {
    var args = []; // new Array();
    for (var i = 0; i < arguments.length; i++) {
        args[i] = arguments[i];
    }
    timedExec(getSleepTime(), function() {
        TestMethod.setLineEditText(args);
    });
}
function 等待(time) {
    setSleepTime(getSleepTime() + time * 1000 - inteval);
}

function 失败后立即停止(flag) {
}
function 返回主界面() {
}
function 重复执行() {
}

// 默认值500,可以不设置
function 设置速度(speed) {
    if (speed === '快') {
        inteval = 500;
        return
    }
    if (speed === '慢') {
        inteval = 1500;
        return
    }
    if (speed === '中') {
        inteval = 1000;
        return
    }
    // setError(true);
}
