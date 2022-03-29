// import "./testapi.js"

TestMethod.launch = function() {
    // TestMethod.startApp(["/usr/bin/dde-control-center", "-s"]);
    // TestMethod.startApp("/usr/bin/deepin-calculator");
    TestMethod.startApp(["/usr/bin/dde-launcher", "-s"]);
}

//var 执行完毕 = function() {
//    resetTimer();
//    updateResult();
//}

// 要有查找的过程
// 录制的时候要对控件的唯一性进行检查并提示
TestMethod.startTest = function() {
    resetConfiguration();
    设置速度('慢')
    // 清理应用('dde-control-center', 'dde-launcher')
    清理应用('dde-control-center')
    return
    // useTimer = false;

    // 选择('帐户')
    // 点击按钮(1)

    // 点击按钮('byAccName', 'backwardbtn');
    // 选择('系统监视器');
    var buttons = [
                'computerbtn',
                'documentbtn',
                'picturebtn',
                'musicbtn',
                'videobtn',
                'downloadbtn'
            ];

    for(var idx = 0; idx < buttons.length; idx++) {
        点击按钮('byAccName', buttons[idx]);
        执行命令("/usr/bin/dde-launcher", "-s")
    }
    return;

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
}
