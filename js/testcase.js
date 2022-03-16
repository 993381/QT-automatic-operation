//!TODO add setEnvironment function

TestMethod.launch = function() {
    TestMethod.startApp(["/usr/bin/dde-control-center", "-s"]);   // QStringList对应数组
}

function sleep(duration) {
  return new Promise(resolve => {
      setTimeout(resolve, duration);
  })
}
async function timedExec(duration, callback) {
    await sleep(duration);
    if (callback) {
        callback();
    }
}

var count = 0;
var inteval = 500;
var sleeptime = 0;
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
    var args = new Array();
    for(var i=0; i<arguments.length; i++)
    {
        args[i] = arguments[i];
    }
    timedExec(getSleepTime(), function () {
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
function 设置速度(speed) {
    if (speed === '快') {
        inteval = 500;
    }
    if (speed === '慢') {
        inteval = 1500;
    }
    if (speed === '中') {
        inteval = 1000;
    }
}

TestMethod.startTest = function() {
    失败后立即停止(1)

    设置速度('中');

    选择('帐户')
    点击('修改密码')
    输入('a', 1)
    等待(3)
    输入('passwd', 2)
    输入('passwd', 3)
    输入('passwd', 4)
    点击('取消')

    失败后立即停止(0)

    返回主界面()

    重复执行(100)

    return

//    await timedExec(1000, function () {
//        TestMethod.clickButtonByText("修改密码", function (res) {
//            console.log("result2: " + res);
//        });
//    });
//    await timedExec(1000, function () {
//        TestMethod.setLineEditText(["passwd1", 1], function (res) {
//            console.log("result3: " + res);
//        });
//    });

//    await timedExec(1000, function () {
//        TestMethod.setLineEditText(["passwd2", 2], function (res) {
//            console.log("result4: " + res);
//        });
//    });
//    await timedExec(1000, function () {
//        TestMethod.setLineEditText(["passwd3", 3], function (res) {
//            console.log("result5: " + res);
//        });
//    });
//    await timedExec(1000, () => {
//        TestMethod.setLineEditText(["passwd", 4], function (res) {
//            console.log("result5: " + res);
//        });
//    });

//    await timedExec(1000, function () {
//        TestMethod.clickButtonByText("取消", function (res) {
//            console.log("result2: " + res);
//        });
//    });

//    Uia.TestMethod.selectListItem(["默认程序", 0], function(msg) {
//        console.log("result1: " + msg);
//    });
//    Uia.TestMethod.selectListItem(["视频", 0]);
//    Uia.TestMethod.selectListItem(["相册", 0]);
//    Uia.TestMethod.closeApp();
}
