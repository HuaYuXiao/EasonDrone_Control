# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## v4.1.2 - 2024.09.09
- [CRUCIAL, bug fix] set priority of RC control to the highest
- [end support] joy stick in simulation mode

## v4.1.1 - 2024.09.07
- [CRUCIAL, bug fix] prevent performing leaked command (skip iteration if current task's done)
- [new feature]: `Return`, `Stabilized`, `Acro`, `Rattitude`, `Altitude`, `Position`
- [bug fix] switch to `Hold` once reach desired `Takeoff` height
- [bug fix] prevent unnecessary subscriber when using EKF2 fusion

## v4.1.0 - 2024.08.24
- [new feature] check whether `Armed` before `AUTO.TAKEOFF`
- [new feature] check whether `have_odom_` before executing command
- reload parameters & functions via `namespace`

## [v4.0.3] - 2024-08-12
- [bug fix]: in `Move` mode, check whether close to destination, for both pos & yaw
- [new feature] process `quadrotor_msgs::PositionCommand` from path-planner
- [new feature] set global origin via `geographic_msgs::GeoPointStamped`
- replace `Apache` lisence with `GPLv3` lisence

## [v4.0.2] - 2024-08-08
- [CRUCIAL, new feature]: `Move` via `quadrotor_msgs::PositionCommand`
- [end support]: `Drone_odom`
- [end support]: unused header files

## [v4.0.1] - 2024-08-08
- [CRUCIAL, new feature]: `AUTO.TAKEOFF` via mavros
- [CRUCIAL, new feature]: `AUTO.LAND` via mavros

## [v4.0.0] - 2024-08-07
- [CRUCIAL, new feature]: `Offboard` via mavros
- [end support]: ignore all other cmds if `Disarm` received

## [v3.6.3] - 2024-08-06
- [CRUCIAL, new feature]: `AUTO.LAND` via mavros
- replace `.yaml` with `#define`

## [v3.6.2] - 2024-08-06
- [end support]: check for geofence
- [end support]: `easondrone_msgs::ControlCommand::Idle`
- [end support]: fake_odom, Optitrack, TFmini
- update `px4ctrl_terminal` to "cpp + h" format

## [v3.6.1] - 2024-08-05
- [new feature]: support `VINS-Fusion`: `/vins_estimator/odometry` to `/mavros/vision_pose/pose`
- [remove support]: `cartographer`, `outdoor`
- [end support]: offset for position and orientation

## [v3.6.0] - 2024-07-26
- [CRUCIAL, new feature]: direct publish `/easondrone/control_command` from `/move_base_simple/goal`
- [remove support]: cmd from `/easondrone/control_command_station`

## [v3.5.2] - 2024-07-26
- [CRUCIAL, new feature]: support `FAST-LIO2`: `/Odometry` to `/mavros/vision_pose/pose`
- [end support]: euler angle estimate
- [not working]: directly remap `nav_msgs::Odometry` to `/mavros/odometry/out`

## v3.5.1:
- OFFBOARD & arm with easondrone_msgs::ControlCommand::OFFBOARD_ARM
- [end support]: check for Command_ID
- [end support]: `time_from_start`
- [not working]: launch for joy_node

## v3.5.0:
- [CRUCIAL]: replace `command_to_mavros` with `setpoint_raw_attitude_pub_`, `arming_client_`, `set_mode_client_`
- remove `state_from_mavros`

## v3.4.4:
- update `px4ctrl_control` to "cpp + h" format

## v3.4.3: 
- remove: move along with trajectory
- remove: move publisher to rviz
- [end support]: agent_num in joy_node
- [remove support]: keyboard_control

## v3.4.2: 
- update odometry source of gazebo

## v3.4.1: 
- [end support]: `message_pub`

## v3.4.0: 
- (bug fix on 2025.02.03) support PID controller with `POS+VEL` loop

## v3.3.4: 
- (removed in v3.5.3) update `rate_hz_` 

## v3.3.3:
- update `get_ref_pose_rviz`

## v3.3.2: 
- add support for `launch`

## v3.3.1: 
- [new feature] (removed in v4.0.0): support move mode `POS_VEL_ACC`

## v3.3.0: 
- [new feature] (removed in v4.0.0): support move mode `XYZ_VEL`

## v3.2.1: 
- [new feature]: catch invalid input of terminal control

## v2.0.0: 
- [new feature] (removed in v4.0.0): control type `NE`, `PID`, `UDE`

## v1.2.0: 
- [new feature]  (removed in v3.6.1) position offset

## v1.0.0: 
- [new feature]: `VICON`
