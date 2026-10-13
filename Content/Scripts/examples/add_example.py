"""The smallest TensorFlow example. On a TensorFlow component (Script = "examples/add_example"):

    Send Json Input {"a": [1, 2], "b": [3, 4]}         -> On Input Results {"c": [4.0, 6.0]}
    Call Custom Function "change_operation" "\"-\""    -> subtracts from then on
"""
import tensorflow as tf

from unreal_bridge.ml import TFPluginAPI


class AddExample(TFPluginAPI):
    def on_setup(self):
        self.operation = tf.add

    def on_json_input(self, data):
        a = tf.constant(data["a"], tf.float32)
        b = tf.constant(data["b"], tf.float32)
        return {"c": self.operation(a, b).numpy().tolist()}

    def change_operation(self, op):
        self.operation = tf.subtract if op == "-" else tf.add
        return op

    changeOperation = change_operation  # the 1.x example's name


def get_api():
    return AddExample.get_instance()
