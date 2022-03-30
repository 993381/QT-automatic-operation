function testCaller(func, args) {
    var res = func(args);
    TestMethod.jsExecFinished(res);
    return res;
}

// 1~2 个参数, text 和 index, index 默认为 0
function 选择() {
    var args = [];
    for (var i = 0; i < arguments.length; i++) {
        args[i] = arguments[i];
    }
    return testCaller(TestMethod.selectListItem, args);
}

// 文字按钮
function 点击(btnText) {
    return testCaller(TestMethod.clickButtonByText, btnText);
}

// 一个参数是根据索引，多个参数是 type+value+index，index 默认为 0
function 点击按扭() {
    console.log("xxxxxxxxxxxxxxxxxxx 点击按扭")
    var args = [];
    for (var i = 0; i < arguments.length; i++) {
        args[i] = arguments[i];
    }
    return testCaller(TestMethod.clickNoTextButton, args);
}

//function 执行命令(cmds) {
//    var args = [];
//    for (var i = 0; i < arguments.length; i++) {
//        args[i] = arguments[i];
//    }
//    // TODO: 处理返回值
//    TestMethod.execCmd(args);
//}

//// bin or /path/to/bin
//function 清理应用(item) {
//    // bash -c "kill - 9 $(pidof dde-control-center)"
//    var args = [];
//    var cmds = "kill -9 ";
//    for (var i = 0; i < arguments.length; i++) {
//        cmds += "$(pidof " + arguments[i] + ")";
//    }
//    args.unshift("bash", "-c", cmds);
//    TestMethod.execCmd(args);
//}

//function 输入() {
//    var args = [];
//    for (var i = 0; i < arguments.length; i++) {
//        args[i] = arguments[i];
//    }
//    return testCaller(TestMethod.setLineEditText, args);
//}

//// TODO, 使用队列定时执行, 就可以模拟等待
//function 等待(time) {
//}

//function 失败后立即停止(flag) {
//}

//function 返回主界面() {
//}

//function 重复执行() {
//}

//function 执行结束() {
//    TestMethod.jsExecFinished();
//}

//// 默认值500,可以不设置
//function 设置速度(speed) {
//    if (speed === '快') {
//        inteval = 500;
//        return
//    }
//    if (speed === '慢') {
//        inteval = 2000;
//        return
//    }
//    if (speed === '中') {
//        inteval = 1000;
//        return
//    }
//}

