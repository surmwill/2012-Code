/******************************************************************************
 * 
 * 	Sprockets 2012 Base Code
 * 	Authors: 	Brendan van Ryn
 * 				Stuart Sullivan
 * 				Aliah McCalla
 * 				Will Surmak
 * Last Updated:	1/04/2012
 * 
 ******************************************************************************/


// Include our own header file with our own macros
#include "NewRobot.h"

// Header file for WPI standard hardware/robot overhead.
#include "WPILib.h"

// Header file for math functions
#include <math.h>

// Macro Functions
// Clears the Driver Station output display
#define CLEARMESSAGE DriverStationLCD::GetInstance()->Clear()

// Provides full free-form formatting, as per the usual printf(). Displays formatted text, a, on Driver Station on line, b.
#define DISPLAYPRINTF(b, a, ...) {DriverStationLCD::GetInstance()->Printf(DriverStationLCD::kUser_Line ## b, 1, a, ## __VA_ARGS__); DriverStationLCD::GetInstance()->UpdateLCD();}

// Prototypes for functions (tells the compiler the format)
float Abs(float); // Returns the absolute value of a number
int AbsI(int); // Returns the absolute value of an integer

// Our main and only class. You could consider this an alteration of "int main(void)" if it makes it easier.
class SimpleTracker : public SimpleRobot
{
    // Global variables accessible to entire program
    Joystick *leftStick; // Left driver joystick
    Joystick *rightStick; // Right driver joystick
    Joystick *speedStick; // Co-driver joystick
    Victor *frontMotor; // Front drive motor
    Victor *rearMotor; // Rear drive motor
    Jaguar *frontSteer; // Front steering motor
    Jaguar *rearSteer; // Rear steering motor
    Jaguar *lateralCollector; // Lateral ball-collector rollers 
    Jaguar *bottomShooter; // Bottom shooting roller
    Encoder *frontSteeringEncoder; // Front steering encoder
    Encoder *rearSteeringEncoder; // Rear steering encoder
    Encoder *driveEncoder; // Drive wheel encoder
    Gyro *steeringGyro; // Steering gyro
    Relay *magazineBelt; // Magazine conveyor
    Relay *magazineBelt2; // Second magazine conveyor motor
    Relay *transverseCollector; // Transverse collector belts
    DigitalInput *armDownSwitch; // The limit switch that indicates the bridge manipulator is down 
    DigitalInput *armUpSwitch; // The limit switch that indicates that the bridge manipulator is up
    DigitalInput *armStartSensor; // The light sensor that detects the primed position for the bridge manipulator
    DigitalInput *armHitSensor; // The light sensor that detects when the bridge manipulator hits the bridge
    DigitalInput *autonomousSwitchOne; // The autonomous switch that determines our autonomous mode
    DigitalInput *autonomousSwitchTwo; // Another autonomous switch that determines our autonomous mode
    AnalogChannel *rollerVoltage; // Voltage reading from shooting rollers (for speed)
    Servo *frontDriveServo; // The servo to shift gears for the front gear box
    Servo *rearDriveServo; // THe servo to shift gears for the rear gear box
    Servo *armEngageServo; // The servo to engage the bridge manipulator arm
    
    /* "Constructor". This code is run when the robot is first turned on. */
    public:
	SimpleTracker(void)
	{	
	// Assign object pointers here.      
        frontMotor = new Victor(FRONTMOTORPORT); // Driving Motors
        rearMotor = new Victor(REARMOTORPORT);
        
        frontSteer = new Jaguar(FRONTSTEERINGMOTORPORT); // Steering Motors
        rearSteer = new Jaguar(REARSTEERINGMOTORPORT);
        
        bottomShooter = new Jaguar(BOTTOMSHOOTINGMOTORPORT); // Other motors
        lateralCollector = new Jaguar(LATERALCOLLECTORPORT);
        
        leftStick = new Joystick(LEFTJOYSTICKPORT); // Joysticks
        rightStick = new Joystick(RIGHTJOYSTICKPORT);
        speedStick = new Joystick(SPEEDJOYSTICKPORT);
        
        frontSteeringEncoder = new Encoder(FRONTSTEERINGENCODERPORTA, FRONTSTEERINGENCODERPORTB); // Encoders
        rearSteeringEncoder = new Encoder(REARSTEERINGENCODERPORTA, REARSTEERINGENCODERPORTB);
        driveEncoder = new Encoder(DRIVEENCODERPORTA, DRIVEENCODERPORTB);
        
        steeringGyro = new Gyro(1, STEERINGGYROPORT); // Analogue devices
        rollerVoltage = new AnalogChannel(ANALOGCHANNELPORT);
        
        magazineBelt = new Relay(MAGAZINERELAYPORT); // Relays
        magazineBelt2 = new Relay(MAGAZINERELAY2PORT);
        transverseCollector = new Relay(TRANSVERSECOLLECTORPORT);
       
        armStartSensor = new DigitalInput(ARMSTARTPOSITIONPORT); // Light sensors
        armHitSensor = new DigitalInput(ARMHITPOSITIONPORT);
        
        autonomousSwitchOne = new DigitalInput(AUTONOMOUSSWITCHPORTONE); // Switches
        autonomousSwitchTwo = new DigitalInput(AUTONOMOUSSWITCHPORTTWO);
        armDownSwitch = new DigitalInput(ARMDOWNSWITCHPORT);
        armUpSwitch = new DigitalInput(ARMUPSWITCHPORT);
        
        frontDriveServo = new Servo(FRONTSERVOPORT); // Servos
        rearDriveServo = new Servo(REARSERVOPORT);
        armEngageServo = new Servo(ARMENGAGESERVOPORT);
        
        // Initialize encoders
        frontSteeringEncoder->Reset();
        frontSteeringEncoder->Start();
        rearSteeringEncoder->Reset();
        rearSteeringEncoder->Start();
        driveEncoder->Reset();
        driveEncoder->Start();
      
        // Initialize gyro 
        steeringGyro->Reset();
        steeringGyro->SetSensitivity(GYROSENSITIVITY);
	}
	
    /* Code to run while in tele-op, or driver-controlled  mode. */
	void OperatorControl(void)
	{
		// Variables
		float leftx, rightx, lefty, righty; // The x- and y-positions of the two driver joysticks
		float speedz; // The position of the co-driver throttle
		float x, y; // The average x- and y-positions between the two driver joysticks
		float swerveSpeed; // The drive motor speed when in swerve-drive mode
		float swerveAngle; // The angle to turn the wheels in swerve-drive mode
		float swerveAngleFront, swerveAngleRear; // The individual angles for the front and back sides, for use in different steering modes
		int steeringMode = SWERVEMODE; // Store the current mode of steering
		int currentButtonState = 0, previousButtonState = 0; // Store the current and previous states of the manual alignment buttons
		int driveWheelEncoderValue; // Distance travelled by robot
		float frontGearSetting = LOWGEARPOSITION; // Store the angle setting for the servos
		float rearGearSetting = LOWGEARPOSITION;
		float armPosition = LOWGEARPOSITION;
		int armStage = 0; // Current stage of bridge manipulator
		int magazineReverseCount = 0; // Allow the magazine to reverse for a bit before shooting
		int servoPauseCount = 0; // Give the servos time to shift
		int magazineAlternator = 0; // Alternate the direction of the magazine when firing to provide spacing for the balls
		int preFire = 0; // Are we trying to use a pre-set firing speed?
		int brakeValue = -1; // Encoder value to maintain when in brake mode
		int armOverride = 0; // Override flag for manual winding of bridge manipulator
		double shooterSpeed = 0.0; // Voltage from motor to read roller speed
		int direction = -1; // The direction of the drive (allows quick reversing)
		int reverseToggleCurrent = 0; // Previous state of reversing button
		int reverseTogglePrevious = 0; // Current state of reversing button
		
		// Reset the drive and shooter encoders, and the timer
		driveEncoder->Reset();
		driveEncoder->Start();
		
		// This code is run repeatedly so long as the robot is in teleop mode.
		while(IsOperatorControl())
		{
			/* Tell the operating system that our program hasn't frozen */
			GetWatchdog().Feed();
			
			/* Message displays */
			CLEARMESSAGE;
			DISPLAYPRINTF(1, "Shooter Speed: %0.3f", shooterSpeed); // The speed of the shooting roller
			DISPLAYPRINTF(2, "Distance: %d", driveWheelEncoderValue); // Value of drive wheel encoder
			
			// Get joystick positions
			leftx = direction * leftStick->GetRawAxis(1);
			lefty = direction * leftStick->GetRawAxis(2);
			rightx = direction * rightStick->GetRawAxis(1);
			righty = direction * rightStick->GetRawAxis(2);
			speedz = (speedStick->GetRawAxis(3) - 1.0f) / 2.0f * -8.0f; // Scale from 0 to 8
			
			// Get voltage from analog channel
			shooterSpeed = rollerVoltage->GetVoltage();
			if(shooterSpeed < 0.0f) shooterSpeed = 0.0f; // Positive voltages only
			
			// Prevent back-rolling
			if(speedz < 0.0f) speedz = 0.0f;
			
			// Run the servo counter
			if(servoPauseCount > 0)
				servoPauseCount--;
			
			// Run the magazine alternator up to a pre-determined value, then reset
			if(magazineAlternator > MAGAZINEALTERNATORLIMIT) magazineAlternator = 0;
			else magazineAlternator++;
			
			// Check  drive mode buttons
			if(SWERVEMODEBUTTON) steeringMode = SWERVEMODE;
			else if(MONSTERMODEBUTTON) steeringMode = MONSTERMODE;
			else if(CARMODEBUTTON) steeringMode = CARMODE;
			
			// Check shooter buttons
			if(FIRSTFIREBUTTON) SetRollerSpeed(FIRESPEEDONE, shooterSpeed), preFire = 1;
			else if(SECONDFIREBUTTON) SetRollerSpeed(FIRESPEEDTWO, shooterSpeed), preFire = 1;
			else if(THIRDFIREBUTTON) SetRollerSpeed(FIRESPEEDTHREE, shooterSpeed), preFire = 1;
			else if(FOURTHFIREBUTTON) SetRollerSpeed(FIRESPEEDFOUR, shooterSpeed), preFire = 1;
			else preFire = 0;
			
			// Check gear shifting buttons
			if(HIGHGEARBUTTON) frontGearSetting = rearGearSetting = HIGHGEARPOSITION, armPosition = ARMDISENGAGEPOSITION, armOverride = 0;
			else if(LOWGEARBUTTON) frontGearSetting = rearGearSetting = LOWGEARPOSITION, armPosition = ARMDISENGAGEPOSITION, armOverride = 0;
			else if(NEUTRALGEARBUTTON) frontGearSetting = LOWGEARPOSITION, rearGearSetting = NEUTRALGEARPOSITION, armPosition = ARMDISENGAGEPOSITION, armOverride = 0;
			else if(BRIDGEMANIPULATEBUTTON && armPosition != ARMENGAGEPOSITION) frontGearSetting = LOWGEARPOSITION, rearGearSetting = NEUTRALGEARPOSITION, armPosition = ARMENGAGEPOSITION, servoPauseCount = SERVOPAUSETIME, armStage = 0, armOverride = 0;
			else if(ARMUNWINDBUTTON) frontGearSetting = LOWGEARPOSITION, rearGearSetting = NEUTRALGEARPOSITION, armPosition = ARMENGAGEPOSITION, armOverride = 1;
			else if(armOverride && !ARMUNWINDBUTTON) frontGearSetting = rearGearSetting = LOWGEARPOSITION, armPosition = ARMDISENGAGEPOSITION, armOverride = 0;
			
			// Move servos to desired position
			frontDriveServo->SetAngle(frontGearSetting);
			rearDriveServo->SetAngle(rearGearSetting);
			armEngageServo->SetAngle(armPosition);
							
			// Get the drive wheel encoder value
			driveWheelEncoderValue = driveEncoder->Get();
			
			// If the brake button isn't being pressed, store a sentinel value as the brake value for reference
			if(!BRAKEMODEBUTTON) brakeValue = -1;
			
			// Check magazine buttons
			if(MAGAZINEENABLEBUTTON || SHOOTBUTTON)
			{
				// Move forward for a set number of counts, then reverse for the rest in each cycle of the alternator
				if(magazineAlternator < MAGAZINEALTERNATORLIMITONE) magazineBelt->Set(Relay::kForward), magazineBelt2->Set(Relay::kForward);
				else magazineBelt->Set(Relay::kReverse), magazineBelt2->Set(Relay::kReverse);
				
				// Prepare to revers when the button is released
				magazineReverseCount = MAGAZINEREVERSETIME;
			}
			else if(MAGAZINEREVERSEBUTTON) magazineBelt->Set(Relay::kReverse), magazineBelt2->Set(Relay::kReverse);
			else if(magazineReverseCount > 0) magazineBelt->Set(Relay::kReverse), magazineBelt2->Set(Relay::kReverse), magazineReverseCount--;
			else magazineBelt->Set(Relay::kOff), magazineBelt2->Set(Relay::kOff);
			
			// Check collector buttons (preFire prevents speed settings from affecting the back rolling of the shooter)
			if(COLLECTORENABLEBUTTON) lateralCollector->Set(COLLECTORSPEED), transverseCollector->Set(Relay::kReverse), bottomShooter->Set(-0.45), preFire = 1;
			else if(COLLECTORREVERSEBUTTON) lateralCollector->Set(-COLLECTORSPEED), transverseCollector->Set(Relay::kForward);
			else 
			{
				lateralCollector->Set(0.0f);
				transverseCollector->Set(Relay::kOff);
				if(lateralCollector->Get() > 0.1f) preFire = 0;
			}
			
			// Check drive-reverse button
			if(REVERSEDRIVEBUTTON) reverseToggleCurrent = 1;
			else reverseToggleCurrent = 0;
			
			// Check manual-alignment button
			if(MANUALALIGNENABLEBUTTON) currentButtonState = 1;
			else currentButtonState = 0;
			
			// If the total y-deflection for each joystick is below the threshold, we shouldn't count it at all, allowing perfect strafing
			if(Abs(lefty) < JOYSTICKDEADBANDY) lefty = 0.0f;
			if(Abs(righty) < JOYSTICKDEADBANDY) righty = 0.0f;
				
			// Average the joystick positions
			x = (leftx + rightx) / 2.0f;
			y = (lefty + righty) / 2.0f;
						
			// Store the states of the toggle buttons from the previous iteration
			previousButtonState = currentButtonState;
			reverseTogglePrevious = reverseToggleCurrent; 
			
			// If we are manipulating the bridge
			if(armPosition == ARMENGAGEPOSITION && servoPauseCount <= 0 && !armOverride)
			{
				// Exit if the button has been released
				if(!BRIDGEMANIPULATEBUTTON) armStage = 4;
				
				// Drive winch until first light sensor is triggered (arm is in primed position)
				if(!armStartSensor->Get() && armStage == 0) rearMotor->Set(0.5f);
				else if(armStage == 0) armStage = 1;
				
				// Stop winch until the light sensor on the bridge dector is triggered (bridge is hit)
				if(!armHitSensor->Get() && armStage == 1) rearMotor->Set(0.0f);
				else if(armStage == 1) armStage = 2;
				
				// Continue to run winch until bottom switch is hit
				if(!armDownSwitch->Get() && armStage == 2) rearMotor->Set(0.85f);
				else if(armStage == 2) armStage = 3;
					
				// Wait for manipulator button to be released
				if(BRIDGEMANIPULATEBUTTON && armStage == 3) rearMotor->Set(0.0f);
				else if(armStage == 3) armStage = 4;
				
				// Return arm
				if(!armUpSwitch->Get() && armStage == 4) rearMotor->Set(-0.2f);
				else if(armStage == 4) armStage = 5;
				
				// Disengage arm, set drive units to low gear, and stop winch motor; reset the armStage for next time
				if(armStage == 5)
				{
					rearGearSetting = LOWGEARPOSITION;
					frontGearSetting = LOWGEARPOSITION;
					armPosition = LOWGEARPOSITION;
					
					rearMotor->Set(0.0f);
				}
			}
						
			// Drive modes
			
			// If the manual-alignment buttons are pressed
			if(MANUALALIGNENABLEBUTTON)
			{
				// Check alignment buttons
				if(MANUALALIGNLEFTREAR) rearSteer->Set(-0.8f);
				else if(MANUALALIGNRIGHTREAR) rearSteer->Set(0.8f);
				else rearSteer->Set(0);
					
				if(MANUALALIGNLEFTFRONT) frontSteer->Set(-0.8f);
				else if(MANUALALIGNRIGHTFRONT)frontSteer->Set(0.8f);
				else frontSteer->Set(0);
			}
			
			// If the lock button is pressed
			else if(LOCKMODEBUTTON)
			{	
				// Lock the wheels to ninety degrees and drive tank-style
				TrackFrontSteering(PULSESPER90FRONT);
				TrackRearSteering(PULSESPER90REAR);
				frontMotor->Set(y);
				rearMotor->Set(y);
			}
			
			// If the brake buttons is pressed
			else if(BRAKEMODEBUTTON)
			{
				// Store value to hold
				if(brakeValue == -1) brakeValue = driveWheelEncoderValue;
				
				// Keep wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
				
				// Maintain position
				TrackDrive(brakeValue, BOTH_MOTORS);
			}
			
			// If the tank turning button is pressed
			else if(TANKTURNINGMODEBUTTON)
			{
				// Turn the wheels in opposite directions
				TrackFrontSteering(PULSESPER90FRONT);
				TrackRearSteering(-PULSESPER90REAR);
				
				// Drive each end tank-style to rotate  
				frontMotor->Set(lefty);
				rearMotor->Set(righty);
			}
				
			// If the pivot mode button is pressed
			else if(PIVOTMODEBUTTON)
			{
				// Keep the front wheels straight and turn the back wheels
				TrackFrontSteering(0);
				TrackRearSteering(PULSESPER90REAR);
				
				// Drive the back to pivot
				frontMotor->Set(0);
				rearMotor->Set(righty);
				
				// Allow manual firing if no automatic firing is in use
				if(!preFire) SetRollerSpeed(speedz, shooterSpeed);
			}
						
			/* Otherwise, we drive normally. If the joysticks are not pushed left or right any significant amount,
			 * drive tank-style */ 
			else if(Abs(x) < JOYSTICKDEADBANDX)
			{
				// Set the drive motors according to their respective joysticks
				frontMotor->Set(lefty);
				
				// Only move rear motor if we aren't in bride-manipulate mode
				if(armPosition != ARMENGAGEPOSITION || servoPauseCount > 0 || armOverride) rearMotor->Set(righty);
				
				// If not pre-set firing speed was selected, allow manual firing
				if(!preFire) SetRollerSpeed(speedz, shooterSpeed);
				
				// Keep the wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
			}
			
			else
			{	
				// Determine our speed and angle of movement (force angle to be in first quadrant)
				swerveSpeed = sqrt(x*x + y*y); // a^2 + b^2 = c^2
				swerveAngle = -Abs(atan(x/y)); // x/y = tan(angle), therefore angle = atan(x/y); negative because of motor direction
				swerveAngle = swerveAngle * 180 / Pi; // 1 radian = Pi/180 radians, so reverse
				swerveAngle = swerveAngle / 90.0f; // Convert from degrees to a percentage of 90 degrees
				
				// Set roller speeds manually if no pre-set speed has been selected
				if(!preFire) SetRollerSpeed(speedz, shooterSpeed);
								
				// Determine the quadrant of the joystick and convert the speed or angle appropriately
				// We assume we're in quadrant one if none of these are true 
				if(x < 0 && y > 0) swerveAngle = -swerveAngle; // Quadrant two, reverse angle
				else if(x < 0 && y <= 0) swerveSpeed = -swerveSpeed; // Quadrant three, reverse drive direction
				else if(x >= 0 && y <= 0) // Quadrant four, reverse both
				{
					swerveSpeed = -swerveSpeed;
					swerveAngle = -swerveAngle;					
				}
				
				// If we are in monster mode and travelling faster than 80% max speed, we switch to car automatically (max speed is sqrt(2))
				if(Abs(swerveSpeed) > 1.2f && steeringMode == MONSTERMODE) steeringMode = CARMODE;
			
				// In monster, we reverse either the front or back steering depending on which direction we're travelling
				if(steeringMode == MONSTERMODE) 
				{
					// Multiplying by 0.75f makes the steering less sensitive as it has a smaller range
					if(swerveSpeed <= 0) swerveAngleFront = -0.75f * swerveAngle, swerveAngleRear = 0.75f * swerveAngle;
					else swerveAngleFront = 0.75f * swerveAngle, swerveAngleRear = -0.75f * swerveAngle;
				}
				
				// In car, we move only the front or only the back wheels, depending on which direction we're travelling
				else if(steeringMode == CARMODE) 
				{
					// Multiplying by 0.5f gives us 45 degrees at most of steering
					if(swerveSpeed <= 0) swerveAngleFront = -0.5f * swerveAngle, swerveAngleRear = 0.0f;
					else swerveAngleRear = -0.5f * swerveAngle, swerveAngleFront = 0.0f;
				}
				
				else 
				{
					// Otherwise, in swerve, both sides get the same angle (negative because of the motor polarity)
					swerveAngleFront = -swerveAngle;
					swerveAngleRear = -swerveAngle;
				}
				
			    // Convert front and rear angle percentages into pulses
			    swerveAngleFront *= PULSESPER90FRONT;
			    swerveAngleRear *= PULSESPER90REAR;
			  
				// Turn steering to desired angle
				TrackFrontSteering((int)swerveAngleFront);
				TrackRearSteering((int)swerveAngleRear);
				
				// Drive at the calculated speed
				frontMotor->Set(swerveSpeed);
					
				// Only move rear wheels if bride manipulate mode is not engaged
				if(armPosition != ARMENGAGEPOSITION || servoPauseCount > 0 || armOverride) rearMotor->Set(swerveSpeed);
			}
			
			// If the manual alignment buttons been released and we previously being pressed, reset the encoders after aligning
			if(previousButtonState == 1 && currentButtonState == 0)
			{
				frontSteeringEncoder->Reset();
				frontSteeringEncoder->Start();
				rearSteeringEncoder->Reset();
				rearSteeringEncoder->Start();
				frontSteer->Set(0);
				rearSteer->Set(0);
			}
			
			// If the drive reverse-button was pressed and has been released, reverse the direction of the joysticks (for driving backwards)
			if(reverseTogglePrevious == 1 && reverseToggleCurrent == 0) direction *= -1;
  		}
	} // End of teleop
	
	/* Code to run while in autonomous, or fully-automated mode. */
	void Autonomous(void)
	{
		// Variables
		int firstSwitch, secondSwitch; // Store the states of the autonomous switches
		
		// Store the values of the switches for the autonomous modes
		firstSwitch = autonomousSwitchOne->Get();
		secondSwitch = autonomousSwitchTwo->Get();
		
		// Check the states of switches and call the correct autonomous mode
		if(firstSwitch && secondSwitch) AutonomousOne(); // Middle position is shoot then strafe
		else if(firstSwitch && !secondSwitch) AutonomousTwo(); // To the right is shoot only
		else if(secondSwitch && !firstSwitch) AutonomousThree(); // To the left is shoot then tip bridge
		else 
		{
			// Display an error if the switch setting is invalid (maybe the switch is unplugged or something)
			CLEARMESSAGE;
			DISPLAYPRINTF(1, "No autonomous switch");
		}
	} // End of Autonomous
	
	/* Move the front wheels to the angle passed to this function in "targetPosition" */
	int TrackFrontSteering(int targetPosition)
	{
		int frontSteeringEncoderValue; // Current wheel position
		int remainingSteeringDistance; // Remaining distance to target position
		float speed; // The speed to set the steering motor to
		
		// Get the current position of the wheels from the encoder and calculate the distance to the target position
		frontSteeringEncoderValue = frontSteeringEncoder->Get();
		remainingSteeringDistance = AbsI(frontSteeringEncoderValue - targetPosition);
		
		/* If the steering is within limits, we simply stop. Otherwise, we move either left or right. The division works thus:
		 * if the wheel is further away from the target than the approach distance, speed will be greater than one; otherwise,
		 * speed will be some percentage, linearly ramping the speed as it approaches zero. */
		if(remainingSteeringDistance < STEERINGDEADBANDFRONT) speed = 0.0f;
		else if(frontSteeringEncoderValue < targetPosition) speed = (float)remainingSteeringDistance / (float)STEERINGAPPROACHDISTANCEFRONT;
		else speed = (float)-remainingSteeringDistance / (float)STEERINGAPPROACHDISTANCEFRONT;
		
		// If we end up with an aforementioned speed greater than one (100%), we clamp it down to exactly 100%
		if(speed > 1.0f) speed = 1.0f;
		else if(speed < -1.0f) speed = -1.0f;
		
		// Turn the motor at the calculated speed
		frontSteer->Set(speed);
		
		/* If the motor is hardly moving, the wheels are where they need to be, so return 0. Otherwise, set the motor speed and
		 * return 1 to indicate that it is still moving. */
		if(speed == 0.0f) return 0;
		else return 1;
	}

	/* This function is the same as the above function, but for the rear wheels, instead. */
	int TrackRearSteering(int targetPosition)
	{
		int rearSteeringEncoderValue;
		int remainingSteeringDistance;
		float speed;
				
		rearSteeringEncoderValue = rearSteeringEncoder->Get();
		remainingSteeringDistance = AbsI(rearSteeringEncoderValue - targetPosition);
				
		if(remainingSteeringDistance < STEERINGDEADBANDREAR) speed = 0.0f;
		else if(rearSteeringEncoderValue < targetPosition) speed = (float)remainingSteeringDistance / (float)STEERINGAPPROACHDISTANCEREAR;
		else speed = (float)-remainingSteeringDistance / (float)STEERINGAPPROACHDISTANCEREAR;
		
		if(speed > 1.0f) speed = 1.0f;
		else if(speed < -1.0f) speed = -1.0f;
			
		rearSteer->Set(speed);
		
		if(speed == 0.0f) return 0; 
		else return 1;
	}
	
	// Maintains a specific speed for the shooting roller. Returns true when roller is at that speed (similar to the steering)
	int SetRollerSpeed(float targetSpeed, float currentSpeed)
	{
		float powerSetting;
		
		// Calculate a power setting based on distance from target speed
		powerSetting = Abs(targetSpeed - currentSpeed) / SHOOTERAPPROACHSPEED + SHOOTERCONSTANTSPEED;
		if(powerSetting > 1.0f) powerSetting = 1.0f;
		
		// We can only speed up or stop. Reversing would strip the gears
		if(currentSpeed < targetSpeed) bottomShooter->Set(powerSetting);
		else bottomShooter->Set(0.0f);
		
		return (Abs(targetSpeed - currentSpeed) < SHOOTERDEADBAND);
	}
	
	// This function attempts to maintain a specific encoder value on the drive block and is also similar to the steering
	float TrackDrive(int targetPosition, char motorMask)
	{
		float distance; // Store the distance of the drive wheels from their target position
		int currentPosition; // Current encoder position
		
		// Get the current encoder position
		currentPosition = driveEncoder->Get();
		
		// Calculate the absolute distance and scale to a power setting
		distance = (float)AbsI(targetPosition - currentPosition);
		distance /= DRIVEAPPROACHDISTANCE;
		if(distance > 1.0f) distance = 1.0f;
		
		// Move forward or backward at a proportional pace until within a deadband
		if(currentPosition < targetPosition - DRIVEDEADBAND) distance *= 1.0f;
		else if(currentPosition > targetPosition + DRIVEDEADBAND) distance *= -1.0f;
		else distance = 0.0f;
		
		// Move only the motors we want (either just the front, just the back, or both)
		if(motorMask == FRONT_MOTOR || motorMask == BOTH_MOTORS) frontMotor->Set(distance);
		if(motorMask == REAR_MOTOR || motorMask == BOTH_MOTORS) rearMotor->Set(distance);
		
		// Return motor speed
		return distance;
	}
	
	// Code for our first possible autonomous mode
	void AutonomousOne(void)
	{
		// Variables
		int rollerPause = MAGAZINEPAUSETIME; // The time to wait for the rollers to spin up before enabling the magazine
		int shootPause = AUTOSHOOTPAUSETIME; // The time to wait for the balls to finish shooting in autonomous
		float shooterSpeed = 0.0f; // Current voltage reading
		int encoderValue = 0; // Value from drive encoder
		
		// Reset the drive encoder and gyro
		steeringGyro->Reset();
		driveEncoder->Reset();
		driveEncoder->Start();
				
		while(IsAutonomous())
		{
			/* Tell the system that the code hasn't frozen. */
			GetWatchdog().Feed();
			
			// Message displays
			CLEARMESSAGE;
			DISPLAYPRINTF(1, "Shoot then strafe"); // Display which autonomous mode is being used
			
			// Get current encoder value and shooter speed
			encoderValue = AbsI(driveEncoder->Get());
			shooterSpeed = rollerVoltage->GetVoltage();
			
			// Set servos
			frontDriveServo->SetAngle(LOWGEARPOSITION);
			rearDriveServo->SetAngle(LOWGEARPOSITION);
			armEngageServo->SetAngle(LOWGEARPOSITION);
			
			// Wait for the rollers to spin up before engaging the magazine belts to fire
			if(rollerPause > 0) 
			{
				// Spin up the rollers to the preset speed for the key shot
				SetRollerSpeed(FIRESPEEDTWO, shooterSpeed);
				rollerPause--;
				
				// Keep the wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
			}
			
			// Fire the ball for a period of time
			else if(shootPause > 0)
			{
				// Fire
				magazineBelt->Set(Relay::kForward);
				magazineBelt2->Set(Relay::kForward);
				shootPause--;
				
				// Spin up the rollers to the preset speed for the key shot
				SetRollerSpeed(FIRESPEEDTWO, shooterSpeed);
				
				// Keep the wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
			}
			
			// Strafe after shooting for a certain distance
			else if(encoderValue < STRAFEDISTANCE)
			{
				TrackFrontSteering((int)(-PULSESPER90FRONT * 0.78));
				TrackRearSteering((int)(-PULSESPER90REAR * 0.78));
				frontMotor->Set(0.75f);
				rearMotor->Set(0.75f);
			}
			
			// Stop
			else
			{
				TrackFrontSteering(0);
				TrackRearSteering(0);
				frontMotor->Set(0.0f);
				rearMotor->Set(0.0f);
			}
		}
	}
	
	// Second of the possible autonomous modes
	void AutonomousTwo(void)
	{	
		while(IsAutonomous())
		{
			/* Tell the system that the code hasn't frozen. */
			GetWatchdog().Feed();
			
			// Display which autonomous mode we're in
			CLEARMESSAGE;
			DISPLAYPRINTF(1, "Shoot only");
			
			// Keep the wheels straight
			TrackFrontSteering(0);
			TrackRearSteering(0);
			
			// Set the servos
			frontDriveServo->SetAngle(LOWGEARPOSITION);
			rearDriveServo->SetAngle(LOWGEARPOSITION);
			armEngageServo->SetAngle(ARMDISENGAGEPOSITION);
			
			// Spin up the rollers to the preset speed for the key shot
			SetRollerSpeed(FIRESPEEDTWO, rollerVoltage->GetVoltage());
			
			// Move belts to fire
			magazineBelt->Set(Relay::kForward), magazineBelt2->Set(Relay::kForward);
			
			// Run the collectors in case another robot feeds us
			transverseCollector->Set(Relay::kReverse);
			lateralCollector->Set(COLLECTORSPEED);
		}
	}
	
	// Third of the possible autonomous modes
	void AutonomousThree(void)
	{
		// Variables
		int rollerPause = MAGAZINEPAUSETIME; // The time to wait for the rollers to spin up before enabling the magazine
		int shootPause = AUTOSHOOTPAUSETIME; // The time to wait for the balls to finish shooting in autonomous
		float gyroAngle = 0; // Current angle from home as read by the gyro
		float shooterSpeed = 0.0f; // Current voltage reading
		int armStage = 0; // Current stage for bridge manipulator
		int encoderValue = 0; // Value from drive encoder
		int downPauseCount = ARMDOWNTIME; // Amount of time to wait while arm is down
		int servoPauseCount = SERVOPAUSETIME; // Time to wait for servos to shift
		float rearSetting = LOWGEARPOSITION, frontSetting = LOWGEARPOSITION, armSetting = LOWGEARPOSITION; // Servo posiions
		
		// Reset the drive encoder and gyro
		steeringGyro->Reset();
		driveEncoder->Reset();
		driveEncoder->Start();
				
		while(IsAutonomous())
		{
			/* Tell the system that the code hasn't frozen. */
			GetWatchdog().Feed();
			
			// Display messages
			CLEARMESSAGE;
			DISPLAYPRINTF(1, "Shoot then tip bridge"); // Display which autonomous mode is being used
			DISPLAYPRINTF(2, "Gyro: %0.2f", gyroAngle);  // The float value of the gyro angle
			DISPLAYPRINTF(3, "Encoder: %d", encoderValue); // The value of the encoder
			
			// Get encoder value and roller speed
			encoderValue = AbsI(driveEncoder->Get());
			shooterSpeed = rollerVoltage->GetVoltage();
			
			// Set servos
			frontDriveServo->SetAngle(frontSetting);
			rearDriveServo->SetAngle(rearSetting);
			armEngageServo->SetAngle(armSetting);
			
			// Wait for the rollers to spin up before engaging the magazine belts to fire
			if(rollerPause > 0) 
			{
				// Spin up the rollers to the preset speed for the key shot
				SetRollerSpeed(FIRESPEEDTWO, shooterSpeed);
				rollerPause--;
				
				// Keep wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
				
				// Set servos
				frontSetting = LOWGEARPOSITION;
				rearSetting = NEUTRALGEARPOSITION;
				armSetting = ARMENGAGEPOSITION;
			}
			
			// Fire the ball
			else if(shootPause > 0)
			{
				// Fire
				magazineBelt->Set(Relay::kForward);
				magazineBelt2->Set(Relay::kForward);
				shootPause--;
				
				// Spin up the rollers to the preset speed for the key shot
				SetRollerSpeed(FIRESPEEDTWO, shooterSpeed);
				
				// Keep wheels straight
				TrackFrontSteering(0);
				TrackRearSteering(0);
				
				// Set servos
				frontSetting = LOWGEARPOSITION;
				rearSetting = LOWGEARPOSITION;
				armSetting = LOWGEARPOSITION;
			}
			
			// Wait for servos to shift
			else if(servoPauseCount > 0) 
			{
				servoPauseCount--;
				
				// Set servos
				frontSetting = LOWGEARPOSITION;
				rearSetting = NEUTRALGEARPOSITION;
				armSetting = ARMENGAGEPOSITION;
			}
			
			// Perform bridge manipulation
			else if(armStage < 3)
			{
				// Stop rollers
				SetRollerSpeed(0.0f, shooterSpeed);
			
				// Get the gyro angle
				gyroAngle = steeringGyro->GetAngle();
				
				// Get the drive encoder value
				encoderValue = AbsI(driveEncoder->Get());
				
				// Turn left or right based on gyro reading to stay straight
				if(gyroAngle + GYRODEADBANDANGLE < 0.0f) TrackRearSteering(PULSESPER90REAR / 8);
				else if(gyroAngle - GYRODEADBANDANGLE > 0.0f) TrackRearSteering(-PULSESPER90REAR / 8);
				else TrackRearSteering(0);
				
				// Keep the front wheels straight and drive forward slowly
				TrackFrontSteering(0);
				
				// Set servos
				frontSetting = LOWGEARPOSITION;
				rearSetting = NEUTRALGEARPOSITION;
				armSetting = ARMENGAGEPOSITION;
				
				// Drive forward
				frontMotor->Set(0.5f);
				
				// Drive winch until first light sensor is triggered (arm is in primed position)
				if(!armStartSensor->Get() && armStage == 0) rearMotor->Set(0.5f);
				else if(armStage == 0) armStage = 1;
				
				// Stop winch until the light sensor on the bridge dector is triggered (bridge is hit) or we travel too far on the encoder
				if(!armHitSensor->Get() && encoderValue < AUTONOMOUSDISTANCE && armStage == 1) rearMotor->Set(0.0f);
				else if(armStage == 1) armStage = 2;
				
				// Continue to run winch until bottom switch is hit
				if(!armDownSwitch->Get() && armStage == 2) rearMotor->Set(0.85f);
				else if(armStage == 2) armStage = 3;
			}
			
			// Wait for a bit
			else if(armStage == 3)
			{
				frontMotor->Set(0.0f);
				rearMotor->Set(0.0f);
				
				if(downPauseCount <= 0) armStage = 4;
				else downPauseCount--;
			}
			
			// Stop moving and return arm to hom
			else if(armStage == 4)
			{
				frontMotor->Set(0.0f);
				
				if(!armUpSwitch->Get()) rearMotor->Set(-0.2f);
				else armStage = 5;
			}
			
			// Bridge tipping is complete
			else
			{
				// Return gears to low
				frontSetting = LOWGEARPOSITION;
				rearSetting = LOWGEARPOSITION;
				armSetting = LOWGEARPOSITION;
				
				// Stop winch and drive
				frontMotor->Set(0.0f);
				rearMotor->Set(0.0f);
			}
		}
	}
}; /* This is an exception to our semi-colon rule, but its one of very few */

// Custom functions
float Abs(float number)
{
	// Return the absolute magnitude of the given number (make it positive)
	if(number < 0.0f) number  = -number;
	return number;
}

int AbsI(int number)
{
	// Return the absolute magnitude of the given integer (make it positive)
	if(number < 0) number = -number;
	return number;
}

/* The entry point is FRC_UserProgram_StartupLibraryInit. Tell the linker where our code is. */
START_ROBOT_CLASS(SimpleTracker);
