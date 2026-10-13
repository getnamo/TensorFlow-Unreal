"""MNIST digits with a small Keras 3 CNN. On a TensorFlow component (Script = "examples/mnist_keras_cnn"):

    Begin Training     trains on MNIST (downloaded once by Keras): On Training Progress after every epoch and
                       every 50 batches, Stop Training ends it early, On Training Complete reports the test
                       accuracy. The model is saved to Saved/TensorFlow/mnist.keras and loaded on start (a
                       training stopped early doesn't replace an existing model).
    Send Tensor Input  a (28, 28) or (28, 28, 1) float image, white digit on black, 0..1 (e.g. Texture To
                       Tensor with Grayscale, Size 28x28) -> {"prediction": 7, "confidence": 0.99, "scores": [...]}
    Send Json Input    {"pixels": [784 values]} (TensorFlow-Unreal 1.x style) -> the same
    On Custom Event    "TrainingSample": {"pixels", "label", "size"}: an image the model trains on

Script Params > Args: "epochs=3", "samples=60000" (train on fewer images for a quick run),
"model=mnist.keras" (file name under Saved/TensorFlow).
"""
import os

import numpy as np

import unreal_bridge as ue
from unreal_bridge.ml import TFPluginAPI


def _arg(name, default):
    for a in ue.args:
        if a.startswith(name + "="):
            return type(default)(a.split("=", 1)[1])
    return default


class MnistKerasCNN(TFPluginAPI):
    def on_setup(self):
        import keras
        self.keras = keras
        self.model_path = os.path.join(ue.project_dir, "Saved", "TensorFlow", _arg("model", "mnist.keras"))
        self.model = None
        self.test_set = None
        if os.path.exists(self.model_path):
            self.model = keras.saving.load_model(self.model_path)
            ue.log("loaded", self.model_path)

    # -- prediction -------------------------------------------------------------------------------------
    def predict(self, image):
        if self.model is None:
            raise RuntimeError("no model yet: train it first (Begin Training)")
        x = np.asarray(image, dtype=np.float32).reshape(1, 28, 28, 1)
        scores = self.keras.ops.convert_to_numpy(self.model(x, training=False))[0]
        best = int(scores.argmax())
        return {"prediction": best, "confidence": float(scores[best]), "scores": [float(s) for s in scores]}

    def on_tensor_input(self, image):
        return self.predict(image)

    def on_json_input(self, data):
        return self.predict(data["pixels"])

    def test_sample(self, index=0):
        """A test-set image and its label, e.g. to send back as a tensor input."""
        if self.test_set is None:
            (_, _), self.test_set = self.keras.datasets.mnist.load_data()
        images, labels = self.test_set
        return {"image": images[index].astype(np.float32) / 255.0, "label": int(labels[index])}

    # -- training ---------------------------------------------------------------------------------------
    def on_begin_training(self):
        keras = self.keras
        (x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
        self.test_set = (x_test, y_test)
        samples, epochs = _arg("samples", 0), _arg("epochs", 3)
        if samples:
            x_train, y_train = x_train[:samples], y_train[:samples]
        x_train = (x_train.astype("float32") / 255.0)[..., None]
        x_test = (x_test.astype("float32") / 255.0)[..., None]
        self.call_event("TrainingSample", {"pixels": x_train[0].ravel().tolist(), "label": int(y_train[0]), "size": {"x": 28, "y": 28}})

        model = keras.Sequential([
            keras.Input((28, 28, 1)),
            keras.layers.Conv2D(32, 3, activation="relu"),
            keras.layers.MaxPooling2D(),
            keras.layers.Conv2D(64, 3, activation="relu"),
            keras.layers.MaxPooling2D(),
            keras.layers.Flatten(),
            keras.layers.Dropout(0.5),
            keras.layers.Dense(10, activation="softmax"),
        ])
        model.compile(loss="sparse_categorical_crossentropy", optimizer="adam", metrics=["accuracy"])
        model.fit(x_train, y_train, batch_size=128, epochs=epochs, validation_split=0.1, verbose=0,
                  callbacks=[ue.keras.StopOnRequest(self), ue.keras.ProgressEvents(self, every_n_batches=50)])

        loss, accuracy = model.evaluate(x_test, y_test, verbose=0)
        summary = {"test_accuracy": float(accuracy), "test_loss": float(loss), "epochs": epochs}
        if self.should_stop and self.model is not None:
            summary["kept_previous_model"] = True  # stopped early: don't replace a finished model with this one
            return summary
        self.model = model  # predictions use it from now on
        os.makedirs(os.path.dirname(self.model_path), exist_ok=True)
        model.save(self.model_path)
        summary["saved"] = self.model_path
        return summary

    def on_save_model(self, name):
        path = os.path.join(os.path.dirname(self.model_path), name or "mnist.keras")
        self.model.save(path)
        return path

    def on_load_model(self, name):
        path = os.path.join(os.path.dirname(self.model_path), name or "mnist.keras")
        self.model = self.keras.saving.load_model(path)
        return path


def get_api():
    return MnistKerasCNN.get_instance()
