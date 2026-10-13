"""Classifies what a render target shows, as it streams. Next to a TensorFlow component running this script
(Script = "examples/camera_stream_classify"), add a Python Render Target Stream named "camera" with the render
target (e.g. one you draw a digit on: light on dark), and train the MNIST model first (examples/mnist_keras_cnn
saves Saved/TensorFlow/mnist.keras).

    On Custom Event  "Prediction": {"prediction": 3, "confidence": 0.98, "frame": 1234, "brightness": 0.12, ...} whenever the predicted
                     digit changes (and at most every Args "interval=0.25" seconds otherwise)
    Send Tensor Input / Send Json Input work as in mnist_keras_cnn
"""
import os
import time

import numpy as np

import unreal_bridge as ue
from unreal_bridge.ml import TFPluginAPI


def _arg(name, default):
    for a in ue.args:
        if a.startswith(name + "="):
            return type(default)(a.split("=", 1)[1])
    return default


class CameraClassifier(TFPluginAPI):
    def on_setup(self):
        import keras
        self.keras = keras
        path = os.path.join(ue.project_dir, "Saved", "TensorFlow", _arg("model", "mnist.keras"))
        self.model = keras.saving.load_model(path) if os.path.exists(path) else None
        if self.model is None:
            ue.warning(f"no model at {path}: run examples/mnist_keras_cnn's training first")
        self.interval = _arg("interval", 0.25)
        self.last = (None, 0.0)

    def classify(self, gray):
        """gray: (H, W) floats 0..1, light digit on dark -> prediction dict."""
        x = self.keras.ops.image.resize(gray[None, :, :, None], (28, 28), interpolation="area")
        scores = self.keras.ops.convert_to_numpy(self.model(x, training=False))[0]
        best = int(scores.argmax())
        return {"prediction": best, "confidence": float(scores[best])}

    def on_tensor_input(self, image):
        image = np.asarray(image, dtype=np.float32)
        return self.classify(image.mean(axis=-1) if image.ndim == 3 else image)

    def on_json_input(self, data):
        return self.classify(np.asarray(data["pixels"], dtype=np.float32).reshape(28, 28))


@ue.on_stream("camera")
def camera(frame):
    api = get_api()
    if getattr(api, "model", None) is None:  # not set up yet, or no trained model
        return
    rgb = frame.rgb()
    gray = rgb.mean(axis=-1, dtype=np.float32)
    if frame.dtype == "uint8":
        gray /= 255.0
    result = api.classify(gray)
    previous, at = api.last
    now = time.monotonic()
    if result["prediction"] != previous or now - at >= api.interval:
        api.last = (result["prediction"], now)
        api.call_event("Prediction", {**result, "frame": frame.frame, "brightness": round(float(gray.mean()), 4), "shape": list(frame.shape), "layout": frame.layout})


_api = None


def get_api():
    global _api
    if _api is None:
        _api = CameraClassifier.get_instance()
    return _api
