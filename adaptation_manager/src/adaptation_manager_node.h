//TODO Insert header comment

#ifndef ADAPTATION_MANAGER_NODE_H
#define ADAPTATION_MANAGER_NODE_H

#include <ros/ros.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/image_encodings.h>
#include <ros/callback_queue.h>
#include <boost/thread.hpp>
#include "std_msgs/Int32.h"
#include "std_msgs/Float32.h"
#include <sys/wait.h>

#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <iosfwd>

#include <iostream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "initimg.h"
#include "fpga_header.h"

extern"C"{
	#include "main.h"
	//#include "call.h"
}

using namespace std; 
using namespace cv;

extern struct timeval  beginning, current1,  current2, current3, current4, current5;
extern int time_tk;
extern int time_notif;

struct Task_in
{
    int req; // 1 : activation, 0 : arrêt

    int texec; // texec , [mintexec, maxtexec]
    int mintexec;
    int maxtexec;

    int qos;      //qos , [minqos, maxqos]
    int minqos;
    int maxqos;

	int priority; //priorite de tache  
};

struct Hw_st 
{
	int av; // 1 (YES : Available) , 0 (NO)
};

struct Step_in
{
	Task_in contrast_img;
	Task_in motion_estim_imu;
	Task_in motion_estim_img;
	Task_in search_landing; 
	Task_in obstacle_avoidance;
	Task_in t_landing;
	Task_in rotoz_s;
	Task_in rotoz_b;
	Task_in replanning;
	Task_in detection; 
	Task_in tracking;

	Hw_st h1;
	Hw_st h2;
	Hw_st h3;
};

//*********** sortie automate
struct Task_out
{  
    int act; // 1 : active, 0: stop or attente ressource
    int version; // -1, (SW =>) 0,  (HW/tile =>) 1, 2, 3

    int code;

    int achievable; // 0 (NO), 1 :tache realisable (YES)

    int up_pos;   // 1 si l'automate peut choisir une version plus rapide sinon 0
    int down_pos; // 1 si l'automate peut choisir une version moins rapide sinon 0
    int keep_pos; // 1 si l'automate peut conserver la version courante sinon 0
    int qos_pos;  // 1 si possible, sinon 0
};

struct Step_out
{
	Task_out contrast_img;
	Task_out motion_estim_imu;
	Task_out motion_estim_img;
	Task_out search_landing; 
	Task_out obstacle_avoidance;
	Task_out t_landing;
	Task_out rotoz_s;
	Task_out rotoz_b;
	Task_out replanning;
	Task_out detection; 
	Task_out tracking;
};




std::vector<std::string> readfile1(const char* path);

void achievable_tab(Step_out s);

int verify(Step_out s);
void publish_to_MM(int a,Step_out s);
std::vector < std::vector<std::string> > scheduler_tab(int n, int m);
void v(std::string t[][7],int i);
void activate_desactivate_task(std::string t[][7],int i,std_msgs::Int32 msg);
void mapping(std::vector< std::vector<std::string> > M);
void comparer(std::vector<std::string> lignes, std::vector<std::string> C3, Step_in e);
void notify_Callback(const std_msgs::Int32::ConstPtr& msg);








#endif