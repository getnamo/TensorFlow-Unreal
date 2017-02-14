import upy_redirect_print
import mnist_softmax as ms
import upythread as ut
import imp

def Train():
	imp.reload(ms)
	print('started.')
	ms.main()
	print('done.')


def test():

	#Train()
	ut.run_on_bt(Train)
