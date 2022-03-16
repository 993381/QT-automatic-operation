var TestMethod = new Object();   // 用来动态添加成员函数/对象
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
        TestMethod.jsLoadFinished();            // 通知Cpp端js加载完成
    });
}
