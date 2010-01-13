#!/bin/sh

# Copyright (C) 2008 RobotCub Consortium, European Commission FP6 Project IST-004370
# Author: Assif Mirza 
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

# Robot Name
#NAME_ROBOT=icubSim
NAME_ROBOT=icub
NAME_APP=iha

## Robot Ports #################################################

DRAGONFLY_LEFT_NAME=/$NAME_ROBOT/cam/left
DRAGONFLY_RIGHT_NAME=/$NAME_ROBOT/cam/right
CAMCALIB_LEFT_NAME=/$NAME_ROBOT/camcalib/left
CAMCALIB_LEFT_PORT_IMAGE=$CAMCALIB_LEFT_NAME/image
CAMCALIB_RIGHT_NAME=/$NAME_ROBOT/camcalib/right
CAMCALIB_RIGHT_PORT_IMAGE=$CAMCALIB_RIGHT_NAME/image

# generic pointers
# To be used by anything that doesn't care whether
# it is a dragonfly or whatever
NAME_SOURCE_IMAGE_LEFTEYE=$DRAGONFLY_LEFT_NAME
NAME_SOURCE_IMAGE_RIGHTEYE=$DRAGONFLY_RIGHT_NAME

# Set a single variable to point to the camera
# so we don't have to specify left/right in scripts
CAMERA=$DRAGONFLY_LEFT_NAME

#this is for an opencv controlled camera which can be projected into the 
#simulator to interact with the robot's face detector, etc
OPENCV_NAME=/gcv
CAM_SIM_PORT=/$NAME_ROBOT/texture2


#this is for the camera on the mobileeye unit
DVGRAB_NAME=/gdv

VIEWER_CAM_LEFT_PORT=/view/cam/left
VIEWER_FACEDETECT_PORT=/view/facedetect
VIEWER_SALIENCE_PORT=/view/salience
VIEWER_SELFFACEDETECT_PORT=/view/selffacedetect
VIEWER_GAZE_PORT=/view/gaze


## MODULE BASE NAMES ###########################################

FACEDETECT_NAME=/$NAME_APP/fd
SELFFACEDETECT_NAME=/$NAME_APP/sfd
PORTAUDIO_NAME=/$NAME_APP/pa
SOUNDSERVER_NAME=/$NAME_APP/sound
DYNAMICS_NAME=/$NAME_APP/dynamics
CONTROLLER_NAME=/$NAME_APP/controller
DATASTORE_NAME=/$NAME_APP/ds
ACTIONSEL_NAME=/$NAME_APP/as
SMI_INTERFACE_NAME=/$NAME_APP/sm
MONITOR_NAME=/$NAME_APP/status
SFW_NAME=/$NAME_APP/sfw
SALIENCE_NAME=/$NAME_ROBOT/salience/right
MOBEYE_NAME=/$NAME_APP/me
MOBEYESERVER_NAME=/$NAME_APP/mes
STM_NAME=/$NAME_APP/stm
AUDIOANALYSER_NAME=/$NAME_APP/beat
## MODULE PORTS ################################################

FACEDETECT_PORT_OUT=$FACEDETECT_NAME/facedetect:out
FACEDETECT_PORT_DETECT=$FACEDETECT_NAME/facedetect:coords
FACEDETECT_PORT_IN=$FACEDETECT_NAME/facedetect:in

SELFFACEDETECT_PORT_OUT=$SELFFACEDETECT_NAME/facedetect:out
SELFFACEDETECT_PORT_DETECT=$SELFFACEDETECT_NAME/facedetect:coords
SELFFACEDETECT_PORT_IN=$SELFFACEDETECT_NAME/facedetect:in

PORTAUDIO_PORT=/$NAME_APP/sound_grabber

SOUNDSERVER_PORT_IN=$SOUNDSERVER_NAME/sndsensor:in
SOUNDSERVER_PORT_OUT=$SOUNDSERVER_NAME/sndsensor:out


AUDIOANALYSER_PORT_IN=$AUDIOANALYSER_NAME/sndsensor:in
AUDIOANALYSER_PORT_OUT=$AUDIOANALYSER_NAME/sndsensor:out

DYNAMICS_PORT_DATA_IN=$DYNAMICS_NAME/data:in
DYNAMICS_PORT_MEM_IN=$DYNAMICS_NAME/mem:in
DYNAMICS_PORT_REWARD_OUT=$DYNAMICS_NAME/reward:out

CONTROLLER_ACTION_CMD=$CONTROLLER_NAME/action:cmd
CONTROLLER_ENCODERS_OUT=$CONTROLLER_NAME/encoders:out

EXPRESSION_RAW_PORT=/$NAME_ROBOT/face/raw/in

DATASTORE_DIST_OUT=$DATASTORE_NAME/currdist:out
DATASTORE_STATUS_OUT=$DATASTORE_NAME/status:out

ACTIONSEL_STATUS_OUT=$ACTIONSEL_NAME/status:out

STM_DATA_IN=$STM_NAME/data:in
STM_SCORE_OUT=$STM_NAME/score:out

SMI_INTERFACE_SENSORS_OUT=$SMI_INTERFACE_NAME/sensor:out
SMI_INTERFACE_MEMSENSORS_OUT=$SMI_INTERFACE_NAME/memsensor:out
SMI_INTERFACE_STATUS_OUT=$SMI_INTERFACE_NAME/status:out
SMI_INTERFACE_IMAGE_IN=$SMI_INTERFACE_NAME/image:in
SMI_INTERFACE_FACE_IN=$SMI_INTERFACE_NAME/facecoords:in
SMI_INTERFACE_GAZE_IN=$SMI_INTERFACE_NAME/gazecoords:in
SMI_INTERFACE_SOUND_IN=$SMI_INTERFACE_NAME/soundsensor:in
SMI_INTERFACE_ENCODERS_IN=$SMI_INTERFACE_NAME/encoders:in

MONITOR_OUT=$MONITOR_NAME/monitor:out
MONITOR_IN=$MONITOR_NAME/monitor:in

SALIENCE_PORT_IN=$SALIENCE_NAME/view
SALIENCE_PORT_OUT=$SALIENCE_NAME/view
SALIENCE_PORT_MAP=$SALIENCE_NAME/map

#MOBILEEYESERVER_PORT_IN=$MOBILEEYESERVER_NAME/gazesensor:out
#MOBILEEYESERVER_PORT=$MOBILEEYESERVER_NAME/gazesensor:out
MOBEYESERVER_PORT=/mobeyeserver

MOBEYE_PORT_GAZE_IN=$MOBEYE_NAME/mobileeye:gaze
MOBEYE_PORT_FACE_IN=$MOBEYE_NAME/mobileeye:face
MOBEYE_PORT_IN=$MOBEYE_NAME/mobileeye:in
MOBEYE_PORT_OUT=$MOBEYE_NAME/mobileeye:out
MOBEYE_PORT_DETECT=$MOBEYE_NAME/mobileeye:coords