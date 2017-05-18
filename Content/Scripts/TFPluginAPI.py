import sys

import tensorflow as tf
import unreal_engine as ue

class TFPluginAPI():
	@classmethod
	def getInstance(cls):
		#This should return an instance of your class even if you subclassed it
		return cls()

	def __init__(self):
		#class scoped variable for stopping
		self.shouldstop = False
		self.stored = {}

	#expected api: setup your model for training
	def setup(self):
		#setup or load your model and pass it into stored
		
		#Usually store session, graph, and model if using keras
		#self.sess = tf.InteractiveSession()
		#self.graph = tf.get_default_graph()
		pass

	#expected api: storedModel and session, json inputs
	def runJsonInput(self, jsonInput):
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

	#expected api: early stopping
	def stop(self):
		self.shouldstop = True

	#expected api: no params forwarded for training? TBC
	def train(self):
		#train here

		#...

		#inside your training loop check if we should stop early
		#if(this.shouldstop):
		#	break
		pass

#required function to get our api
def getApi():
	#return CLASSNAME.getInstance()
	return TFPluginAPI.getInstance()