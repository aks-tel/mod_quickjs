<p>
    This module was originally designed to recover IVRs functionality, following the replacement of mod_spidermonkey with mod_v8 under the Freeswitch. <br>
    QuickJS was chosen due to its fast script startup, good performance, and memory efficiency, I use JavaScript for writing calls processing applications (IVR, dialers, and so on) and these criterias quite important. <br>
    Despite infrequent commits, this module is constantly involved and gained new features gradually. <br>
    I'm primarily interested in the capabilities related with processing audio/video, data operation, and ML (within keeping lightweight and performance).  <br>
</p>

## version 1.8
 - added new classes: chat, json-rpc (see: chat-test.js, json-rpc.js) <br>
 - fixed some bugs

## version 1.7
 - added configuration option 'use_std' for enabling functions from std/os modules <br>
 - added 'import' function for laoding so/js modules (see examples/dyn_module) <br> 
 - added DBH class for interaction with  freeswitch DBH <br>
 - many changes in Session (speech detection, etc) <br>
 - IVS class was removed <br>
 - odbc was removed <br>
    
## version 1.6
Was an experimental version for testing some ideas
 - added IVS class that helps to capture media streams and work with them <br>
   see: v16_echo.js (and other examples with v16_ prefix)
 - new features in the Curl class that allows to work in asynchronous mode <br>
   see: curl_async_test.js
 - new examples: <br>
    - [Stream capturing and working with chunks](https://github.com/aks-tel/mod_quickjs/blob/main/examples/v16_echo.js)
    - [Simple transcription through whispered](https://github.com/aks-tel/mod_quickjs/blob/main/examples/v16_whisperd.js)
    - [Voice assistant based on OpenAI services (chatGPT + whisper)](https://github.com/aks-tel/mod_quickjs/blob/main/examples/v16_chatgpt.js)
    - [Asynchronous requests in the Curl](https://github.com/aks-tel/mod_quickjs/blob/main/examples/curl_async_test.js)
 
## version 1.0
 Quite old version, developed as a replacement for mod_spidermonkey (with capabilities to launch its scripts without changes)
 - [Build and installation guide](https://github.com/aks-tel/mod_quickjs/blob/main/docs/installation_guide.pdf)
 - [Functions](https://github.com/aks-tel/mod_quickjs/blob/main/docs/builtin_functions_v10.pdf)
 - [Classes](https://github.com/aks-tel/mod_quickjs/blob/main/docs/builtin_classes_v10.pdf)
 - [Examples](examples/)

