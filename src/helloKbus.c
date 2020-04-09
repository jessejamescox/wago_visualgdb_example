//-----------------------------------------------------------------------------
//  Copyright (c) WAGO Kontakttechnik GmbH & Co. KG
//
//  PROPRIETARY RIGHTS are involved in the subject matter of this material.
//  All manufacturing, reproduction, use and sales rights pertaining to this
//  subject matter are governed by the license agreement. The recipient of this
//  software implicitly accepts the terms of the license.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
///  \file     adi_visualgdb_example.c
///
///  \version  $Id:0.0.1
///
///  \brief    Simple demo program for using kbus ADI interface .
///            with Visual Studio and VisualGDB
///
///  \author   J. Cox WAGO Corp., U.S.A
//----------------------------------------------------------------------------- 
// standard include files
//-----------------------------------------------------------------------------

#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
//-----------------------------------------------------------------------------
// include files for KBUS WAGO ADI
//-----------------------------------------------------------------------------
#include <dal/adi_application_interface.h>

//-----------------------------------------------------------------------------
// defines and test setup
//-----------------------------------------------------------------------------

// priorities
#define KBUS_MAINPRIO 40       // main loop

//-----------------------------------------------------------------------
/// MAIN
//-----------------------------------------------------------------------

int
main(void)
{
	// vars for ADI-interface
	tDeviceInfo deviceList[10];            // the list of devices given by the ADI
	size_t nrDevicesFound;                 // number of devices found
	size_t nrKbusFound;                    // position of the kbus in the list
	tDeviceId kbusDeviceId;                // device ID from the ADI
	tApplicationDeviceInterface* adi;     // pointer to the application interface
	uint32_t taskId = 0;                   // task Id 
	tApplicationStateChangedEvent event;   // var for the event interface of the ADI

	// process data
	uint16_t pd_in[4096];      // kbus input process data (WORDS)
	uint16_t pd_out[4096];     // kbus output process data (WORDS)

	// generic vars
	int i = 0, loops = 0;
	time_t last_t = 0, new_t;
	long unsigned runtime = 0;
	struct sched_param s_param;

	// startup info */
	printf("***********************************************************************\n");
	printf("***       KBUS Demo Application with ViusualGDB      V0.0.1         ***\n");
	printf("***********************************************************************\n");

	// clear process memory
	memset(pd_in, 0, sizeof(pd_in));
	memset(pd_out, 0, sizeof(pd_out));

	// connect to ADI-interface
	adi = adi_GetApplicationInterface();

	// init interface
	adi->Init();

	// scan devices
	adi->ScanDevices();
	adi->GetDeviceList(sizeof(deviceList), deviceList, &nrDevicesFound);

	// find kbus device
	nrKbusFound = -1;
	for (i = 0; i < nrDevicesFound; ++i)
	{
		if (strcmp(deviceList[i].DeviceName, "libpackbus") == 0)
		{
			nrKbusFound = i;
			printf("KBUS device found as device %i\n", i);
		}
	}

	// kbus not found > exit
	if(nrKbusFound == -1)
	{
		printf("No KBUS device found \n");
		adi->Exit();   // disconnect ADI-Interface
		return - 1;   // exit program
	}

	// switch to RT Priority
	s_param.sched_priority = KBUS_MAINPRIO;
	sched_setscheduler(0, SCHED_FIFO, &s_param);
	printf("switch to RT Priority 'KBUS_MAINPRIO'\n");

	// open kbus device 
	kbusDeviceId = deviceList[nrKbusFound].DeviceId;
	if (adi->OpenDevice(kbusDeviceId) != DAL_SUCCESS)
	{
		printf("Kbus device open failed\n");
		adi->Exit();   // disconnect ADI-Interface
		return - 2;   // exit program
	}
	printf("KBUS device open OK\n");


	// Set application state to "Running" to drive kbus by your selve.
	event.State = ApplicationState_Running;
	if (adi->ApplicationStateChanged(event) != DAL_SUCCESS)
	{
		// Set application state to "Running" failed
		printf("Set application state to 'Running' failed\n");
		adi->CloseDevice(kbusDeviceId);   // close kbus device
		adi->Exit();   // disconnect ADI-Interface
		return - 3;   // exit programm
	}
	printf("Set application state to 'Running' \n");

	int errorFlag = 0;
    
	// run main loop while no errors are present
	while(errorFlag == 0)
	{
		usleep(100);   // wait 10 ms

		uint32_t retval = 0;

		// Use function "libpackbus_Push" to trigger one KBUS cycle.
		if(adi->CallDeviceSpecificFunction("libpackbus_Push", &retval) != DAL_SUCCESS)
		{
			// CallDeviceSpecificFunction failed
			printf("CallDeviceSpecificFunction failed\n");
			adi->CloseDevice(kbusDeviceId);   // close kbus device
			adi->Exit();   // disconnect ADI-Interface
			errorFlag = -4;
			return -4; // exit programm
		}

		if (retval != DAL_SUCCESS)
		{
			// Function 'libpackbus_Push' failed
			printf("Function 'libpackbus_Push' failed\n");
			adi->CloseDevice(kbusDeviceId);   // close kbus device
			adi->Exit();   // disconnect ADI-Interface
			errorFlag = -5;
			return -5; // exit programm
		}

		loops++;

		// Trigger Watchdog
		adi->WatchdogTrigger();

		// 1s tick for test output
		new_t = time(NULL);
		if (new_t != last_t)
		{
			last_t = new_t;
			runtime++;

			// read inputs
			adi->ReadStart(kbusDeviceId, taskId); /* lock PD-In data */
			adi->ReadBytes(kbusDeviceId, taskId, 0, 2, (uint16_t*)&pd_in[0]); /* read 1 byte from address 0 */
			adi->ReadEnd(kbusDeviceId, taskId); /* unlock PD-In data */
			
			// calculate something and write the outputs
			float temperatureC = (pd_in[0] / 10);
			float temperatureF = ((temperatureC * 1.8) + 32);
			
			// reset the word
			pd_out[0] = 0;
			
			// set the bits to turn DO into temp gague
			if (temperatureF > 70){(pd_out[0]  |= 1 << 15); }
			if (temperatureF > 71){(pd_out[0]  |= 1 << 14); }
			if (temperatureF > 72){(pd_out[0]  |= 1 << 13); }
			if (temperatureF > 73){(pd_out[0]  |= 1 << 12); }
			if (temperatureF > 74){(pd_out[0]  |= 1 << 11); }
			if (temperatureF > 75){(pd_out[0]  |= 1 << 10); }
			if (temperatureF > 76){(pd_out[0]  |= 1 << 9); }
			if (temperatureF > 77){(pd_out[0]  |= 1 << 8); }
			if (temperatureF > 78){(pd_out[0]  |= 1 << 7); }
			if (temperatureF > 79){(pd_out[0]  |= 1 << 6); }
			if (temperatureF > 80){(pd_out[0]  |= 1 << 5); }
			if (temperatureF > 81){(pd_out[0]  |= 1 << 4); }
			if (temperatureF > 82){(pd_out[0]  |= 1 << 3); }
			if (temperatureF > 83){(pd_out[0]  |= 1 << 2); }
			if (temperatureF > 84){(pd_out[0]  |= 1 << 1); }
			if (temperatureF > 85){(pd_out[0]  |= 1 << 0); }

			
			// write outputs
			adi->WriteStart(kbusDeviceId, taskId); /* lock PD-out data */
			adi->WriteBytes(kbusDeviceId, taskId, 0, 2, (uint16_t*)&pd_out[0]); /* write */
			adi->WriteEnd(kbusDeviceId, taskId); /* unlock PD-out data */
			loops = 0;
			// show temperature data
			printf("TC Temperature = %.2fC || %.2fF \n", temperatureC, temperatureF);
			printf("Output Value = %d", pd_out[0]);
		}

	} // while ..

	// close kbus device
	adi->CloseDevice(kbusDeviceId);

	adi->Exit();   // disconnect ADI-Interface
	return 0;   // exit program
}//eof