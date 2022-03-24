// import "./testapi.js"

TestMethod.launch = function() {
    TestMethod.startApp(["/usr/bin/dde-control-center", "-s"]);   // QStringList对应数组
    // TestMethod.startApp("/usr/bin/deepin-calculator");   // QStringList对应数组
}

//var 执行完毕 = function() {
//    resetTimer();
//    updateResult();
//}

// 要有查找的过程
// 录制的时候要对控件的唯一性进行检查并提示
// TestMethod.startTest = function() {
    resetTimer();
    // useTimer = false;

    失败后立即停止(1)
    设置速度('快')
    选择('帐户')
    点击('修改密码')
    输入('a', 1)
    // 等待(3)
    输入('passwd', 2)
    输入('passwd', 3)
    输入('passwd', 4)
    点击('取消')

    失败后立即停止(0)

    返回主界面()

    重复执行(100)

    执行结束()

//    return
//}
