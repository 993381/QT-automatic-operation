//!TODO add setEnvironment function

Uia.startApp = function() {
    Uia.TestMethod.startApp(["/usr/bin/dde-control-center", "-s"]);   // QStringList对应数组
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
async function main() {
    while (true) {
        await timedExec(2000, function () {
            console.log("exec timeout 2000");
        });
        await timedExec(1000, function () {
            console.log("exec timeout 1000");
        });
        await timedExec(3000, function () {
            console.log("exec timeout 3000");
        });
    }
}

Uia.startTest = async function() {
//    main();
//    return;
    await timedExec(1000, function () {
        Uia.TestMethod.selectListItem(["帐户", 0], function (res) {
            console.log("result1: " + res);
        });
    });
    await timedExec(1000, function () {
        Uia.TestMethod.clickButtonByText("修改密码", function (res) {
            console.log("result2: " + res);
        });
    });
    await timedExec(1000, function () {
        Uia.TestMethod.setLineEditText(["passwd1", 1], function (res) {
            console.log("result3: " + res);
        });
    });

    await timedExec(1000, function () {
        Uia.TestMethod.setLineEditText(["passwd2", 2], function (res) {
            console.log("result4: " + res);
        });
    });
    await timedExec(1000, function () {
        Uia.TestMethod.setLineEditText(["passwd3", 3], function (res) {
            console.log("result5: " + res);
        });
    });
    await timedExec(1000, function () {
        Uia.TestMethod.setLineEditText(["passwd", 4], function (res) {
            console.log("result5: " + res);
        });
    });

    await timedExec(1000, function () {
        Uia.TestMethod.clickButtonByText("取消", function (res) {
            console.log("result2: " + res);
        });
    });

//    Uia.TestMethod.selectListItem(["默认程序", 0], function(msg) {
//        console.log("result1: " + msg);
//    });
//    Uia.TestMethod.selectListItem(["视频", 0]);
//    Uia.TestMethod.selectListItem(["相册", 0]);
//    Uia.TestMethod.closeApp();
}
