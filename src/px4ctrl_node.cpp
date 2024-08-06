/***************************************************************************************************************************
* px4_pos_controller.cpp
*
* Author: Qyp
* Maintainer: Eason Hua
// last updated on 2024.08.06
*
* Introduction:  PX4 Position Controller 
*         1. 从应用层节点订阅/easondrone/control_command话题（ControlCommand.msg），接收来自上层的控制指令
*         2. 调用位置环控制算法，计算加速度控制量，并转换为期望角度。（可选择cascade_PID, PID, UDE, passivity-UDE, NE+UDE位置控制算法）
*         3. 通过command_to_mavros.h将计算出来的控制指令发送至飞控（通过mavros包发送mavlink消息）
*         4. PX4固件通过mavlink_receiver.cpp接收该mavlink消息。
***************************************************************************************************************************/

#include "px4ctrl_node.h"


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>主 函 数<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(int argc, char **argv){
    ros::init(argc, argv, "px4ctrl_node");
    ros::NodeHandle nh("~");

    state_sub = nh.subscribe<mavros_msgs::State>
            ("/mavros/state", 10, state_cb);
    odom_sub_ = nh.subscribe<nav_msgs::Odometry>
            ("/mavros/local_position/odom", 10, odometryCallback);
    //【订阅】指令 本话题为任务模块生成的控制指令
    easondrone_ctrl_sub_ = nh.subscribe<easondrone_msgs::ControlCommand>
            ("/easondrone/control_command", 10, easondrone_ctrl_cb_);
    //【订阅】无人机状态 本话题来自px4_pos_estimator.cpp
    drone_state_sub = nh.subscribe<easondrone_msgs::DroneState>
            ("/easondrone/drone_state", 10, drone_state_cb);

    local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
            ("mavros/setpoint_position/local", 10);
    // 【发布】角度/角速度期望值 坐标系 ENU系
    //  本话题要发送至飞控(通过Mavros功能包 /plugins/setpoint_raw.cpp发送), 对应Mavlink消息为SET_ATTITUDE_TARGET (#82), 对应的飞控中的uORB消息为vehicle_attitude_setpoint.msg（角度） 或vehicle_rates_setpoint.msg（角速度）
    setpoint_raw_attitude_pub_ = nh.advertise<mavros_msgs::AttitudeTarget>
            ("/mavros/setpoint_raw/attitude", 10);
    //【发布】位置控制器的输出量:期望姿态
    att_ref_pub = nh.advertise<easondrone_msgs::AttitudeReference>
            ("/easondrone/control/attitude_reference", 10);

    // 【服务】解锁/上锁 本服务通过Mavros功能包 /plugins/command.cpp 实现
    arming_client = nh.serviceClient<mavros_msgs::CommandBool>
            ("/mavros/cmd/arming");
    // 【服务】修改系统模式 本服务通过Mavros功能包 /plugins/command.cpp 实现
    arming_client = nh.serviceClient<mavros_msgs::SetMode>
            ("/mavros/set_mode");

    dt = 0.02;

    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20.0);

    // wait for FCU connection
    while(ros::ok() && !current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

    geometry_msgs::PoseStamped pose;
    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 0;

    //send a few setpoints before starting
    for(int i = 100; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }

    offb_set_mode.request.custom_mode = "OFFBOARD";

    arm_cmd.request.value = true;

    // 用于与mavros通讯的类，通过mavros发送控制指令至飞控【本程序->mavros->飞控】
    command_to_mavros _command_to_mavros;

    // 位置控制器声明 可以设置自定义位置环控制算法
    pos_controller_cascade_PID pos_controller_cascade_pid;

    // 初始化命令
    Command_Now.Mode                                = easondrone_msgs::ControlCommand::Move;
    Command_Now.Reference_State.Move_mode           = easondrone_msgs::PositionReference::XYZ_POS;
    Command_Now.Reference_State.Move_frame          = easondrone_msgs::PositionReference::ENU_FRAME;

    // 记录启控时间
    ros::Time begin_time = ros::Time::now();
    float last_time = control_utils::get_time_in_sec(begin_time);

    cout << "[px4ctrl_node] controller initialized" << endl;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>主  循  环<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    while(ros::ok()){
        // 当前时间
        cur_time = control_utils::get_time_in_sec(begin_time);
        dt = cur_time - last_time;
        dt = constrain_function2(dt, 0.008, 0.012);
        last_time = cur_time;

        //执行回调函数
        ros::spinOnce();

        switch (Command_Now.Mode){
            case easondrone_msgs::ControlCommand::Arm:{
                ROS_INFO("FSM_EXEC_STATE: Arm");

                if (!current_state.armed) {
                    arm_cmd.request.value = true;

                    if (arming_client.call(arm_cmd) && arm_cmd.response.success) {
                        cout_color("Vehicle Armed", GREEN_COLOR);
                    }
                }

                break;
            }

            case easondrone_msgs::ControlCommand::Offboard:{
                ROS_INFO("FSM_EXEC_STATE: Offboard");

                if (current_state.mode != "OFFBOARD") {
                    offb_set_mode.request.custom_mode = "OFFBOARD";

                    if (arming_client.call(offb_set_mode) && offb_set_mode.response.mode_sent) {
                        cout_color("Offboard enabled", GREEN_COLOR);
                    }
                }

                break;
            }

            // 【Takeoff】 从摆放初始位置原地起飞至指定高度，偏航角也保持当前角度
            case easondrone_msgs::ControlCommand::Takeoff:{
                if (Command_Last.Mode != easondrone_msgs::ControlCommand::Takeoff){
                    cout << "[px4ctrl] Takeoff to desired point" << endl;
                    // 设定起飞位置
                    Takeoff_position[0] = odom_pos_[0];
                    Takeoff_position[1] = odom_pos_[1];
                    Takeoff_position[2] = odom_pos_[2];

                    //
                    Command_Now.Reference_State.Move_mode       = easondrone_msgs::PositionReference::XYZ_POS;
                    Command_Now.Reference_State.Move_frame      = easondrone_msgs::PositionReference::ENU_FRAME;
                    Command_Now.Reference_State.position_ref[0] = Takeoff_position[0];
                    Command_Now.Reference_State.position_ref[1] = Takeoff_position[1];
                    Command_Now.Reference_State.position_ref[2] = Takeoff_position[2] + Takeoff_height_;
                    Command_Now.Reference_State.yaw_ref         = odom_yaw_;
                }

                break;
            }

            // 【Move】 ENU系移动。只有PID算法中才有追踪速度的选项，其他控制只能追踪位置
            case easondrone_msgs::ControlCommand::Move:{
                //对于机体系的指令,需要转换成ENU坐标系执行,且同一ID号内,只执行一次.
                if(Command_Now.Reference_State.Move_frame != easondrone_msgs::PositionReference::ENU_FRAME){
                    Body_to_ENU();
                }

                break;
            }

            // 【Hold】 悬停。当前位置悬停
            case easondrone_msgs::ControlCommand::Hold:{
                Command_Now.Reference_State.Move_mode       = easondrone_msgs::PositionReference::XYZ_POS;
                Command_Now.Reference_State.Move_frame      = easondrone_msgs::PositionReference::ENU_FRAME;
                Command_Now.Reference_State.position_ref[0] = odom_pos_[0];
                Command_Now.Reference_State.position_ref[1] = odom_pos_[1];
                Command_Now.Reference_State.position_ref[2] = odom_pos_[2];
                Command_Now.Reference_State.yaw_ref         = odom_yaw_; //rad

                break;
            }

            // 【Land】 降落。当前位置原地降落，降落后会自动上锁，且切换为mannual模式
            case easondrone_msgs::ControlCommand::Land:{
                ROS_INFO("FSM_EXEC_STATE: Land");

                if (current_state.mode != "AUTO.LAND") {
                    offb_set_mode.request.custom_mode = "AUTO.LAND";

                    if (arming_client.call(offb_set_mode) && offb_set_mode.response.mode_sent) {
                        cout_color("AUTO.LAND enabled", GREEN_COLOR);
                    }
                }

                break;
            }

            case easondrone_msgs::ControlCommand::Manual:{
                if (current_state.mode == "OFFBOARD") {
                    offb_set_mode.request.custom_mode = "MANUAL";

                    if (arming_client.call(offb_set_mode) && offb_set_mode.response.mode_sent) {
                        cout_color("MANUAL enabled", GREEN_COLOR);
                    }
                }
                break;
            }

            // 【Disarm】 上锁
            case easondrone_msgs::ControlCommand::Disarm:{
                ROS_INFO("FSM_EXEC_STATE: DISARM");

                if (current_state.armed) {
                    arm_cmd.request.value = false;

                    if (arming_client.call(arm_cmd) && arm_cmd.response.success) {
                        cout_color("Vehicle Disarmed", GREEN_COLOR);
                    }
                }

                break;
            }
        }

        //执行控制
        _ControlOutput = pos_controller_cascade_pid.pos_controller(_DroneState, Command_Now.Reference_State, dt);

        throttle_sp[0] = _ControlOutput.Throttle[0];
        throttle_sp[1] = _ControlOutput.Throttle[1];
        throttle_sp[2] = _ControlOutput.Throttle[2];

        _AttitudeReference = control_utils::ThrottleToAttitude(throttle_sp, Command_Now.Reference_State.yaw_ref);

        //发送解算得到的期望姿态角至PX4
        _command_to_mavros.send_attitude_setpoint(_AttitudeReference);

        //发布期望姿态
        att_ref_pub.publish(_AttitudeReference);

        Command_Last = Command_Now;
        rate.sleep();
    }

    return 0;
}
