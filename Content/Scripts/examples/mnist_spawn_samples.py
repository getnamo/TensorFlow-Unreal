"""Streams MNIST test images to Unreal as events, e.g. to spawn an actor showing each one
(Script = "examples/mnist_spawn_samples"):

    Call Custom Function "spawn_samples" "[10]"  -> On Custom Event "SpawnSample" {"pixels": [784 values 0..1],
                                                    "label": 3, "index": 42, "size": {"x": 28, "y": 28}}
                                                    ten times (ToTexture2D (Grayscale Array) shows one)
    Begin Training                               -> keeps streaming one image every 0.2 s until Stop Training
"""
import random
import time

import unreal_bridge as ue
from unreal_bridge.ml import TFPluginAPI


class SpawnSamples(TFPluginAPI):
    def on_setup(self):
        import keras
        (_, _), (self.images, self.labels) = keras.datasets.mnist.load_data()

    def _send(self, index):
        self.call_event("SpawnSample", {
            "pixels": (self.images[index].ravel() / 255.0).tolist(),
            "label": int(self.labels[index]),
            "index": int(index),
            "size": {"x": 28, "y": 28},
        })

    def spawn_samples(self, count=1):
        for _ in range(int(count)):
            self._send(random.randrange(len(self.images)))
        return count

    def on_begin_training(self):
        sent = 0
        while not self.should_stop:
            self._send(random.randrange(len(self.images)))
            sent += 1
            time.sleep(0.2)
        return {"sent": sent}


def get_api():
    return SpawnSamples.get_instance()
