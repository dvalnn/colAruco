FROM osrf/ros:melodic-desktop-full

RUN apt-get update && apt-get install --no-install-recommends -y nano

RUN apt-get update; apt-get upgrade -y
RUN apt-get install -y ros-melodic-usb-cam ros-melodic-aruco-ros ros-melodic-camera-calibration

COPY setup_workspace.sh /setup_workspace.sh
RUN chmod +x /setup_workspace.sh
RUN /setup_workspace.sh

CMD [ "/bin/bash" ]