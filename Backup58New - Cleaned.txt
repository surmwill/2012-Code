/* Contains the macro definitions for testing on the new robot. */

#warning "New robot"

// Button macros
#define SWERVEMODEBUTTON rightStick->GetRawButton(1)
#define MONSTERMODEBUTTON leftStick->GetRawButton(1)
#define CARMODEBUTTON rightStick->GetRawButton(6)
#define LOCKMODEBUTTON rightStick->GetRawButton(5)
#define BRAKEMODEBUTTON leftStick->GetRawButton(2)
#define TANKTURNINGMODEBUTTON rightStick->GetRawButton(3)
#define REVERSEDRIVEBUTTON speedStick->GetRawButton(9)

#define MANUALALIGNENABLEBUTTON (leftStick->GetRawButton(8) && leftStick->GetRawButton(9))
#define MANUALALIGNLEFTREAR leftStick->GetRawButton(7)
#define MANUALALIGNRIGHTREAR leftStick->GetRawButton(10)
#define MANUALALIGNLEFTFRONT leftStick->GetRawButton(6)
#define MANUALALIGNRIGHTFRONT leftStick->GetRawButton(11)

#define HIGHGEARBUTTON leftStick->GetRawButton(5)
#define LOWGEARBUTTON leftStick->GetRawButton(4)
#define NEUTRALGEARBUTTON leftStick->GetRawButton(3)
#define ARMUNWINDBUTTON rightStick->GetRawButton(11)

#define BRIDGEMANIPULATEBUTTON speedStick->GetRawButton(7)
#define PIVOTMODEBUTTON rightStick->GetRawButton(2)

#define SHOOTBUTTON (speedStick->GetRawButton(1) && (speedStick->GetRawButton(3) || speedStick->GetRawButton(4) || speedStick->GetRawButton(5) || speedStick->GetRawButton(8)))
#define FIRSTFIREBUTTON speedStick->GetRawButton(4)
#define SECONDFIREBUTTON speedStick->GetRawButton(3)
#define THIRDFIREBUTTON speedStick->GetRawButton(5)
#define FOURTHFIREBUTTON speedStick->GetRawButton(8)

#define MAGAZINEENABLEBUTTON speedStick->GetRawButton(11)
#define MAGAZINEREVERSEBUTTON speedStick->GetRawButton(10)
#define COLLECTORENABLEBUTTON speedStick->GetRawButton(11)
#define COLLECTORREVERSEBUTTON speedStick->GetRawButton(10)

// Port numbers
// Motors are on PWM ports
#define FRONTMOTORPORT 3 
#define REARMOTORPORT 4 
#define FRONTSTEERINGMOTORPORT 2
#define REARSTEERINGMOTORPORT 1
#define BOTTOMSHOOTINGMOTORPORT 9
#define LATERALCOLLECTORPORT 10

// The gear-shifting servos are on PWM ports
#define FRONTSERVOPORT 5
#define REARSERVOPORT 6
#define ARMENGAGESERVOPORT 8

// Joysticks are on USB ports
#define LEFTJOYSTICKPORT 4
#define RIGHTJOYSTICKPORT 1
#define SPEEDJOYSTICKPORT 3

// Encoders are on Digital IO ports
#define FRONTSTEERINGENCODERPORTA 3
#define FRONTSTEERINGENCODERPORTB 4
#define REARSTEERINGENCODERPORTA 1
#define REARSTEERINGENCODERPORTB 2
#define DRIVEENCODERPORTA 14
#define DRIVEENCODERPORTB 13

// Limit switches are digital inputs 
#define ARMDOWNSWITCHPORT 7
#define ARMUPSWITCHPORT 8

// Light sensors are also digital inputs
#define ARMSTARTPOSITIONPORT 9
#define ARMHITPOSITIONPORT 10

// Analogue channels
#define STEERINGGYROPORT 2
#define ANALOGCHANNELPORT 4

// The autonomous switches enable/disable digital inputs
#define AUTONOMOUSSWITCHPORTONE 5
#define AUTONOMOUSSWITCHPORTTWO 6

// Relays have their own ports
#define MAGAZINERELAYPORT 2
#define MAGAZINERELAY2PORT 5
#define TRANSVERSECOLLECTORPORT 1 

// Constants
// Hardware constants
#define GYROSENSITIVITY 0.007f // Volts per degree per second
#define PULSESPER90FRONT 1350 // Number of encoder pulses in a 90-degree turn (front steering)
#define PULSESPER90REAR 600 // Number of encoder pulses in a 90-degree turn (rear steering)
#define STEERINGDEADBANDFRONT 40 // Number of encoder pulses of proximity needed for the front steering
#define STEERINGAPPROACHDISTANCEFRONT 100 // Number of encoder pulses of proximity after which the wheels begin to slow down
#define STEERINGDEADBANDREAR 10 // Number of encoder pulses of proximity needed for the rear steering
#define STEERINGAPPROACHDISTANCEREAR 60 // Number of encoder pulses of proximity after which the wheels begin to slow down
#define MAGAZINEALTERNATORLIMIT 2000 // The number of times the IsOpteratorControl() loop must run before the magazine alternator resets
#define MAGAZINEALTERNATORLIMITONE 1800 // The number of times the IsOperatorControl() loop must run before the magazine reverses

// Mask values for TrackDrive() function (a mask, because 1 OR 2 == 3)
#define FRONT_MOTOR 1 // Tells the TrackDrive() Function to run the front motor
#define REAR_MOTOR 2 // Tells the TrackDrive() Function to run the rear motor
#define BOTH_MOTORS 3 // Tells the TrackDrive() Function to run both motors

// Hardware threshold constants
#define JOYSTICKDEADBANDX 0.08f // Minimum detectable joystick deflection
#define JOYSTICKDEADBANDY 0.08f
#define GYRODEADBANDANGLE 3.0f // Allowable error for gyro angle when tracking
#define DRIVEAPPROACHDISTANCE 70.0f // Distance at which the drive motors begin to slow during brake mode
#define DRIVEDEADBAND 2.0f // Allowable error for brake mode encoder

// Other constants
#define Pi 3.1415926535897932384626433832795028841971f
#define SWERVEMODE 0 // Drive modes
#define MONSTERMODE 1
#define CARMODE 2
#define LOWGEARPOSITION 160.0f // Angles for the gear-shifting servos
#define HIGHGEARPOSITION 70.0f
#define NEUTRALGEARPOSITION 110.0f
#define ARMENGAGEPOSITION 70.0f
#define ARMDISENGAGEPOSITION 120.0f
#define MAGAZINEREVERSETIME 700 // Time for magazine to reverse after running
#define SERVOPAUSETIME 1000 // Time to wait for arm-engaging servo to shift
#define AUTONOMOUSDISTANCE 1900 // Maximum distance to travel in autonomous
#define MAGAZINEPAUSETIME 400 // Amount of time to wait during autonomous for rollers to spin up to speed
#define AUTOSHOOTPAUSETIME 13000 // Amount of time to wait during autonomous before moving (third mode only)
#define SHOOTERAPPROACHSPEED 0.60f // Distance from target speed at which the rollers begin to ramp down
#define SHOOTERCONSTANTSPEED 0.1f // Minimum power value for the shooter (a maintenance value)
#define SHOOTERDEADBAND 0.5f // Distance from target speed at which the roller speed is considered to be within a good enough threshold
#define COLLECTORSPEED -1.00f // Speed of collector motors
#define STRAFEDISTANCE 1900 // Distance to strafe in first autonomous mode
#define ARMDOWNTIME 8000 // Time in autonomous mode to pause when the arm first lowers

// Shooter constants
#define FIRESPEEDONE 5.65f // Pre-set firing speeds
#define FIRESPEEDTWO 6.80f
#define FIRESPEEDTHREE 3.2f
#define FIRESPEEDFOUR 2.0f
