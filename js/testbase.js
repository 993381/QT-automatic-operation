// 用于封装 QWebChannel 并初始化基础的函数接口
var TestMethod = new Object();                  // 用来支持动态添加成员函数/对象
var globalTester = null;
if (!globalTester) {
    globalTester = new QWebChannel(qt.webChannelTransport, function (channel) {
        // 获取 Qt 通道类
        TestMethod = channel.objects.tester;
        // 接收 Qt 发来的数据
        TestMethod.Qt2JsMessage.connect(function (msg) {
            console.log("xxxxxxxxxxxxx 3 " + msg);
        });
        // 调用 Qt 函数
        console.log("QWebChannel init finished.");
        TestMethod.jsLoadFinished();            // 通知 Cpp 端 js 加载完成
    });
}

// 使用定时器支持定时调用，达到每一步执行都有延时的效果
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
