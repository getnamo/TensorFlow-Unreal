import unreal_engine as ue
import tensorflow as tf
import upythread as ut
import json
import imp
#import mnistSimple
import sys
import importlib

class TensorFlowComponent:

	# constructor adding a component
	def __init__(self):
		ue.log('Component Init')

	# this is called on game start
	def begin_play(self):
		ue.log('Beginplay')

		ue.log('importing TF module: ' + self.uobject.TensorFlowModule)
		self.tf = importlib.import_module(self.uobject.TensorFlowModule)
		imp.reload(self.tf)

		#train
		if(self.uobject.ShouldTrainOnBeginPlay):
			self.train()

	#bp to python test
	def pythonfunction(self, args):
		ue.log('python function call with: ' + args)

	#tensor input
	def tensorinput(self, args):
		#call a blueprint function
		#self.uobject.bpfunction('hi from python')

		ue.log('TF inputs passed: ' + args)

		#pass the raw json to the script to handle
		resultJson = self.tf.runJsonInput(self.trained, json.loads(args))

		#pass prediction json back
		self.uobject.OnResultsFunction(json.dumps(resultJson))

	#single threaded call
	def trainBlocking(self):
		ue.log(self.uobject.TensorFlowModule + ' training started on bt thread.')
		self.trained = self.tf.train()

	#multi-threaded call
	def train(self, args=None):
		ue.log(self.uobject.TensorFlowModule + ' training scheduled.')

		if(self.uobject.ShouldUseMultithreading):
			try:
				ut.run_on_bt(self.trainBlocking)
			except:
				e = sys.exc_info()[0]
				ue.log('TensorFlowComponent error: ' + str(e))
		else:
			self.trainBlocking()