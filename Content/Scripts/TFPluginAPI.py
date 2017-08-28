import sys

import tensorflow as tf
import unreal_engine as ue

class TFPluginAPI():
	@classmethod
	def getInstance(cls):
		#This should return an instance of your class even if you subclassed it
		return cls()

	## Private
	def __init__(self):
		#class scoped variable for stopping
		self.shouldStop = False		#check this variable to stop training early
		self.shouldRetrain = False	#use this variable to force your training
		self.stored = {}

	#internal don't need to override this
	def _resetTrainingTrigger(self):
		self.shouldStop = False

	#internal don't need to override this: early stopping
	def _stopTraining(self):
		self.shouldStop = True
		self.onStopTraining()

	## Public
	
	#call this inside your class to emit a custom event on gt, don't override the function
	def callEvent(self, event, data = None, useJson = False):
		self.tf_component.custom_event(event, data, useJson)

	#expected api: setup your model for training
	def onSetup(self):
		#setup or load your model and pass it into stored
		
		#Usually store session, graph, and model if using keras
		#self.sess = tf.InteractiveSession()
		#self.graph = tf.get_default_graph()
		pass

	#expected api: storedModel and session, json inputs
	def onJsonInput(self, jsonInput):
		#e.g. our json input could be a pixel array
		#pixelarray = jsonInput['pixels']

		#run input on your graph
		#e.g. sess.run(model['y'], feed_dict)
		# where y is your result graph and feed_dict is {x:[input]}

		#...

		#return a json you will parse e.g. a prediction
		result = {}
		result['prediction'] = 0

		return result



	#expected api: no params forwarded for training? TBC
	def onBeginTraining(self):
		#train here

		#...

		#inside your training loop check if we should stop early
		#if(self.shouldStop):
		#	break
		pass

	def onStopTraining(self):
		#you should be listening to self.shouldStop, but you can also receive this call
		pass

#required function to get our api
def getApi():
	#return CLASSNAME.getInstance()
	return TFPluginAPI.getInstance()