"""The example map's script (/TensorFlow/Examples/MnistExample): an MNIST digit classifier you train, stop, and
watch read digits from a render target.

BP_MnistExample runs it on a TensorFlow component next to a Python Render Target Stream named "camera" that
streams the canvas the actor draws test digits into. Keys: T train, S stop training, N next digit. At the
first start it trains (no saved model yet); later starts load Saved/TensorFlow/mnist.keras instead.

Events for the map (On Custom Event), with display-ready text:
    "Status"      a status line (training progress, accuracy)
    "Digit"       {"pixels": [784 values 0..1], "label": 3, "text": "..."}: draw this into the canvas
    "Prediction"  {"prediction": 3, "confidence": 0.98, "text": "Camera sees: 3 (98%)"}
"""
import random
import time

import numpy as np

import unreal_bridge as ue
from mnist_keras_cnn import MnistKerasCNN, _arg

KEYS = "[T] train  [S] stop  [N] next digit"


class MnistDemo(MnistKerasCNN):
    def on_setup(self):
        super().on_setup()
        self.first_training = True
        self.epochs = _arg("epochs", 3)
        self.last_prediction = (None, 0.0)
        self.call_event("Status", f"Loaded the trained model.   {KEYS}" if self.model is not None
                        else "No trained model yet: training starts...")

    # -- training ------------------------------------------------------------------------------------
    def on_begin_training(self):
        if self.first_training and self.model is not None:
            self.first_training = False  # Train On Begin Play: only when there's no model yet
            return {"skipped": True, "reason": "a trained model was loaded"}
        self.first_training = False
        self.call_event("Status", "Loading MNIST and building the model...")
        result = super().on_begin_training()
        verb = "Stopped early" if self.should_stop else "Trained"
        kept = " (kept the previous model)" if result.get("kept_previous_model") else ""
        self.call_event("Status", f"{verb}: test accuracy {result['test_accuracy']:.1%}{kept}   {KEYS}")
        return result

    def report_progress(self, data):
        super().report_progress(data)  # OnTrainingProgress (JSON)
        epoch = data.get("epoch", 0) + 1
        if data.get("epoch_end"):
            text = f"Epoch {epoch}/{self.epochs} done, validation accuracy {data.get('val_accuracy', 0):.1%}"
        else:
            text = f"Training: epoch {epoch}/{self.epochs}, batch {data.get('batch', 0)}, accuracy {data.get('accuracy', 0):.1%}"
        self.call_event("Status", text + "   [S] stop")

    # -- digits --------------------------------------------------------------------------------------
    def next_digit(self):
        """Sends a random MNIST test image to draw (event "Digit")."""
        index = random.randrange(10000)
        sample = self.test_sample(index)
        self.call_event("Digit", {
            "pixels": [round(float(v), 3) for v in sample["image"].ravel()],
            "label": sample["label"],
            "text": f"Drawn: test image {index}, a {sample['label']}",
        })
        return sample["label"]

    def classify(self, gray):
        """gray: (H, W) floats 0..1, light digit on dark."""
        x = self.keras.ops.image.resize(gray[None, :, :, None], (28, 28), interpolation="area")
        scores = self.keras.ops.convert_to_numpy(self.model(x, training=False))[0]
        best = int(scores.argmax())
        return {"prediction": best, "confidence": float(scores[best])}


@ue.on_stream("camera")
def camera(frame):
    """The canvas render target, as it streams: classify it with the current model."""
    api = get_api()
    if getattr(api, "model", None) is None:
        return
    gray = frame.rgb().mean(axis=-1, dtype=np.float32)
    if frame.dtype == "uint8":
        gray /= 255.0
    if gray.max() < 0.05:
        return  # nothing drawn yet
    result = api.classify(gray)
    previous, at = api.last_prediction
    now = time.monotonic()
    if result["prediction"] != previous or now - at >= 1.0:
        api.last_prediction = (result["prediction"], now)
        api.call_event("Prediction", {**result, "text": f"Camera sees: {result['prediction']} ({result['confidence']:.0%})"})


_api = None


def get_api():
    global _api
    if _api is None:
        _api = MnistDemo.get_instance()
    return _api
