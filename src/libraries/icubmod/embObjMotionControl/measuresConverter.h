// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-


/* Copyright (C) 2012  iCub Facility, Istituto Italiano di Tecnologia
 * Author: Valentina Gaggero
 * email: valentina.gaggero@iit.it
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

// update comment hereafter

/**
 * @ingroup icub_hardware_modules
 * \defgroup eth2ems eth2ems
 *
 * Implements <a href="http://wiki.icub.org/yarpdoc/d3/d5b/classyarp_1_1dev_1_1ICanBus.html" ICanBus interface <\a> for a ems to can bus device.
 * This is the eth2ems module device.
 *
 * Copyright (C) 2012 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 *
 * Author: Valentina Gaggero
 *
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 * This file can be edited at src/modules/....h
 *
 */


#include <yarp/dev/ControlBoardHelper.h>

#ifndef __measureConverterh__
#define __measureConverterh__

class measureConvFactors
{
public:
    double* angleToEncoder;
    double* dutycycleToPWM;
    double* ampsToSensor;
    double* newtonsToSensor;

};

class torqueControlConvFactors
{

private:
    int numofjoints;
public:
    double * angleToEncoder;
    double * newtonsToSensor;



    torqueControlConvFactors(int numofjoints);

    bool init (const double *angleToEncoderFactors, const double *newtonsToSensorFactors);

    ~torqueControlConvFactors();

    double get_N2S(int joint);

    double get_A2E(int joint); 
};



class measuresConverter : public ControlBoardHelper
{

private:
    torqueControlConvFactors *trqCrtlFactors;

public:

     measuresConverter( int n, const int *aMap, torqueControlConvFactors &trqCrtlFact, measureConvFactors &measFact);

    ~measuresConverter();

    double getAngleToEncoder(int j);

    void convertTrqPid_N2S(int j, yarp::dev::Pid &pid);//NEWTON TO SENSOR

    void convertTrqPid_S2N(int j, yarp::dev::Pid &pid);

    void convertPosPid_A2E(int j, yarp::dev::Pid &pid);//ANGLE TO SENSOR

    void convertPosPid_E2A(int j, yarp::dev::Pid &pid);

    double convertTrqMotorBemfParam_MachineUnitsToMetric(int j, double bemf);

    double convertTrqMotorKtaufParam_MachineUnitsToMetric(int j, double ktau);

    double convertTrqMotorBemfParam_MetricToMachineUnits(int j, double bemf);

    double convertTrqMotorKtaufParam_MetricToMachineUnits(int j, double ktau);
};

#endif