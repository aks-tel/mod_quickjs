//
// requires module version v1.8.0
//
consoleLog('info', "****** JsonRPC tets *******");

var jrpc = new JsonRPC("http://127.0.0.1:8181/rpc/", "secret");

consoleLog('info', "jrpc.url...............: " + jrpc.url);
consoleLog('info', "jrpc.proxy.............: " + jrpc.proxy);
consoleLog('info', "jrpc.requestTimeout....: " + jrpc.requestTimeout);
consoleLog('info', "jrpc.connectTimeout....: " + jrpc.connectTimeout);
consoleLog('info', "jrpc.credentials.......: " + jrpc.credentials);
consoleLog('info', "jrpc.enableException...: " + jrpc.enableExceptions);

jrpc.enableExceptions = false;

consoleLog('info', "\n\n-------- tests start ----------");

var res;
var it = 0;
while(true) {

    consoleLog('info', "--------> call::test1() / " + it);
    res = jrpc.perform('NLPService', 'test1', ['en', 'test-model', 'my names is alex']);
    if(jrpc.isError(res)) consoleLog('info', "TEST1-ERROR: origin=" + res.origin + ", code=" + res.code + ", message=" + res.message);
    else consoleLog('info', "TEST1-TESULT: [" + res + "], JSON=" + JSON.stringify(res));

    msleep(1000);
    consoleLog('info', "--------> call::test2() / " + it);
    res = jrpc.perform('NLPService', 'test2', ['en', 'test-model', 'test string 1 2 3 4']);
    if(jrpc.isError(res)) consoleLog('info', "TEST2-ERROR: origin=" + res.origin + ", code=" + res.code + ", message=" + res.message);
    else consoleLog('info', "TEST2-TESULT: [" + res + "], JSON=" + JSON.stringify(res));

    msleep(1000);
    consoleLog('info', "--------> call::test3() / " + it);
    res = jrpc.perform('NLPService', 'test3', ['en', 'test-model', 'test string 333 444']);
    if(jrpc.isError(res)) consoleLog('info', "TEST3-ERROR: origin=" + res.origin + ", code=" + res.code + ", message=" + res.message);
    else consoleLog('info', "TEST3-TESULT: [" + res + "], JSON=" + JSON.stringify(res));

    consoleLog('info', "--------------------------------------------------------------------------------------------------------------\n");

    msleep(1000);
    if(++it > 100) break;
}


consoleLog('info', "----------------- tests done ------------------");

