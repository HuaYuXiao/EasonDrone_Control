/***************************************************************************************************************************
* terminal_control.cpp
*
* Author: Qyp
* Edited by: Eason Hua
* Update Time: 2024.08.05
*
* Introduction:  test function for sending ControlCommand.msg
***************************************************************************************************************************/

#include <ros/ros.h>
#include <iostream>
#include <Eigen/Eigen>

#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <nav_msgs/Path.h>

#include "KeyboardEvent.h"
#include <easondrone_msgs/ControlCommand.h>

using namespace std;


//即将发布的command
easondrone_msgs::ControlCommand Command_to_pub;

//发布
ros::Publisher easondrone_ctrl_pub_;


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>　函数声明　<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void mainloop();

void generate_com(int Move_mode, float state_desired[4]);


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>　主函数　<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int main(int argc, char **argv){
    ros::init(argc, argv, "terminal_control");
    ros::NodeHandle nh;

    //　【发布】控制指令
    easondrone_ctrl_pub_ = nh.advertise<easondrone_msgs::ControlCommand>
            ("/easondrone/control_command", 10);

    // 初始化命令 - Idle模式 电机怠速旋转 等待来自上层的控制指令
    Command_to_pub.Mode                                = easondrone_msgs::ControlCommand::Idle;
    Command_to_pub.Reference_State.Move_mode           = easondrone_msgs::PositionReference::XYZ_POS;
    Command_to_pub.Reference_State.Move_frame          = easondrone_msgs::PositionReference::ENU_FRAME;

    //固定的浮点显示
    cout.setf(ios::fixed);
    //setprecision(n) 设显示小数精度为n位
    cout << setprecision(2);
    //左对齐
    cout.setf(ios::left);
    // 强制显示小数点
    cout.setf(ios::showpoint);
    // 强制显示符号
    //cout.setf(ios::showpos);

    mainloop();

    return 0;
}

void mainloop(){
    int Control_Mode = 0;
    bool valid_Control_Mode = false;
    int Move_mode = 0;
    bool valid_move_mode = false;
    int Move_frame = 0;
    bool valid_move_frame = false;
    float state_desired[4];
    bool valid_x_input = false;
    bool valid_y_input = false;
    bool valid_z_input = false;
    bool valid_yaw_input = false;

    while(ros::ok()){
        while (!valid_Control_Mode){
            cout << "-------- EasonDrone Terminal--------" << endl;
            cout << "Please choose the Command.Mode: " << endl;
            cout << "0 for IDLE, 1 for TAKEOFF, 2 for HOLD, 3 for LAND, " << endl;
            cout << "4 for MOVE, 5 for DISARM, 6 - ARM & OFFBOARD" << endl;
            if (cin >> Control_Mode) {
                if (Control_Mode == 0 ||
                    Control_Mode == 1 ||
                    Control_Mode == 2 ||
                    Control_Mode == 3 ||
                    Control_Mode == 4 ||
                    Control_Mode == 5 ||
                    Control_Mode == 6) {
                    valid_Control_Mode = true;
                }
                else{
                    cout << "Invalid input! Please enter a valid command mode." << endl;
                }
            }
            else {
                // Clear error flags
                cin.clear();
                // Discard invalid input
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input! Please enter an integer." << endl;
            }
        }
        valid_Control_Mode = false;

        switch (Control_Mode){
            case easondrone_msgs::ControlCommand::Idle:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Idle;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case 6:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::OFFBOARD_ARM;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case easondrone_msgs::ControlCommand::Takeoff:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Takeoff;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case easondrone_msgs::ControlCommand::Hover:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Hover;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case easondrone_msgs::ControlCommand::Land:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Land;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case easondrone_msgs::ControlCommand::Move:{
                while (!valid_move_mode) {
                    cout << "Please choose the Command.Reference_State.Move_mode: 0 POS, 1 XY_POS_Z_VEL, 2 XY_VEL_Z_POS, 3 VEL" << endl;
                    if (cin >> Move_mode) {
                        if (Move_mode == 0 ||
                            Move_mode == 1 ||
                            Move_mode == 2 ||
                            Move_mode == 3) {
                            valid_move_mode = true;
                        } else {
                            cout << "Invalid input! Please enter a valid Move_mode." << endl;
                        }
                    } else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter an integer." << endl;
                    }
                }
                valid_move_mode = false;

                while (!valid_move_frame) {
                    cout << "Please choose the Command.Reference_State.Move_frame: 0 for ENU_FRAME, 1 for BODY_FRAME" << endl;
                    if (cin >> Move_frame) {
                        if (Move_frame == 0 || Move_frame == 1) {
                            valid_move_frame = true;
                        } else {
                            cout << "Invalid input! Please enter 0 or 1." << endl;
                        }
                    } else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter an integer." << endl;
                    }
                }
                valid_move_frame = false;

                cout << "Please input the reference state [x y z yaw]: " << endl;

                while (!valid_x_input) {
                    cout << "setpoint_t[0] --- x [m] : " << endl;
                    if (cin >> state_desired[0]) {
                        valid_x_input = true;
                    }
                    else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number." << endl;
                    }
                }
                valid_x_input = false;

                while (!valid_y_input) {
                    cout << "setpoint_t[1] --- y [m] : " << endl;
                    if (cin >> state_desired[1]) {
                        valid_y_input = true;
                    }
                    else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number." << endl;
                    }
                }
                valid_y_input = false;

                while (!valid_z_input) {
                    cout << "setpoint_t[2] --- z [m] : " << endl;
                    if (cin >> state_desired[2]) {
                        if (state_desired[2] >= 0.0) {
                            valid_z_input = true;
                        }
                        else {
                            cout << "Invalid input for z! Please enter a non-negative value" << endl;
                        }
                    }
                    else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number." << endl;
                    }
                }
                valid_z_input = false;

                while (!valid_yaw_input) {
                    cout << "setpoint_t[3] --- yaw [deg] : " << endl;
                    if (cin >> state_desired[3]) {
                        // Check if yaw is within the range
                        if (state_desired[3] >= -180 && state_desired[3] < 180) {
                            valid_yaw_input = true;
                        }
                        else {
                            cout << "Invalid input for yaw! Please enter a value between -180 and 180" << endl;
                        }
                    }
                    else {
                        // Clear error flags
                        cin.clear();
                        // Discard invalid input
                        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        cout << "Invalid input! Please enter a number." << endl;
                    }
                }
                valid_yaw_input = false;

                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Move;
                Command_to_pub.Reference_State.Move_mode = Move_mode;
                Command_to_pub.Reference_State.Move_frame = Move_frame;
                // yaw_rate control
                // Command_to_pub.Reference_State.Yaw_Rate_Mode = 1;
                generate_com(Move_mode, state_desired);

                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }

            case easondrone_msgs::ControlCommand::Disarm:{
                Command_to_pub.header.stamp = ros::Time::now();
                Command_to_pub.Mode = easondrone_msgs::ControlCommand::Disarm;
                easondrone_ctrl_pub_.publish(Command_to_pub);

                break;
            }
        }

        cout << "-------- MISSION RECEIVED --------\n" << endl;
    }
}

void generate_com(int Move_mode, float state_desired[4]){
    //# Move_mode 2-bit value:
    //# 0 for position, 1 for vel, 1st for xy, 2nd for z.
    //#                   xy position     xy velocity
    //# z position       	0b00(0)       0b10(2)
    //# z velocity		0b01(1)       0b11(3)

    if(Move_mode == easondrone_msgs::PositionReference::XYZ_ACC){
        cout << "ACC control not support yet." <<endl;
    }

    if((Move_mode & 0b10) == 0) //xy channel
    {
        Command_to_pub.Reference_State.position_ref[0] = state_desired[0];
        Command_to_pub.Reference_State.position_ref[1] = state_desired[1];
        Command_to_pub.Reference_State.velocity_ref[0] = 0;
        Command_to_pub.Reference_State.velocity_ref[1] = 0;
    }
    else{
        Command_to_pub.Reference_State.position_ref[0] = 0;
        Command_to_pub.Reference_State.position_ref[1] = 0;
        Command_to_pub.Reference_State.velocity_ref[0] = state_desired[0];
        Command_to_pub.Reference_State.velocity_ref[1] = state_desired[1];
    }

    if((Move_mode & 0b01) == 0) //z channel
    {
        Command_to_pub.Reference_State.position_ref[2] = state_desired[2];
        Command_to_pub.Reference_State.velocity_ref[2] = 0;
    }
    else{
        Command_to_pub.Reference_State.position_ref[2] = 0;
        Command_to_pub.Reference_State.velocity_ref[2] = state_desired[2];
    }

    Command_to_pub.Reference_State.acceleration_ref[0] = 0;
    Command_to_pub.Reference_State.acceleration_ref[1] = 0;
    Command_to_pub.Reference_State.acceleration_ref[2] = 0;

    if(Command_to_pub.Reference_State.Yaw_Rate_Mode == 1){
        Command_to_pub.Reference_State.yaw_rate_ref = state_desired[3];
    }
    else{
        Command_to_pub.Reference_State.yaw_ref = state_desired[3]/180.0*M_PI;
    }
}
