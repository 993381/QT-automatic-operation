// 用于封装 QWebChannel 并初始化基础的函数接口
var TestMethod = {};                            // 用来支持动态添加成员函数/对象
var globalTester = null;
if (!globalTester) {
    globalTester = new QWebChannel(qt.webChannelTransport, function (channel) {
        // 获取 Qt 通道类
        TestMethod = channel.objects.tester;
        // 接收 Qt 发来的数据
        TestMethod.Qt2JsMessage.connect(function (msg) {
            console.log("Qt2JsMessage xxxxxxxxxxxxx " + msg);
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

// 获取被调用函数在js中的代码行数
// https://www.jb51.net/article/152635.htm
// 可以根据读取的脚本计算行数
function getCallerFileNameAndLine(){
    function getException() {
        try {
            throw Error('');
        } catch (err) {
            return err;
        }
    }

    const err = getException();

    const stack = err.stack;    // 直接alert(stack) 就能看到了
    const stackArr = stack.split('\n');
    let callerLogIndex = 0;
    for (let i = 0; i < stackArr.length; i++) {
        if (stackArr[i].indexOf('Map.Logger') > 0 && i + 1 < stackArr.length) {
            callerLogIndex = i + 1;
            break;
        }
    }

    if (callerLogIndex !== 0) {
        const callerStackLine = stackArr[callerLogIndex];
        return `[${callerStackLine.substring(callerStackLine.lastIndexOf(path.sep) + 1, callerStackLine.lastIndexOf(':'))}]`;
    } else {
        return '[-]';
    }
}
