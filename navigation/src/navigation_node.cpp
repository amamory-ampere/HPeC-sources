/*************************************************************************************
 * File   : navigation_nodel.cpp, file to create easily ros nodes for HPeC
 * Copyright (C) 2018 Lab-STICC Laboratory
 * Author(s) :  Erwan Moréac, erwan.moreac@univ-ubs.fr (EM)
 *
 * This model allows the user to create an application ros node that 
 * the adaptation manager can handle in the HPeC project.
 * 
 * This model must be used by copying the folder, then rename the package and files
 * according to your needs.
 * 
 * You can test this program:
 * 	1. run on a terminal "roscore" to get a ROS master
 *  2. on another terminal, run the command "rosrun navigation_nodel navigation_nodel_node"
 *************************************************************************************/
#include "navigation_node.h"


void state_cb(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}

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

	ros::Subscriber state_sub = nh.subscribe<mavros_msgs::State>
            ("mavros/state", 10, state_cb);
    ros::Publisher local_pos_pub = nh.advertise<geometry_msgs::PoseStamped>
            ("mavros/setpoint_position/local", 10);
    ros::ServiceClient arming_client = nh.serviceClient<mavros_msgs::CommandBool>
            ("mavros/cmd/arming");
	ros::ServiceClient takeoff_client = nh.serviceClient<mavros_msgs::CommandTOL>
            ("/mavros/cmd/takeoff");
	ros::ServiceClient landing_client = nh.serviceClient<mavros_msgs::CommandTOL>
            ("/mavros/cmd/land");
    ros::ServiceClient set_mode_client = nh.serviceClient<mavros_msgs::SetMode>
            ("mavros/set_mode");

	ros::Subscriber pos_sub = nh.subscribe("mavros/global_position/global", 
								1000, 
								gps_callback);
	
	//the setpoint publishing rate MUST be faster than 2Hz
    ros::Rate rate(20.0);


	ROS_INFO("Wait 1 second!");

	ros::Duration(1).sleep(); // sleep for 1 second

    // wait for FCU connection
    while(ros::ok() && !current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

	ROS_INFO("Navigation connected!");

    geometry_msgs::PoseStamped pose;
    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 2;

    //send a few setpoints before starting
    for(int i = 50; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }
	
	mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "GUIDED";

    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;
	mavros_msgs::CommandTOL takeoff_cmd;
	takeoff_cmd.request.altitude 	= 613.448;
	takeoff_cmd.request.latitude 	= -35.363;
	takeoff_cmd.request.longitude 	= 149.165;
	mavros_msgs::CommandTOL landing_cmd;
	landing_cmd.request.altitude 	= 0;
	landing_cmd.request.latitude 	= 0;
	landing_cmd.request.longitude 	= 0;
	

	bool flying = false;

    ros::Time last_request = ros::Time::now();
	ros::Time test_pose    = ros::Time::now();

	geometry_msgs::PoseStamped positest;
    positest.pose.position.x = 10;
    positest.pose.position.y = 10;
    positest.pose.position.z = 20;

    while(ros::ok()){
        if( current_state.mode != "GUIDED" &&
            (ros::Time::now() - last_request > ros::Duration(5.0)))
		{
            if( set_mode_client.call(offb_set_mode) &&
                offb_set_mode.response.mode_sent)
			{
                ROS_INFO("GUIDED enabled");
				ROS_INFO("Altitude = %f Longitude = %f Latitude = %f ",altitude, longitude, latitude);
            }
            last_request = ros::Time::now();
        } 
		else
		{
            if( !current_state.armed &&
                (ros::Time::now() - last_request > ros::Duration(5.0)))
			{
                if( arming_client.call(arm_cmd) &&
                    arm_cmd.response.success)
				{
                    ROS_INFO("Vehicle armed");
					ROS_INFO("Altitude = %f Longitude = %f Latitude = %f ",altitude, longitude, latitude);
                }
                last_request = ros::Time::now();
            }

			if( current_state.armed &&  altitude < (DEFAULT_HEIGHT + 2) &&
				current_state.mode == "GUIDED" &&
                (ros::Time::now() - last_request > ros::Duration(0.2)))
			{
                if( takeoff_client.call(takeoff_cmd) &&
                    takeoff_cmd.response.success)
				{
                    ROS_INFO("Takeoff started");
					ROS_INFO("Altitude = %f Longitude = %f Latitude = %f ",altitude, longitude, latitude);
					flying = true;
					pose.pose.position.x = 0;
    				pose.pose.position.y = 0;
    				pose.pose.position.z = 10;
                }
                last_request = ros::Time::now();
            }

			/*if( current_state.armed && flying &&
                (ros::Time::now() - last_request > ros::Duration(10.0)))
			{
                if( landing_client.call(landing_cmd) &&
                    landing_cmd.response.success)
				{
                    ROS_INFO("Landing started");
					ROS_INFO("Altitude = %f Longitude = %f Latitude = %f ",altitude, longitude, latitude);
					flying = false;
					pose.pose.position.x = 0;
    				pose.pose.position.y = 0;
    				pose.pose.position.z = 0;
                }
                last_request = ros::Time::now();
            }*/


        }

		/*if(ros::Time::now() - test_pose > ros::Duration(20.0))
        	local_pos_pub.publish(positest);
		else*/
			local_pos_pub.publish(pose);

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}



/*******************************************************************
 * imu_callback
 * Author : EM 
 * @param imu_msg, UAV GPS position from topic listened
 * 
 * Callback function to get IMU data
*******************************************************************/
void imu_callback(const sensor_msgs::Imu::ConstPtr &imu_msg)
{
    printf("\nSeq: [%d]", imu_msg->header.seq);
    printf("\nOrientation-> x: [%f], y: [%f], z: [%f], w: [%f]",
			 imu_msg->orientation.x, imu_msg->orientation.y, 
			 imu_msg->orientation.z, imu_msg->orientation.w);

   	double quatx= imu_msg->orientation.x;
   	double quaty= imu_msg->orientation.y;
   	double quatz= imu_msg->orientation.z;
   	double quatw= imu_msg->orientation.w;

    tf::Quaternion q(quatx, quaty, quatz, quatw);
    tf::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);

    printf("\nRoll: [%f],Pitch: [%f],Yaw: [%f]",roll,pitch,yaw);
    return ;

}

/*******************************************************************
 * gps_callback
 * Author : EM 
 * @param position, UAV GPS position from topic listened
 * 
 * Callback function to get GPS position
 * Here, only altitude is used but there are other data on position
 * (i) Sample code, you can erase if useless
*******************************************************************/
void gps_callback(const sensor_msgs::NavSatFix::ConstPtr &position)
{
   altitude  = position->altitude;
   latitude  = position->latitude;
   longitude = position->longitude;
}

