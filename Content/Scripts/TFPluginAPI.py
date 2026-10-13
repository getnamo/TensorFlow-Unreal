"""TensorFlow-Unreal 1.x compatibility: `from TFPluginAPI import TFPluginAPI` keeps working.

New scripts: `from unreal_bridge.ml import TFPluginAPI` (or MLPluginAPI, the same with snake_case hooks).
"""
from unreal_bridge.ml import TFPluginAPI  # noqa: F401
