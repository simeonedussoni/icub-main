// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// vim:expandtab:tabstop=4:shiftwidth=4:softtabstop=4:

/*
 * Copyright (C) 2008 RobotCub Consortium, European Commission FP6 Project IST-004370
 * Author: Assif Mirza
 * email:   assif.mirza@robotcub.org
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include <iCub/iha/SensorMotorInterfaceModule.h>

#include <iCub/iha/iha_utils.h>
#include <iCub/iha/image_utils.h>

#include <iCub/iha/FaceDetectLoop.h>
#include <iCub/iha/ImageReadLoop.h>
#include <iCub/iha/RewardReadLoop.h>
#include <iCub/iha/SoundSensorReadLoop.h>

using namespace iCub::iha;

/**
 * @addtogroup icub_iha_SensorMotorInterface
 *

This module collects the following data from supporting IHA processes:
- the icub_iha_IcubControl process
  - provides the timestep to be used as the timestep for the output.
  - The rate of the output from the sensor motor interface process should be the same as the rate of the IcubControl encoders value process.
  - unnormalized encoders values from the robot motors
  - the current action number
- the robot camera images port
  - These are processed to produce "sensor" values by pixelating the image and providing a "luminance" value for each larger pixel.
  - Luminance is calculated as 0.3 R + 0.59 G + 0.11 B. See the function imageToSensorArray()
  - the pixelation detail is controlled by the parameters \c --num_image_sensors_x and \c --num_image_sensors_y
- the icub_iha_IhaFaceDetect process
  - although this receives the coordinate of the face, the output is either 0="no face" or 1="face detected"
- the icub_iha_SoundSensor process
  - value of sound intensity between [0,1]
- the icub_iha_MotivationDynamics process
  - provides the consolidated reward value

Additionally this process can provide a reward feedback as a facial expression that can serve to assist in the interaction between robot and human partner. It can be turned on and off using the \c --reward_display switch and the individual expression actions executed are given by \c --action_ehi \c --action_emid \c --action_elo. The switching thresholds are set using \c --th_ehi and \c --th_elo.

\section lib_sec Libraries
- YARP libraries.
- IHA Debug Library

\section parameters_sec Parameters
\verbatim
--dbg [INT]   : debug printing level
--name [STR]  : process name for ports
--file [STR]  : config file

--connect_to_image [STR]       : connect to specified port for images
--connect_to_coords [STR]      : connect to specified port for detected face coordinates
--connect_to_reward [STR]      : connect to specified port for consolidated reward value
--connect_to_soundsensor [STR] : connect to specified port for sound sensor value
--connect_to_encoders [STR]    : connect to specified port for encoders
--connect_to_action [STR]      : connect to port for sending emotion actions

--num_image_sensors_x [INT] : image pixelation
--num_image_sensors_y [INT] : image pixelation
--action_ehi [INT]          : action to execute on high reward
--action_elo [INT]          : action to execute on low reward
--action_emid [INT]         : action to execute on medium reward
--th_ehi [FLT]              : threshold for high reward
--th_elo [FLT]              : threshold for low reward
--reward_display [STR]      : TRUE/FALSE generate actions based on reward
--echo_output [STR]          : TRUE/FALSE print sensor output
\endverbatim

\section portsa_sec Ports Accessed
- /iha/controller/action:cmd 
- /iha/controller/encoders:out
- /iCub/cam/left (or any other camera image port)
- /iha/fd/facedetect:coords
- /iha/sound/sndsensor:out

- /iha/status/monitor:in - status information collection port

\section portsc_sec Ports Created
- /iha/sm/sensor:out - consolidated sensor stream
- /iha/sm/actions:out - to send expression actions (should be connected to the IcubActionControl process)
 
- /iha/sm/image:in  - to recieve images
- /iha/sm/coords:in - to receive face detect coords
- /iha/sm/reward:in - to recive consolidated reward value
- /iha/sm/soundsendor:in - to receive sound intensity value
- /iha/sm/encoders:in - to recieve motor positions

- /iha/sm/status:out - to write status info for monitor process
- /iha/sm/quit  - module quit port

\section conf_file_sec Configuration Files
conf/ihaSensorMotorInterface.ini

Sample INI file:
\verbatim
name iha

###########################################################
# emotion actions
#
# Physical Display ON/OFF
reward_display TRUE
# Hi/Lo/Mid Actions
action_ehi 1
action_elo 16
action_emid 2
# Thresholds
th_ehi 0.8
th_elo 0.4
#
###########################################################


###########################################################
# For image pixelation
num_image_sensors_x 4
num_image_sensors_y 4
#
###########################################################
\endverbatim
\section tested_os_sec Tested OS
Linux

\section example_sec Example Instantiation of the Module
ihaSensorMotorInterface --name /iha/sm --file conf/ihaSensorMotorInterface.ini  --connect_to_image /iCub/cam/left --connect_to_coords /iha/fd/facedetect:coords --connect_to_reward /iha/dynamics/reward:out --connect_to_soundsensor /iha/sound/sndsensor:out --connect_to_encoders /iha/controller/encoders:out --connect_to_action /iha/controller/action:cmd --dbg 50

See also the script $ICUB_ROOT/app/iha_manual/iha_smi.sh

\see iCub::contrib::SensorMotorInterfaceModule

\author Assif Mirza

Copyright (C) 2008 RobotCub Consortium

CopyPolicy: Released under the terms of the GNU GPL v2.0.

This file can be edited at \in src/interactionHistory/sensor_motor_interface/src/SensorMotorInterfaceModule.cpp.
*/

SensorMotorInterfaceModule::SensorMotorInterfaceModule(){
    imageSensorarray=NULL;
}

SensorMotorInterfaceModule::~SensorMotorInterfaceModule(){ 
    if (imageSensorarray!=NULL) delete[] imageSensorarray;
}


bool SensorMotorInterfaceModule::open(Searchable& config){
   
	if (config.check("dbg")) { IhaDebug::setLevel(config.find("dbg").asInt()); }
 	ACE_OS::fprintf(stderr, "Debug level : %d\n",IhaDebug::getLevel());

    if (config.check("help","if present, display usage message")) {
		cerr << "Usage : " << "\n"
		<< "------------------------------------------" << "\n"
		<< "  --dbg [INT]   : debug printing level" << "\n"
		<< "  --name [STR]  : process name for ports" << "\n"
		<< "  --file [STR]  : config file" << "\n"
		<< "---------------------------------------------------------------------------" << "\n"
        << "  --connect_to_image [STR]       : connect to specified port for images" << "\n"
        << "  --connect_to_coords [STR]      : connect to specified port for detected face coordinates" << "\n"
        << "  --connect_to_reward [STR]      : connect to specified port for reward value" << "\n"
        << "  --connect_to_soundsensor [STR] : connect to specified port for sound sensor value" << "\n"
        << "  --connect_to_encoders [STR]    : connect to specified port for encoders" << "\n"
        << "  --connect_to_action [STR]      : connect to port for sending emotion actions" << "\n"
        << "  --connect_to_expression [STR]  : connect to port for sending emotion actions" << "\n"
		<< "---------------------------------------------------------------------------" << "\n"
        << "  --num_image_sensors_x [INT] : image pixelation" << "\n"
        << "  --num_image_sensors_y [INT] : image pixelation" << "\n"
        << "  --action_ehi [INT]          : action to execute on high reward" << "\n"
        << "  --action_elo [INT]          : action to execute on low reward" << "\n"
        << "  --action_emid [INT]         : action to execute on medium reward" << "\n"
        << "  --th_ehi [FLT]              : threshold for high reward" << "\n"
        << "  --th_elo [FLT]              : threshold for low reward" << "\n"
        << "  --reward_display [STR]      : TRUE/FALSE generate actions based on reward" << "\n"
        << "  --echo_output [STR]          : TRUE/FALSE print sensor output" << "\n"
		<< "---------------------------------------------------------------------------" << "\n"
		<< "\n";
        return false;
    }

    bool ok = true;

	//------------------------------------------------------
    // Read parameters
    //
	echo_output = boolStringTest(config.check("echo_output",Value("FALSE")).asString());
	IhaDebug::pmesg(DBGL_INFO,"echo_output %s\n",echo_output?"TRUE":"FALSE");

	// image pixelation
	num_image_sensors_x = config.check("num_image_sensors_x",Value(8)).asInt();
	num_image_sensors_y = config.check("num_image_sensors_y",Value(8)).asInt();
	IhaDebug::pmesg(DBGL_INFO,"num_image_sensors x/y : %d/%d\n",num_image_sensors_x,num_image_sensors_y);

    // allocate the memory for the sensor array
    imageSensorarray=new int[num_image_sensors_x*num_image_sensors_y];
    memset(imageSensorarray,0,sizeof(int)*num_image_sensors_x*num_image_sensors_y);

	// emotive actions. The actions executed in response
	// to reward values, and the thresholds
	bool reward_display = boolStringTest(config.check("reward_display",Value("TRUE")).asString());
	IhaDebug::pmesg(DBGL_INFO,"reward_display %s\n",reward_display?"TRUE":"FALSE");

    int action_ehi = config.check("action_ehi",Value(1)).asInt();
    int action_elo = config.check("action_elo",Value(16)).asInt();
    int action_emid = config.check("action_emid",Value(2)).asInt();
    double th_ehi = config.check("th_ehi",Value(0.8)).asDouble();
    double th_elo = config.check("th_elo",Value(0.3)).asDouble();

    current_eout=action_emid;
    IhaDebug::pmesg(DBGL_INFO,"Emote actions: (%d) < %f (%d) < %f (%d)\n",action_elo,th_elo,action_emid,th_ehi,action_ehi);
	//------------------------------------------------------
	
	//------------------------------------------------------
	// open the output port where we write action advice
    // this should be connected to the robot controller
    ConstString actionPortOutName = getName("action:out");
	IhaDebug::pmesg(DBGL_INFO,"Writing actions to port %s\n",actionPortOutName.c_str());
	actionPortOut.open(actionPortOutName.c_str());
	//------------------------------------------------------

	//------------------------------------------------------
	// open the output port where we write expressions
    // this should be connected to the raw port
    ConstString expressionRawPortName = getName("expression:out");
	IhaDebug::pmesg(DBGL_INFO,"Writing expressions to port %s\n",expressionRawPortName.c_str());
	expressionRawPort.open(expressionRawPortName.c_str());
	//------------------------------------------------------

	//------------------------------------------------------
	// create names of the sensor ports
    ConstString imagePortName = getName("image:in");
    ConstString coordsPortName = getName("coords:in");
    ConstString rewardPortName = getName("reward:in");
    ConstString soundSensorPortName = getName("soundsendor:in");
    ConstString sensorMotorPortName = getName("encoders:in");

	//------------------------------------------------------
	// open the ports
	imagePort.open(imagePortName.c_str());
	coordsPort.open(coordsPortName.c_str());
	rewardPort.open(rewardPortName.c_str());
	soundSensorPort.open(soundSensorPortName.c_str());
	sensorMotorPort.open(sensorMotorPortName.c_str());

	//------------------------------------------------------
    // Create the sensor thread objects
	imageReadLoop = new ImageReadLoop (imagePort);
	faceDetectLoop = new FaceDetectLoop (coordsPort);
	rewardReadLoop = new RewardReadLoop (rewardPort, actionPortOut, expressionRawPort,
                                    reward_display, action_ehi, action_elo, action_emid, th_ehi, th_elo);
	soundSensorReadLoop = new SoundSensorReadLoop (soundSensorPort);

	//------------------------------------------------------
	// start the sensor threads
	imageReadLoop->start();
	faceDetectLoop->start();
	rewardReadLoop->start();
	soundSensorReadLoop->start();

	//------------------------------------------------------
	// make automatic connections
	if (config.check("connect_to_image")) {
		if (connectToParam(config,"connect_to_image",imagePortName.c_str(), 0.25, this)) {
			IhaDebug::pmesg(DBGL_INFO,"Connected to Image\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_coords")) {
		if (connectToParam(config,"connect_to_coords",coordsPortName.c_str(), 0.25, this)) {
            IhaDebug::pmesg(DBGL_INFO,"Connected to Face Coords\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_reward")) {
		if (connectToParam(config,"connect_to_reward",rewardPortName.c_str(), 0.25, this)) {
			IhaDebug::pmesg(DBGL_INFO,"Connected to Rewards\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_soundsensor")) {
		if (connectToParam(config,"connect_to_soundsensor",soundSensorPortName.c_str(), 0.25, this)) {
			IhaDebug::pmesg(DBGL_INFO,"Connected to sound sensor\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_encoders")) {
		if (connectToParam(config,"connect_to_encoders",sensorMotorPortName.c_str(), 0.25, this)) {
			IhaDebug::pmesg(DBGL_INFO,"Connected to encoders\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_action")) {
		// reverse connection
		if (connectToParamReverse(config,"connect_to_action",actionPortOutName.c_str(), 0.25, this)) {
            IhaDebug::pmesg(DBGL_INFO,"Connected to Action\n");
        } else {
            ok = false;
        }
	}
	if (config.check("connect_to_expression")) {
		// reverse connection
		if (connectToParamReverse(config,"connect_to_expression",expressionRawPortName.c_str(), 0.25, this)) {
            IhaDebug::pmesg(DBGL_INFO,"Connected to Expression Raw\n");
        } else {
            ok = false;
        }
	}
	//------------------------------------------------------
    

	//------------------------------------------------------
    // Main Sensor Output Port
    ConstString sensorOutputPortName = getName("sensor:out");
	// open the sensor output port 
	sensorOutputPort.open(sensorOutputPortName.c_str());

	// start the sensor reader loop
	//IhaDebug::pmesg(DBGL_STATUS1, "Starting Sensor Loop\n");
	//SensorLoop sensorLoop(sensorPort,sensorDataRate,numImageSensorsX,numImageSensorsY,true,true,true);
	//sensorLoop.start();

	//------------------------------------------------------
    // Create a port to write status to
    ConstString statusPortName = getName("status:out");
    statusPort.open(statusPortName.c_str());


    ok &= quitPort.open(getName("quit"));
    attach(quitPort, false);
    //attach(quitPort);
    //attachTerminal();
    return ok;
}

bool SensorMotorInterfaceModule::close(){
    fprintf(stderr,"coordsPort.close();\n");
    coordsPort.close();
	fprintf(stderr,"imagePort.close();\n");
	imagePort.close();
	fprintf(stderr,"rewardPort.close();\n");
	rewardPort.close();
	fprintf(stderr,"soundSensorPort.close();\n");
	soundSensorPort.close();
    fprintf(stderr,"sensorMotorPort.close();\n");
    sensorMotorPort.close();
    fprintf(stderr,"statusPort.close();\n");
    statusPort.close();
    fprintf(stderr,"actionPortOut.close();\n");
    actionPortOut.close();
    fprintf(stderr,"sensorOutputPort.close();\n");
    sensorOutputPort.close();
    return true;
}

bool SensorMotorInterfaceModule::interruptModule(){
    fprintf(stderr,"actionPortOut.interrupt();\n");
    actionPortOut.interrupt();
    fprintf(stderr,"sensorOutputPort.interrupt();\n");
    sensorOutputPort.interrupt();
    fprintf(stderr,"coordsPort.interrupt();\n");
    coordsPort.interrupt();
	fprintf(stderr,"imagePort.interrupt();\n");
	imagePort.interrupt();
	fprintf(stderr,"rewardPort.interrupt();\n");
	rewardPort.interrupt();
	fprintf(stderr,"soundSensorPort.interrupt();\n");
	soundSensorPort.interrupt();
    fprintf(stderr,"sensorMotorPort.interrupt();\n");
    sensorMotorPort.interrupt();
    fprintf(stderr,"statusPort.interrupt();\n");
    statusPort.interrupt();
    return true;
}

bool SensorMotorInterfaceModule::updateModule(){

    // Read the encoders data
    Bottle* smData = sensorMotorPort.read();

    // create an output bottle
    Bottle outData;

    // timestep is first
    Value ts=Value(smData->get(0));
    outData.addInt(ts.asInt());
    writeStatus(statusPort,"SensMotIf","TS",ts.asInt());

    // copy all the encoders data except the last item to our output
    for (int i=1;i<smData->size()-1;i++) {
        Value item = Value(smData->get(i));
        outData.addDouble(item.asDouble());
    }

    // face detect - just output the first item which is a boolean detect/not
    if (faceDetectLoop->getFaceState()) {
        outData.addDouble(1);
    } else {
        outData.addDouble(0);
    }

    // value of the sound sensor
    outData.addDouble(soundSensorReadLoop->getCurrentSoundSensor());

    // current action value
    Value ca=Value(smData->get(smData->size()-1));
    outData.addDouble(ca.asDouble());

    // reward
    outData.addDouble(rewardReadLoop->getCurrentReward());
    
    // image sensors last
    yarp::sig::ImageOf< yarp::sig::PixelRgb >* ci = imageReadLoop->aquireCurrentImage();
    if (ci==NULL) {
        for (int i=0;i<num_image_sensors_x*num_image_sensors_y;i++) {
            outData.addDouble(0);
        }
    } else 	{
        imageToSensorArray(ci, imageSensorarray, num_image_sensors_x,num_image_sensors_y);
        for (int i=0;i<num_image_sensors_x*num_image_sensors_y;i++) {
            outData.addDouble(imageSensorarray[i]);
        }
    }
    imageReadLoop->releaseCurrentImage();

    // write to the output port
    sensorOutputPort.write(outData);
    if (echo_output) {
        IhaDebug::pmesg(DBGL_STATUS1,"%s\n",outData.toString().c_str());
    }

    if (IhaDebug::getLevel()==DBGL_STATUSLINE) {
        IhaDebug::pmesg(DBGL_STATUSLINE,"TS: %d Reward: %f\r",ts.asInt(),rewardReadLoop->getCurrentReward());
    } else {
        IhaDebug::pmesg(DBGL_STATUS1,"TS: %d Reward: %f\n",ts.asInt(),rewardReadLoop->getCurrentReward());
    }

    return true;
}

bool SensorMotorInterfaceModule::respond(const Bottle &command,Bottle &reply){
        
    fprintf(stderr,"SensorMotorInterfaceModule::respond %s\n",command.toString().c_str());

    if (command.toString()=="quit") {
        interruptModule();
        yarp::os::Time::delay(1);
        close();
        yarp::os::Time::delay(10);
    }

    return false;
} 	
