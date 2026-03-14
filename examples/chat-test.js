//
// requires module version 1.8.0 
//
consoleLog('notice', "*** CHAT TEST ***");

var chat = new Chat();
var loop = 100;

//
// chat QJS|console|scriptId|test message 123
//
console_log("notice", "script-id: " + script.id);
console_log("notice", "TEST: chat QJS|console|" +script.id+ "|test message 123");

while(loop) {
    var msg = chat.getMessage();
    if(msg) {
        console_log("notice", "MESSAGE: " + JSON.stringify(msg));
    }

    loop--;
    msleep(1000);
}
console_log("notice", "****** script-end *******");
