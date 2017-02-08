# tensorflow-ue4

Unreal Engine plugin for [TensorFlow](https://www.tensorflow.org/).

This plugin contains C++, Blueprint and python scripts that encapsulate TensorFlow operations as an _Actor Component_. The component allows you to either include normal tensorflow python scripts with little change and have the I/O forwarded from UE4 or to (in the future) construct tensorflow operations directly from blueprint.

Currently incomplete, requires a new basic API for blueprint support. See https://github.com/getnamo/tensorflow-ue4/issues for details. 

## Dependencies
Depends on [UnrealEnginePython](https://github.com/getnamo/UnrealEnginePython) plugin and [SocketIO Client](https://github.com/getnamo/socketio-client-ue4) plugin.

The UnrealEnginePython plugin fork contains changes to allow automatic dependency resolution via pip. Simply specifying tensorflow as _pythonModule_ dependency in https://github.com/getnamo/tensorflow-ue4/blob/master/Content/Scripts/upymodule.json makes the editor auto-resolve the dependency on first run. The fork also contains multi-threading support and callback on game-thread to allow TensorFlow to work without noticeably impacting the game thread.

SocketIO Client is used to easy conversion between C++ and blueprint types (structs and variables) and JSON. Can also be used to easily connect to a web service.

## Examples
Project examples found under [tensorflow-ue4-examples](https://github.com/getnamo/tensorflow-ue4-examples). 

It is also the main repository where all development is tracked for all plugin dependencies for tensorflow.
