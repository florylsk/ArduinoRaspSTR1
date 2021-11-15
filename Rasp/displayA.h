#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/**********************************************************
 *  Function: displayInit
 *  Parameters: signalRT = signal number for display thread
 between SIGRTMIN and SIGRTMIN
 *********************************************************/
int displayInit(int signalRT);

/**********************************************************
 *  Function: displaySlope
 *	Parameters: slope = -1 => down ground
 *				slope =  0 => flat ground
 *				slope =  1 =>   up ground
 *********************************************************/
int displaySlope(int slope);

/**********************************************************
 *  Function: displaySpeed
 *	Parameters: speed = value of speed to display
 *********************************************************/
int displaySpeed(double speed);

/**********************************************************
 *  Function: displayGas
 *	Parameters: gas = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayGas(int gas);

/**********************************************************
 *  Function: displayBrake
 *	Parameters: brake = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayBrake(int brake);

/**********************************************************
 *  Function: displayMix
 *	Parameters: mixer = 1 -> ON; 0 -> OFF
 *********************************************************/
int displayMix(int mixer);



/**********************************************************
 *  Function: simulator
 *  Parameters: request -> request message
 *   			answer  -> answer message
 *********************************************************/
int simulator(char request[10], char answer[10]);

#endif /* _DISPLAY_H_ */

