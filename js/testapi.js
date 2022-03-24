// 该文件对外提供中英文的 API

//!TODO add setEnvironment function

//!TODO: var testConfig = new Object

var count = 0;
var inteval = 500;
var sleeptime = 0;
var useTimer = true; // 提供不用定时立即执行的选项
var globalResult = true;


// 异步执行可以给个 id 和 cpp 交互，获取每次执行的结果
// 每次执行脚本前要resetTimer, 重新设置定时器才能马上执行
function resetTimer() {  // resetConfig
    count = 0;
    inteval = 500;
    sleeptime = 0;
    useTimer = true;
    globalResult = true;
}

function getSleepTime() {
    sleeptime = inteval*count;
    count++;
    return sleeptime;
}
function setSleepTime(time) {
    sleeptime = time;
}
function updateResult(res) {
    globalResult = globalResult && res;
}

// func 统一为全部都有返回值，都为 bool 类型
function funcTimedCaller(func, args) {
    if (!useTimer) {
        func(args, function (result) { updateResult(result) });
        return;
    }
    timedExec(getSleepTime(), function () {
        func(args, function (result) { updateResult(result) });
    });
}

function execFinished() {
    var func = TestMethod.jsExecFinished;
    if (!useTimer) {
        func(globalResult);
        return;
    }
    timedExec(getSleepTime(), function () {
        func(globalResult);
    });
}

function 选择(item) {
    funcTimedCaller(TestMethod.selectListItem, item);
}

function 点击(btnText) {
    funcTimedCaller(TestMethod.clickButtonByText, btnText);
}

function 输入() {
    var args = []; // 多参数的存到数组中，对应于 QStringList
    for (var i = 0; i < arguments.length; i++) {
        args[i] = arguments[i];
    }
    funcTimedCaller(TestMethod.setLineEditText, args);
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

function 执行结束() {
    TestMethod.jsExecFinished();
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
