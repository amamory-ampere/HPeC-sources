//TODO Insert header comment

#ifndef ADAPTATION_MANAGER_NODE_H
#define ADAPTATION_MANAGER_NODE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ctime>

#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/image_encodings.h>
#include "std_msgs/Int32.h"
#include "std_msgs/Float32.h"
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/thread.hpp>

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

#include "am_defines.h"
#include "adaptation_manager_struct.h"

#include "utils.h"
#include "handle_applications.h"
#include "reconfiguration_automate.h"
#include "reconfig.h"

#include "CommSharedMemory.hpp" //EM, shared_memory_lib header

using namespace std; 
using namespace cv;

//### EM, Potential functions to modify
void achievable_tab(Step_out s); //EM, TODO: CHANGE IT OR DELETE IT
bool verify(Step_out s);
void publish_to_MM(bool a,Step_out s);
void notify_Callback(const std_msgs::Int32::ConstPtr& msg);
//###

bool 	compare(std::vector<App_timing_qos> time_qos
				, std::vector<Task_in> C3, Step_in e);
int		find_BTS_addr(vector<Bitstream_map> bts_map, int version_code);
vector<Bitstream_map> 	read_BTS_MAP(const char* path);
vector<Task_in> 		read_C3(const char* path);
vector<App_timing_qos> 	read_time_qos(const char* path);
vector<Map_app_out>  	init_output(Step_out const& step_output);
vector<App_scheduler>	create_scheduler_tab(vector<Map_app_out> const& map_config_app
											, vector<Map_app_out> const& prev_map_config_app
											, vector<Bitstream_map> const& bitstream_map);
void 	activate_desactivate_task(int app_index, std_msgs::Int32 msg);
void	check_sequence(vector<Map_app_out> & map_config_app);

void	wait_release(int app, int region_id, vector<Map_app_out> const& prev_map_config_app
					, MemoryCoordinator	& shared_memory);
void 	task_mapping(vector<Map_app_out> const& map_config_app
				, vector<Map_app_out> const& prev_map_config_app
				, vector<Bitstream_map> const& bitstream_map
				, MemoryCoordinator & shared_memory);
void 	compare_data_access_speed(MemoryCoordinator & shared_memory);
void	sh_mem_setup(MemoryCoordinator & shared_memory, vector<Task_in> C3);
vector<Task_in>			sh_mem_read_C3(MemoryCoordinator & shared_memory);
vector<App_timing_qos> 	sh_mem_read_time_qos(MemoryCoordinator & shared_memory);

/*********** Global variables ***********/ 
extern vector<Map_app_out> prev_app_output_config;
extern vector<Map_app_out> app_output_config;
extern bool first_step;
#endif


