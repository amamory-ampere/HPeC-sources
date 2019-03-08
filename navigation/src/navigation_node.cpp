/* 
 * This file is part of the HPeC distribution (https://github.com/Kamiwan/HPeC-sources).
 * Copyright (c) 2018 Lab-STICC Laboratory.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
/*************************************************************************************
 * File   : navigation_node.cpp,
 * Copyright (C) 2018 Lab-STICC Laboratory
 * Author(s) :  Erwan Moréac, erwan.moreac@univ-ubs.fr (EM)
 * Created on: February 21, 2019 
 *
 *************************************************************************************/
#include "navigation_node.hpp"



/**##################################################################
 * MAIN
 * Author : EM 
 * 
 * Launch the ROS node and subscribe to topics in order to wait 
 * for orders.
##################################################################**/
int main(int argc, char **argv)
{
    ros::init(argc, argv, "navigation_node");
    ros::NodeHandle nh;

    //the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20.0);

    NavCommand my_nav_command(&nh);
    
    while(ros::ok())
    {
        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}



    //EM, Move UAV with velocity commands
    /*geometry_msgs::TwistStamped vel_msg;
    vel_msg.header.stamp = ros::Time::now();
    vel_msg.header.frame_id = "fcu";
    vel_msg.twist.linear.y = 2;

    ROS_INFO("SENDING VELOCITY COMMANDS");
    for(int i = 100; ros::ok() && i > 0; --i){
        //local_pos_pub.publish(pose);
        //global_pos_pub.publish(target);
        if(i<50)
            vel_msg.twist.linear.y = -2;

        cmd_vel_pub.publish(vel_msg);
        ros::spinOnce();
        rate.sleep();
    }

    ROS_INFO("NEW GLOBAL position published");*/

    
