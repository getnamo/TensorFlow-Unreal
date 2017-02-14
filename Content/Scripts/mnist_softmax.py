# Copyright 2015 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

"""A very simple MNIST classifier.

See extensive documentation at
http://tensorflow.org/tutorials/mnist/beginners/index.md
"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import sys
import os.path
import time

# Import data
from tensorflow.examples.tutorials.mnist import input_data

import tensorflow as tf

def main():
  mnist = input_data.read_data_sets('/tmp/tensorflow/mnist/input_data', one_hot=True)

  # Create the model
  x = tf.placeholder(tf.float32, [None, 784], name='x')
  W = tf.Variable(tf.zeros([784, 10]), name='W')
  b = tf.Variable(tf.zeros([10]), name='b')
  y = tf.matmul(x, W) + b

  # Define loss and optimizer
  y_ = tf.placeholder(tf.float32, [None, 10], name='y_')

  # The raw formulation of cross-entropy,
  #
  #   tf.reduce_mean(-tf.reduce_sum(y_ * tf.log(tf.nn.softmax(y)),
  #                                 reduction_indices=[1]))
  #
  # can be numerically unstable.
  #
  # So here we use tf.nn.softmax_cross_entropy_with_logits on the raw
  # outputs of 'y', and then average across the batch.
  cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(y, y_))
  train_step = tf.train.GradientDescentOptimizer(0.5).minimize(cross_entropy)


  sess = tf.InteractiveSession()
  # Train
  tf.global_variables_initializer().run()
  for i in range(1000):
    if(i%100 == 0):
      print(i)
    batch_xs, batch_ys = mnist.train.next_batch(100)
    sess.run(train_step, feed_dict={x: batch_xs, y_: batch_ys})

  # Test trained model
  correct_prediction = tf.equal(tf.argmax(y, 1), tf.argmax(y_, 1))
  accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))
  print(sess.run(accuracy, feed_dict={x: mnist.test.images,
                                      y_: mnist.test.labels}))

  #save the trained model
  saveModel(sess)

  # for now test getting the model back
  #print('model is ' + str(restoreModel(sess)))

  recallTrained()

def recallTrained():
  mnist = input_data.read_data_sets('/tmp/tensorflow/mnist/input_data', one_hot=True)

  # Create the model
  x = tf.placeholder(tf.float32, [None, 784], name='x')
  W = tf.Variable(tf.zeros([784, 10]), name='W')
  b = tf.Variable(tf.zeros([10]), name='b')
  y = tf.matmul(x, W) + b

  # Define loss and optimizer
  y_ = tf.placeholder(tf.float32, [None, 10], name='y_')

  sess = tf.Session()

  restoreModel(sess)

  correct_prediction = tf.equal(tf.argmax(y, 1), tf.argmax(y_, 1))
  accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))
  print(sess.run(accuracy, feed_dict={x: mnist.test.images,
                                      y_: mnist.test.labels}))


def saveModel(sess):
  saver = tf.train.Saver()
  saver.save(sess, '/tmp/mnist-model')

def restoreModel(sess):
  saver = tf.train.Saver()
  saver.restore(sess, '/tmp/mnist-model')