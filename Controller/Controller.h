/*
 * Controller.h
 *
 *  Created on: Mar 27, 2017
 *      Author: stefano
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

// CONTROL PANEL
#define CTRLPANEL_NEVTS		6

#define RWND_BTN		0x0001
#define STOP_BTN		0X0002
#define PLAY_BTN		0x0004
#define PAUS_BTN		0X0008
#define LOAD_BTN		0X0010
#define FRWD_BTN		0X0020

extern short CTRLPANEL_EVTS[CTRLPANEL_NEVTS];

//FSPECT PANEL
#define FSPANEL_NEVTS		2

#define ZOOMIN_BTN		0X0040
#define ZOOMOUT_BTN		0X0080

extern short FSPANEL_EVTS[FSPANEL_NEVTS];
//POSITION PANEL

#endif /* CONTROLLER_H_ */
