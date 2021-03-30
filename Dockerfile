# This is an auto generated Dockerfile for ros:ros-base
# generated from docker_images/create_ros_image.Dockerfile.em
FROM ros:melodic-ros-core-bionic

# install bootstrap tools
RUN apt-get update && apt-get install --no-install-recommends -y \
    build-essential \
    python-rosdep \
    python-rosinstall \
    python-rosinstall-generator \
    python-wstool \
    python-vcstools \
    && rm -rf /var/lib/apt/lists/*

# bootstrap rosdep
RUN rosdep init 
#rosdep update --rosdistro $ROS_DISTRO


# Now create the ros user itself
RUN adduser --gecos "ROS User" --disabled-password ros
RUN usermod -a -G dialout ros

# And, as that user...
USER ros

RUN rosdep update

USER root

# install ros packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-melodic-ros-base=1.4.1-0* \
    && rm -rf /var/lib/apt/lists/*
RUN apt-get update; apt-get upgrade -y
RUN apt-get install -y ros-melodic-usb-cam ros-melodic-aruco-ros ros-melodic-camera-calibration
RUN apt-get update; apt-get upgrade -y


RUN mkdir -p /home/ros/workspace/src
RUN HOME=/home/ros

RUN /bin/bash -c '. /opt/ros/melodic/setup.bash; catkin_init_workspace /home/ros/workspace/src'
RUN /bin/bash -c '. /opt/ros/melodic/setup.bash; cd /home/ros/workspace; catkin_make'
ADD .bashrc \~/


CMD [ "/bin/bash" ]