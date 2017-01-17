import unreal_engine as ue
import tensorflow as tf
import json
import imp
import operator
import mnistSimple

class TensorFlowComponent:

	# constructor adding a component
	def __init__(self):
		ue.log('Init from plugin dir!')
		self.train('')

	# this is called on game start
	def begin_play(self):
		ue.log('Beginplay')

		#call a blueprint function
		self.uobject.bpfunction('hi from python')


	#bp to python test
	def pythonfunction(self, args):
		ue.log('python function call with: ' + args)

	#tensor input
	def tensorinput(self, args):
		ue.log('TF inputs passed: ' + args)

		imgstruct = json.loads(args)
		pixelarray = imgstruct['pixels']
		ue.log('image len: ' + str(len(pixelarray)))

		#do tf stuff here with passed image
		sess = self.trained['sess']
		feed_dict = {self.trained['x']: [pixelarray]}

		ue.log(feed_dict)

		#get predicted result
		result = sess.run(self.trained['y'], feed_dict)

		ue.log(result)

		index, value = max(enumerate(result[0]), key=operator.itemgetter(1))

		ue.log('max: ' + str(value) + 'at: ' + str(index))

		#set the prediction result
		imgstruct['prediction'] = index

		#pass prediction back
		self.uobject.OnResultsFunction(json.dumps(imgstruct))

	def train(self, args):
		ue.log('init data for ' + args)

		imp.reload(mnistSimple)

		ue.log('mnistSimple reloaded')
		
		self.trained = mnistSimple.train()

		ue.log('trained x: ' + str(self.trained['x']))
		ue.log('trained y: ' + str(self.trained['y']))
		ue.log('trained W: ' + str(self.trained['W']))
		ue.log('trained b: ' + str(self.trained['b']))