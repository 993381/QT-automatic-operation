// 该文件对外提供中英文的 API

//!TODO add setEnvironment function

//!TODO: var testConfig = new Object

var count = 0;
var inteval = 500;
var sleeptime = 0;
// 提供不用定时立即执行的选项
var useTimer = true;
// 单步结果默认为假
var singleResult = false;
// 多步结果默认为真
var multiResult = true;

// 记录已执行的步数
var stepCount = 0;

// 记录总的执行步数
// var totalCount = 0;

// 异步获取到的结果数量
var resultCount = 0;

// 有必要将测试结果记录下来。在cpp接口里面记录更好一点
var testResultFile = '/tmp/test_result.log';


// 提供发生错误后是立即停止还是继续执行的设置接口。应该是要全部执行的

// 异步执行可以给个 id 和 cpp 交互，获取每次执行的结果
// 每次执行脚本前要resetTimer, 重新设置定时器才能马上执行
function resetConfiguration() {
    count = 0;
    inteval = 500;
    sleeptime = 0;
    useTimer = true;

    stepCount = 0;
    // totalCount = 0;
    resultCount = 0;

    singleResult = false;
    multiResult = true;
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
    resultCount++;
    singleResult = res;
    multiResult = multiResult && res;
}

// func 统一为全部都有返回值，都为 bool 类型
function funcTimedCaller(func, args) {
    stepCount++;
    if (!useTimer) {
        func(args, function (result) { updateResult(result); });
        return;
    }
    timedExec(getSleepTime(), function () {
        func(args, function (result) { updateResult(result); });
    });
}

// 当执行到这个函数，stepCount应该等于resultCount说明执行完毕，结果全部获取完毕
function execFinished() {
    var func = TestMethod.jsExecFinished;
    if (!useTimer) {
        if (stepCount == resultCount) {
            func(singleResult);
        } else {
            // 延时递归检测是否执行完毕
            timedExec(100, function () {
                if (stepCount == resultCount) {
                    func(singleResult);
                } else {
                    execFinished();
                }
            });
        }
        return;
    }
    timedExec(getSleepTime(), function () {
        if (stepCount == resultCount) {
            func(singleResult);
        } else {
            useTimer = false; // 取消定时，去上面的分支继续检测
            execFinished();
        }
    });
}

function 选择(item) {
    var args = []; // 多参数的存到数组中，对应于 QStringList
    for (var i = 0; i < arguments.length; i++) {
        args[i] = arguments[i];
    }
    funcTimedCaller(TestMethod.selectListItem, args);
}

// 文字按钮
function 点击(btnText) {
    funcTimedCaller(TestMethod.clickButtonByText, btnText);
}
// 图形按钮
function 点击图形按钮(btnIndex) {
    funcTimedCaller(TestMethod.clickNoTextButton, btnIndex);
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
