# tensorflow-ue4

[![GitHub release](https://img.shields.io/github/release/getnamo/tensorflow-ue4/all.svg)](https://github.com/getnamo/tensorflow-ue4/releases)
[![Github All Releases](https://img.shields.io/github/downloads/getnamo/tensorflow-ue4/total.svg)](https://github.com/getnamo/tensorflow-ue4/releases)

[Unreal Engine](https://www.unrealengine.com) plugin for [TensorFlow](https://www.tensorflow.org/). Enables training and implementating state of the art [machine learning](https://en.wikipedia.org/wiki/Machine_learning) algorithms for your unreal projects. 

This plugin source contains C++, Blueprint and python scripts that encapsulate TensorFlow operations as an _Actor Component_. The plugin depends on [UnrealEnginePython](https://github.com/getnamo/UnrealEnginePython) plugin fork and [SocketIO Client](https://github.com/getnamo/socketio-client-ue4) plugin.

Releases for this plugin contain compiled versions of all dependency plugins and you should be able to drag and drop it into your project.

If you have ideas or fixes, consider contributing! See https://github.com/getnamo/tensorflow-ue4/issues for details. 

## Installation & Setup

 1.	(GPU only) [Install CUDA and cudNN pre-requisites](https://www.tensorflow.org/install/install_windows) if you're using compatible GPUs (NVIDIA)
 2.	[Download Latest Release](https://github.com/getnamo/tensorflow-ue4/releases) choose CPU or GPU download version if supported.
 3.	Create new or choose project.
 4.	Browse to your project folder (typically found at _Documents/Unreal Project/{Your Project Root}_)

![copy plugins](http://i.imgur.com/Dktr6JK.png)
 
 5.	Copy *Plugins* folder into your Project root.
 6.	(Optional) All plugins should be enabled by default, you can confirm via Edit->Plugins. Scroll down to Project and you should see three plugins, TensorFlow in Computing, Socket.IO Client in Networking and UnrealEnginePython in Scripting Languages. Click Enabled if any is disabled and restart the Editor and open your project again.
 7.	Plugin is now ready to use.

## Note on Dependencies
Depends on [UnrealEnginePython](https://github.com/getnamo/UnrealEnginePython) plugin fork and [SocketIO Client](https://github.com/getnamo/socketio-client-ue4) plugin. Both of these are included in every [release](https://github.com/getnamo/tensorflow-ue4/releases) so you don't need to manually include them.

The UnrealEnginePython plugin fork contains changes to enable multi-threading, python script plugin encapsulation and automatic dependency resolution via pip. Simply specifying tensorflow as _pythonModule_ dependency in https://github.com/getnamo/tensorflow-ue4/blob/master/Content/Scripts/upymodule.json makes the editor auto-resolve the dependency on first run. The multi-threading support contains a callback system allowing long duration operations to happen on a background thread (e.g. training) and then receiving callbacks on your game-thread. This enables TensorFlow to work without noticeably impacting the game thread.

SocketIO Client is used for easy conversion between C++ and blueprint types (structs and variables) and JSON. Can also be used optionally to easily connect to a real-time web service via [socket.io](https://socket.io/).

## Examples

[![mnist spawn samples](http://i.imgur.com/kvsLXvF.gif)](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSpawnSamples.py)

A project with example content is found at [tensorflow-ue4-examples](https://github.com/getnamo/tensorflow-ue4-examples). The repository shows off different e.g. mnist classification examples inside UE4, but should expand as more plug and play examples are made. Consider contributing your cool samples!

It is also the main repository where all development is tracked for the other plugin dependencies for tensorflow.

## Python API

You can either train directly or use a trained model inside UE4.

To start, add your python script file to _{Project Root Folder}/Content/Scripts_.

wrap your tensorflow python code by subclassing TFPluginAPI.

#### MySubClass(TFPluginAPI)

import ```tensorflow```, ```unreal_engine``` and ```TFPluginAPI``` in your module file and subclass the TFPluginAPI class with the following functions.

```python
import tensorflow as tf
import unreal_engine as ue
from TFPluginAPI import TFPluginAPI

class ExampleAPI(TFPluginAPI):

	#expected optional api: setup your model for training
	def onSetup(self):
		pass
		
	#expected optional api: parse input object and return a result object, which will be converted to json for UE4
	def onJsonInput(self, jsonInput):
		result = {}
		return result

	#expected optional api: start training your network
	def onBeginTraining(self):
		pass
    
#NOTE: this is a module function, not a class function. Change your CLASSNAME to reflect your class
#required function to get our api
def getApi():
	#return CLASSNAME.getInstance()
	return ExampleAPI.getInstance()
```

Note the ```getApi()``` module function which needs to return a matching instance of your defined class. The rest of the functionality depends on what API you wish to use for your use case. At the moment the plugin supports input/output from UE4 via JSON encoding.

If you wish to train in UE4, implement your logic in ```onBeginTraining()``` and ensure you check for ```self.shouldstop``` after each batch/epoch to handle early exit requests from the user e.g. when you _EndPlay_ or manually call ```StopTraining``` on the tensorflow component. You will also receive an optional ```onStopTraining``` callback when the user stops your training session.

If you have a trained model, simply setup your model/load it from disk and omit the training function, and forward your evaluation/input via the ```onJsonInput(jsonArgs)``` callback. See [mnistSaveLoad.py example](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSaveLoad.py) on how to train a network once, and then save the model, reloading it on setup such that you skip retraining it every time.

Note that both ```onTrain()``` and ```onSetup()``` are asynchronous by default with no additional code required by the developer. If you use a high level library like e.g. keras, may need to store your *tf.Session* and *tf.Graph* separately and use it as default ```with self.session.as_default():``` and ```with self.graph.as_default():``` to evaluate, since the call will be done a separate thread from the training one.


Below is a very basic example of using tensorflow to add or subtract values passed in as ```{"a":<float number or array>, "b":<float number or array>}```.

```python
import tensorflow as tf
import unreal_engine as ue
from TFPluginAPI import TFPluginAPI

class ExampleAPI(TFPluginAPI):

	#expected optional api: setup your model for training
	def onSetup(self):
		self.sess = tf.InteractiveSession()

		self.a = tf.placeholder(tf.float32)
		self.b = tf.placeholder(tf.float32)

		#operation
		self.c = self.a + self.b
		pass
		
	#expected optional api: json input as a python object, get a and b values as a feed_dict
	def onJsonInput(self, jsonInput):
		
		#show our input in the log
		print(jsonInput)

		#map our passed values to our input placeholders
		feed_dict = {self.a: jsonInput['a'], self.b: jsonInput['b']}

		#run the calculation and obtain a result
		rawResult = self.sess.run(self.c,feed_dict)
		
		#embedd the answer as a string value in a python object
		return {'answer':str(rawResult)}

	#custom function to change the operation type
	def changeOperation(self, type):
		if(type == '+'):
			self.c = self.a + self.b

		elif(type == '-'):
			self.c = self.a - self.b


	#expected optional api: We don't do any training in this example
	def onBeginTraining(self):
		pass
    
#NOTE: this is a module function, not a class function. Change your CLASSNAME to reflect your class
#required function to get our api
def getApi():
	#return CLASSNAME.getInstance()
	return ExampleAPI.getInstance()
```

A full example using mnist can be seen here: https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSimple.py

A full example using save/load setup can be seen here: https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSaveLoad.py

Another full example using keras api can be found here: https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistKerasCNN.py. Note the keras callback used for stopping training after current batch completes, this cancels training on early gameplay exit e.g. EndPlay.

#### Asynchronous Events to Tensorflow Component

If you need to stream some data to blueprint e.g. during training you can use the ```self.callEvent()``` api. 

##### String Format
The format is ```self.callEvent('EventName', 'MyString')```

##### Json Format
The format is ```self.callEvent('EventName', PythonObject, True)```

Example use case in [mnistSpawnSamples.py](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSpawnSamples.py#L121) where sample training images are emitted to unreal for preview.

## Blueprint API

### Load your python module from your TensorflowComponent
Once you've [written your python module](https://github.com/getnamo/tensorflow-ue4#mysubclasstfpluginapi), Select your TensorflowComponent inside your actor blueprint

![select component](http://i.imgur.com/f9Syql1.png)

and change the TensorFlowModule name to reflect your _filename_ without .py. e.g. if my python file is _ExampleAPI.py_ it would look like this

![change module name](http://i.imgur.com/mpzymgd.png)

Optionally disable the verbose python log and change other toggles such as training on _BeginPlay_ or disabling multithreading (not recommended).

### Training

By default the _onBeginTraining()_ function will get called on the component's begin play call. You can optionally untick this option and call _Begin Training_ manually.

![manual train](http://i.imgur.com/YM3KZwy.png)

### Sending Json inputs to your model for e.g. prediction

You control what type of data you forward to your python module and the only limitation for the current api is that it should be JSON formatted.

#### Basic Json String
In the simplest case you can send e.g. a basic json string ```{"MyString","SomeValue"}``` constructed using SIOJson like so

![send json string](http://i.imgur.com/xizBrpt.png)

#### Any UStruct Example

SIOJson supports completely user defined structs, even ones only defined in blueprint. It's highly recommended to use such structs for a convenient way to organize your data and to reliably decode it on the python side. Below is an example where we send a custom bp struct and encode it straight to JSON.

![send custom struct](http://i.imgur.com/Ova2xzf.png)

with the struct defined in blueprint as

![custom struct definition](http://i.imgur.com/hg3qlSK.png)

You can also interweave structs, even common unreal types so feel free to mix and match both of the above methods. In this particular example we interweave a 3D vector in a json object we defined. The sent input should now be ```{"SomeVector":{"x":1.0,"y":2.3,"z":4.3}}```

![send struct](http://i.imgur.com/NJ48M70.png)


#### Special convenience case: UTexture2D

A convenience function wraps a UTexture2D into a json object with ```{"pixels":[<1D array of pixels>], "size":{"x":<image width>,:"y":<image height>}}``` which you can reshape using numpy.

![send texture](http://i.imgur.com/vSq2xea.png)

Note that this currently will convert an image into full alpha greyscale. If you need color texture inputs, use own custom method or make a pull request.

#### Custom functions

If you need to call python functions from blueprint which the current api doesn't support, you can do so by using the ```CallCustomFunction``` method on the _TensorflowComponent_. You specify the function name and pass in a string as arguments. The function runs on the game thread and will return immediately with an expected string value. For both arguments and returning values, JSON encoding is recommended, but optional.

![custom function call](http://i.imgur.com/ejBs8cI.png)

Example custom function call passing in a string argument to [```changeOperation```](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/addExample.py#L31) in [addExample.py](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/addExample.py)

### Handling Tensorflow Events

Select your _Tensorflow Component_ from your actor blueprint and then click + to subscribe to the chosen event in the event graph. 

![events](http://i.imgur.com/2q7486k.png)

current api supports the following events

#### On Input Results

Called when _onJsonInput()_ completes in your python module. The returned data is a json string of the return data you pass at the end of the function.

![onresults](http://i.imgur.com/lLbtmVt.png)

Normally you'd want to convert this string into _SIOJsonObject_ so you can use your results data in blueprint. It is also typical to have a prediction field attached to this object for e.g. classification tasks.

If you have a regular return format, consider making your own custom bp struct and fill its value from the json string like this

![fill struct from json](http://i.imgur.com/IBWYzw9.png)

Note that the function will only fill fields that have matching names and ignore all other struct fields. This means you can safely fill a partial struct from a json string that has more fields than the struct defines.

#### On Training Complete

When the _onBeginTraining()_ call is complete you receive this event with ```{'elapsed':<time taken>}``` json, optionally with additional return data passed in from your function.

![ontraining](http://i.imgur.com/XiZhH04.png)

#### On Event

If you use [```self.callEvent()```](https://github.com/getnamo/tensorflow-ue4#asynchronous-events-to-tensorflow-component) you will receive this event dispatch. You can filter your event types by the event name and then do whatever you need to with the data passed in.

![onevent](http://i.imgur.com/ny0aEZv.png)

For example [mnistSpawnSamples.py](https://github.com/getnamo/tensorflow-ue4-examples/blob/master/Content/Scripts/mnistSpawnSamples.py#L121) uses ```self.callEvent()``` to async stream training images and we'd filter this via checking for ```'PixelEvent'```

## Blueprint Utilities

### Conversion
A large portion of the plugin capability comes from its ability to convert data types. See [TensorflowBlueprintLibrary.h](https://github.com/getnamo/tensorflow-ue4/blob/master/Source/TensorFlow/Public/TensorFlowBlueprintLibrary.h) for full declarations and code comments.

#### UTexture2D to float array (grayscale)

Convert a UTexture2D as grayscale to a 1D float array; obtains size from texture.

_Blueprint_

```
ToGrayScaleFloatArray (Texture2D)
```

_C++_
```c++
static TArray<float> Conv_GreyScaleTexture2DToFloatArray(UTexture2D* InTexture);
```


#### UTexture2D to float array

Convert a UTexture2D to a 1D float array; obtains size from texture. Expects 4 1-byte values per pixel e.g. RGBA.

_Blueprint_

```
ToFloatArray (Texture2D)
```

_C++_
```c++
static TArray<float> Conv_Texture2DToFloatArray(UTexture2D* InTexture);
```

#### Invert Float Array

Invert values in a given float array (1->0, 0->1) on a 0-1 scale.

_Blueprint_ 

```
InvertFloatArray
```

_C++_
```c++
static TArray<float> InvertFloatArray(const TArray<float>& InFloatArray);
```

#### Float array to UTexture2D

Convert a 4 value per pixel float array to a UTexture2D with specified size, if size is unknown (0,0), it will assume a square array.

_Blueprint_ 

```
ToTexture2D (Float Array)
```

_C++_
```c++
static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0,0));
```


#### Float array (Grayscale) to UTexture2D

Convert a 1 value per pixel float array to a UTexture2D with specified size, if size is unknown (0,0), it will assume a square array.

_Blueprint_ 

```
ToTexture2D (Grayscale Array)
```

_C++_
```c++
static UTexture2D* Conv_FloatArrayToTexture2D(const TArray<float>& InFloatArray, const FVector2D Size = FVector2D(0,0));
```

#### ToTexture2D (Render Target 2D)

Convert a UTextureRenderTarget2D to a UTexture2D

_Blueprint_ 

```
ToTexture2D (Render Target 2D)
```

_C++_
```c++
static UTexture2D* Conv_RenderTargetTextureToTexture2D(UTextureRenderTarget2D* InTexture);
```

#### ToFloatArray (bytes)

Convert a byte array into a float array, normalized by the passed in scale

_Blueprint_ 

```
ToFloatArray (bytes)
```

_C++_
```c++
static TArray<float> Conv_ByteToFloatArray(const TArray<uint8>& InByteArray, float Scale = 1.f);
```

## TF Audio Capture Component

A c++ component that uses windows api to capture and stream microphone audio without the need of an online subsystem. See https://github.com/getnamo/tensorflow-ue4/blob/master/Source/TFAudioCapture/Public/TFAudioCaptureComponent.h for details on API. 

This component is aimed to be used for native speech recognition when Tensorflow examples mature.

## File Utility Component

A simple blueprint wrapper to save and load bytes from file. Allows to easily flush e.g. audio capture for later use. See https://github.com/getnamo/tensorflow-ue4/blob/master/Source/CoreUtility/Public/FileUtilityComponent.h for details on API.

## [License](https://github.com/getnamo/tensorflow-ue4/blob/master/LICENSE)
Plugin - [MIT](https://opensource.org/licenses/MIT)

TensorFlow and TensorFlow Icon - [Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0)



