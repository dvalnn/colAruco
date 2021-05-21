#!/bin/bash

mkdir -p /catkin_ws/src/
cd /catkin_ws

. /opt/ros/melodic/setup.bash
catkin_make

echo "source . /opt/ros/melodic/setup.bash" >> /root/.bashrc