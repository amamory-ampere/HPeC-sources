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
 * Author(s) :  Erwan Moréac, erwan.moreac@univ-ubs.fr (EM)
 * Created on: September 17, 2018
 * 
 * Main file of the Adaptation Manager ROS node.
 *   This application controls the execution of other HPeC nodes and execute a 
 *   reconfiguration automaton to get the right schedule.
 *   The configuration is given according Mission Manager requests 
 *************************************************************************************/

#include "adaptation_manager_node.h"

using namespace std; 
using namespace cv;

/****** GLOBAL VARIABLES ********/
vector<Map_app_out> prev_app_output_config;
vector<Map_app_out> app_output_config;
bool first_step = true;

boost::shared_ptr<ros::Publisher> search_land_pub = NULL;
boost::shared_ptr<ros::Publisher> contrast_img_pub = NULL;
boost::shared_ptr<ros::Publisher> motion_estim_imu_pub = NULL;
boost::shared_ptr<ros::Publisher> motion_estim_img_pub = NULL;
boost::shared_ptr<ros::Publisher> obstacle_avoidance_pub = NULL;
boost::shared_ptr<ros::Publisher> t_landing_pub = NULL;
boost::shared_ptr<ros::Publisher> rotoz_s_pub = NULL;
boost::shared_ptr<ros::Publisher> rotoz_b_pub = NULL;
boost::shared_ptr<ros::Publisher> replanning_pub = NULL;
boost::shared_ptr<ros::Publisher> detection_pub = NULL;
boost::shared_ptr<ros::Publisher> tracking_pub = NULL;

boost::shared_ptr<ros::Publisher> achievable_pub=NULL;
/****** END GLOBAL VARIABLES ********/


void achievable_tab(Step_out s){
     int i;
    vector< vector<string> > M(11); 
   for(i=0; i < 11; ++i) {
     M[i] = vector<string>(2);
   }
   M[0][0] = "contrast_img";
   M[1][0] = "motion_estim_imu";
   M[2][0] = "motion_estim_img";
   M[3][0] = "search_landing";
   M[4][0] = "obstacle_avoidance";
   M[5][0] = "t_landing";
   M[6][0] = "rotoz_s";
   M[7][0] = "rotoz_b";
   M[8][0] = "replanning";
   M[9][0] = "detection";
   M[10][0] = "tracking";

   M[0][1] = to_string(s.contrast_img.achievable);
   M[1][1] = to_string(s.motion_estim_imu.achievable);
   M[2][1] = to_string(s.motion_estim_img.achievable);
   M[3][1] = to_string(s.search_landing.achievable);
   M[4][1] = to_string(s.obstacle_avoidance.achievable);
   M[5][1] = to_string(s.t_landing.achievable);
   M[6][1] = to_string(s.rotoz_s.achievable);
   M[7][1] = to_string(s.rotoz_b.achievable);
   M[8][1] = to_string(s.replanning.achievable);
   M[9][1] = to_string(s.detection.achievable);
   M[10][1] = to_string(s.tracking.achievable);

    ofstream file(PATH_ACHIEVABLE_TAB, ios::out); 
   if ( file ) 
    {       
        for (int i = 0; i < 11; ++i){
            if(M[i][1]=="0"){
              file <<M[i][0]<<endl;
              file <<M[i][1]<<endl; 
            }
        }
         file.close();  
    }  else{
                cerr << "Impossible d'ouvrir le fichier !" << endl;
    }

}


//******************BOUDABZA Ghizlane; verification de la realisation de toutes les APPs..
bool verify(Step_out s){
    bool a = true; 
    a = a & (s.contrast_img.achievable) & (s.motion_estim_imu.achievable)
          & (s.motion_estim_img.achievable)& (s.search_landing.achievable)
          & (s.obstacle_avoidance.achievable)& (s.t_landing.achievable)
          & (s.rotoz_s.achievable)& (s.rotoz_b.achievable)& (s.replanning.achievable)
          & (s.detection.achievable)& (s.tracking.achievable);
    return a;
}

//********************* Publier l'alerte à Mission Manager
void publish_to_MM(bool a,Step_out s)
{   
    std_msgs::Int32 msg1;
    //remplissage de liste des taches nn realisables
       achievable_tab(s);
       if(a==1)
    {
        ROS_INFO("[LOADING][BITSTREAM][SCHEDULING] , Achievable : [%d]", a);
    }else{
        ROS_INFO("[ALERT][MISSION_MANAGER]");
        msg1.data =1;      
        achievable_pub->publish(msg1);   
        ROS_INFO("STATE OF Total_Achievable : [%d]", a);
        ROS_INFO("[LOADING][BITSTREAM][LAST AUTOMATE OUTPUT]");
    }
}


//******************fonction d'activation des taches en sw ou hw: en (-), ou en (s,f,-)
void activate_desactivate_task(int app_index, std_msgs::Int32 msg){

    switch (app_index)
    {
    case 0:
        contrast_img_pub->publish(msg);
        break;
    case 1:
        motion_estim_imu_pub->publish(msg);
        break;
    case 2:
        motion_estim_img_pub->publish(msg);
        break;
    case 3:
        search_land_pub->publish(msg);
        break;
    case 4:
        obstacle_avoidance_pub->publish(msg);
        break;
    case 5:
        t_landing_pub->publish(msg);
        break;
    case 6:
        rotoz_s_pub->publish(msg);
        break;
    case 7:
        rotoz_b_pub->publish(msg);
        break;
    case 8:
        replanning_pub->publish(msg);
        break;
    case 9:
        detection_pub->publish(msg);
        break;
    case 10:
        tracking_pub->publish(msg);
        break;
    default:
        break;
    }
}


//**************************BOUDABZA Ghizlane; 
//la fonction qui verifie est ce que texe appartient à l'intervalle [Tmin, Tmax]...pour chaque fonction;
bool compare(std::vector<App_timing_qos> time_qos, std::vector<Task_in> C3) 
{ 
    if(time_qos.size() != C3.size())
    {
        ROS_ERROR("COMPARE function failed! C3 and time_qos are not of the same size! C3=%ld and time_qos=%ld"
                    ,time_qos.size(), C3.size());
        exit(1);
    }

    vector<bool> T; 
    bool res=true;

    for(int j = 0; j < C3.size(); j++)
    {
        if((time_qos[j].texec >= C3[j].mintexec && time_qos[j].qos <= C3[j].mintexec) 
            && (time_qos[j].qos >= C3[j].minqos && time_qos[j].qos <= C3[j].maxqos))
            T.push_back(true);
        else
            T.push_back(false);
    }

    for(int i=0;i<T.size();i++){
        res= res & T[i];
        cout<<T[i]<<" ";
    }
    cout<<endl;

    return res;
}

//***************************
void notify_Callback(const std_msgs::Int32::ConstPtr& msg){
      ROS_INFO("[RECU][MISSION_M][CHANGEMENT D'INTERVALLES DANS TABLE C3..] \n");
      ROS_INFO("%d", msg->data);
      float texe; 

    vector<Task_in> C3 = read_C3(PATH_TABLE_C3);
      Step_in e;
    e.init();
    e.load_C3(C3); 
      do1(e);
}



/************************************************************
*	EM 24/07/2018, MAIN
*	Adaptation manager main code
************************************************************/
int main (int argc, char ** argv)
{   
    ros::init(argc, argv, "adaptation_manager_node");
    ros::NodeHandle nh;

    ROS_INFO("[ADAPTATION MANAGER] [RUNNING] \n");

     //contrast_img_pub;
    contrast_img_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/contrast_img_mgt_topic", 1));
    //motion_estim_imu_pub;
    motion_estim_imu_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/motion_estim_imu_mgt_topic", 1));
     //motion_estim_img_pub;
    motion_estim_img_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/motion_estim_img_mgt_topic", 1));
    //search_land_pub;
    search_land_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/search_landing_area_mgt_topic", 1));
     //obstacle_avoidance_pub;
    obstacle_avoidance_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/obstacle_avoidance_mgt_topic", 1));
     //t_landing_pub;
    t_landing_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/t_landing_mgt_topic", 1));
     //rotoz_s_pub;
    rotoz_s_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/rotoz_s_mgt_topic", 1));
     //rotoz_b_pub;
    rotoz_b_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/rotoz_b_mgt_topic", 1));
    //replanning_pub;
    replanning_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/replanning_mgt_topic", 1));   
     //detection_pub;
    detection_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/detection_mgt_topic", 1));
     //tracking_pub;
    tracking_pub= boost::make_shared<ros::Publisher>( 
                     nh.advertise<std_msgs::Int32>("/tracking_mgt_topic", 1));

    achievable_pub= boost::make_shared<ros::Publisher>( 
                        nh.advertise<std_msgs::Int32>("/achievable_topic", 1000));

    ros::Subscriber notify_from_MM_sub;
        notify_from_MM_sub = nh.subscribe("/notify_from_MM_topic", 1000, notify_Callback);
                    
    ros::Publisher cpu_pub;
        cpu_pub = nh.advertise<std_msgs::Float32>("/cpu_load_topic", 1);				    
    //*****************Ghizlane BOUDABZA
    std_msgs::Float32 load;

    //EM, Data structure setup
    Step_in     automaton_in;
    Step_out    automaton_out;
    vector<Task_in>         C3      = read_C3(PATH_TABLE_C3);
    vector<Bitstream_map>   bts_map = read_BTS_MAP(PATH_MAP_TAB);
    //EM, Shared Memory setup
    MemoryCoordinator shMemAccess("Admin");
    sh_mem_setup(shMemAccess, C3);

    //EM, First Step use with start configuration
    automaton_out = do1(automaton_in); //Call reconfiguration automaton

    //EM, For the 1st Step, init each attributes of prev_config to 0 
    Map_app_out empty_map;
    empty_map.init();
    for(size_t i = 0; i < APPLICATION_NUMBER; i++)
    {
        prev_app_output_config.push_back(empty_map);
    }        
    app_output_config = init_output(automaton_out); //convert automaton_out


    task_mapping(app_output_config, prev_app_output_config, bts_map, shMemAccess);
    
    /* MAIN LOOP
    vector<App_timing_qos> time_qos_data;
    ros::Rate loop_rate(10); //10hz = 100ms, 0.1hz=10s
    while(ros::ok())
    { 
        ros::spinOnce();

        load.data = cpuload ( ) ;
        cpu_pub.publish( load );

        time_qos_data = read_time_qos(PATH_TIME_QOS);
        if(time_qos_data.size()!=0)
        {
            e.update_timing_qos(time_qos_data);	
            if(!compare(time_qos_data,C3))
            {
                do1(e); 
                publish_to_MM(verify(automaton_out),automaton_out);  
            }
        }
        loop_rate.sleep();
    }
    */
}

vector<Task_in> read_C3(const char* path)
{
    vector<Task_in> res;
    vector<string> file_content = readfile(path);

    Task_in tmp;
    for(int i=0; i<file_content.size(); i+=9)
    {
        //EM, The first string every 9 rows is the Task name, ex: [0],[9]...
        tmp.req			= stoi(file_content[i+1]);
        tmp.texec		= stoi(file_content[i+2]);
        tmp.mintexec	= stoi(file_content[i+3]);
        tmp.maxtexec	= stoi(file_content[i+4]);
        tmp.qos			= stoi(file_content[i+5]);
        tmp.minqos 		= stoi(file_content[i+6]);
        tmp.maxqos 		= stoi(file_content[i+7]);
        tmp.priority	= stoi(file_content[i+8]);

        res.push_back(tmp);
    }
   
    cout << "There are "<< file_content.size() << " lines in the file" << endl;
    cout << "There are "<< res.size() << " tasks" << endl;
       for (size_t i = 0; i < res.size(); i++)
    {
        //cout << "Task " << i << " : " <<  endl;
        //res[i].print();
    }
    return res;
}




vector<App_timing_qos> read_time_qos(const char* path)
{
    vector<App_timing_qos> res;
    vector<string> file_content = readfile(path);

    App_timing_qos tmp;
    for(int i=0; i<file_content.size(); i+=3)
    {
        //EM, The first string every 3 rows is the Task name, ex: [0],[3]...
        tmp.texec		= stoi(file_content[i+1]);
        tmp.qos			= stoi(file_content[i+2]);
        res.push_back(tmp);
    }
   
    cout << "There are "<< file_content.size() << " lines in the file" << endl;
    cout << "There are "<< res.size() << " tasks" << endl;
    for (size_t i = 0; i < res.size(); i++)
    {
        //cout << "Task " << i << " : " <<  endl;
        //res[i].print();
    }
    return res;
}

vector<Map_app_out>	init_output(Step_out const& step_output)
{
    //EM, instantiation of the returned structure
    vector<Map_app_out> res;
    Map_app_out tmp;
    tmp.init();
    for(int i=0; i < APPLICATION_NUMBER; i++)
        res.push_back(tmp);

    if(first_step)
    {
        prev_app_output_config = res;
        first_step = false;
    }

    //EM, I know it's dirty, it would have been better with an array of attributes...
    res[0] 	= step_output.contrast_img;
    res[1] 	= step_output.motion_estim_imu;
    res[2] 	= step_output.motion_estim_img;
    res[3] 	= step_output.search_landing;
    res[4] 	= step_output.obstacle_avoidance;
    res[5] 	= step_output.t_landing;
    res[6] 	= step_output.rotoz_s;
    res[7] 	= step_output.rotoz_b;
    res[8] 	= step_output.replanning;
    res[9] 	= step_output.detection;
    res[10] = step_output.tracking;

    check_sequence(res);
    return res;
}

void check_sequence(vector<Map_app_out> & map_config_app)
{
    if(map_config_app.size() < APPLICATION_NUMBER)
    {
        std::cout << "The map_config_app table provided is too small! map_config_app size=" << map_config_app.size() << std::endl;
        return; //EM, to leave a void function
    }

    //EM, check if 2 active apps have the same HW Tile location 
    //	  => meaning sequence execution
    for(int i=0; i < (APPLICATION_NUMBER - 1); i++)
        if(map_config_app[i].active != 0 
            && map_config_app[i].version_code < MULTI_APP_THRESHOLD_CODE
            && map_config_app[i].region_id != 0)
            for(int j=i+1; j < APPLICATION_NUMBER; j++)
                if(map_config_app[i].region_id == map_config_app[j].region_id
                    && map_config_app[j].active != 0)
                {
                    map_config_app[i].fusion_sequence = "s";
                    map_config_app[j].fusion_sequence = "s";
                }
}

vector<Bitstream_map> read_BTS_MAP(const char* path)
{
    vector<Bitstream_map> res;
    vector<string> file_content = readfile(path);

    Bitstream_map tmp;
    for(int i=0; i<file_content.size(); i+=2)
    {
        tmp.version_code		= stoi(file_content[i]);
        tmp.bitstream_addr		= stoi(file_content[i+1]);
        res.push_back(tmp);
    }
   
       cout << "There are "<< file_content.size() << " lines in the file" << endl;
    cout << "There are "<< res.size() << " Bitstreams" << endl;
       for (size_t i = 0; i < res.size(); i++)
    {
        //cout << "App MAP " << i << " : " <<  endl;
        res[i].print();
    }
    return res;
}

int find_BTS_addr(vector<Bitstream_map> bts_map, int version_code)
{
    for (int i = 0; i < bts_map.size(); i++)
        if(bts_map[i].version_code == version_code)
            return bts_map[i].bitstream_addr;

    return -1;
}

void task_mapping(vector<Map_app_out> const& map_config_app, 
            vector<Map_app_out> const& prev_map_config_app, 
            vector<Bitstream_map> const& bitstream_map, 
            MemoryCoordinator & shared_memory)
{
    vector<App_scheduler> scheduler_array = create_scheduler_tab(map_config_app, prev_map_config_app,bitstream_map);
    std_msgs::Int32 msg;

    //EM, First loop: disable each Task, not only those that have to be stopped,
    //in order to free HW Tiles. This is not a problem since all tasks in 
    //scheduler change of state, involving a Stop at a time.
    cout << endl;
    for(size_t i = 0; i < scheduler_array.size(); i++)
    {
        msg.data = 0; 
        activate_desactivate_task(scheduler_array[i].app_index, msg);	
        cout << "\033[1;31m Disable Task no: " << scheduler_array[i].app_index << "\033[0m"  << endl;
    }

    //EM, Second loop: Ensure that all Tiles that are gonna be configured are freed.
    for(size_t i = 0; i < scheduler_array.size(); i++)
    {
        if(scheduler_array[i].region_id != 0 && scheduler_array[i].active ==1) 
        {
            wait_release(scheduler_array[i].app_index, scheduler_array[i].region_id, prev_map_config_app, shared_memory);
        }
    }

    //EM, Third loop: activation of each Task except those in sequence "s"
    for(size_t i = 0; i < scheduler_array.size(); i++)
    {
        //Enable SW tasks
        if(scheduler_array[i].region_id == 0 && scheduler_array[i].active == 1) 
        {
            msg.data = 1; 
            activate_desactivate_task(scheduler_array[i].app_index, msg);
            cout << "\033[1;32m Enable SW version of Task no: " << scheduler_array[i].app_index << "\033[0m" << endl;
        }

        //Enable classic HW tasks
        if(scheduler_array[i].region_id != 0 && scheduler_array[i].active == 1
            && scheduler_array[i].fusion_sequence != "f" && scheduler_array[i].fusion_sequence != "s") 
        {
            //EM, TODO: LOAD bitstream in FPGA!!! 
            msg.data = 2; 
            activate_desactivate_task(scheduler_array[i].app_index, msg);
            cout << "\033[1;36m Enable HW version of Task no: " << scheduler_array[i].app_index 
                    << " in Tile no: " << scheduler_array[i].region_id << "\033[0m" << endl;
        }

        //Enable HW tasks in fusion "f"
        if(scheduler_array[i].region_id != 0 && scheduler_array[i].active == 1
            && scheduler_array[i].fusion_sequence == "f")
        {
            //EM, TODO: LOAD bitstream in FPGA!!! 
            msg.data = scheduler_array[i].version_code - scheduler_array[i].region_id; 
            //The 2 fusionned tasks will be activated, only one ROS node can understand the msg.data and launch the fusion
            activate_desactivate_task(scheduler_array[i].app_index, msg);
            cout << "\033[36m Enable HW FUSION version of Task no: " << scheduler_array[i].app_index 
                    << " in Tile no: " << scheduler_array[i].region_id << "\033[0m" << endl;
        }
    }
    cout << endl;
    //EM, TODO: work on sequence HW tasks "s"
}


vector<App_scheduler>	create_scheduler_tab(vector<Map_app_out> const& map_config_app
                                            , vector<Map_app_out> const& prev_map_config_app
                                            , vector<Bitstream_map> const& bitstream_map)
{
    vector<App_scheduler> res;
    App_scheduler tmp;
    //EM, we add in the scheduler only applications with a different configuration from previous doStep()
    for(int i = 0; i < map_config_app.size(); i++)
        if( map_config_app[i].active != prev_map_config_app[i].active //EM, different state OR different version_code
          || map_config_app[i].version_code != prev_map_config_app[i].version_code)
        {
            tmp.app_index = i;
            tmp = map_config_app[i];
            if(map_config_app[i].region_id != 0) //EM, if HW version -> need bitstream addr
                tmp.bitstream_addr = find_BTS_addr(bitstream_map, map_config_app[i].version_code);
            else
                tmp.bitstream_addr = 0;
            tmp.loaded	= 0;
            tmp.done 	= 0;

            res.push_back(tmp);
        }

    cout << "Scheduler array size = " << res.size() << " : " <<  endl;
    for(int i = 0; i < res.size(); i++)
    {
        cout << "Scheduler array [" << i << "] : " <<  endl;
        res[i].print();
    }
    return res;
}


void	wait_release(int app, int region_id, vector<Map_app_out> const& prev_map_config_app
                    , MemoryCoordinator & shared_memory)
{
    for(int app_index = 0; app_index < prev_map_config_app.size(); app_index++)
        if(prev_map_config_app[app_index].region_id == region_id 
            && prev_map_config_app[app_index].active == 1)
            {
                cout << "\033[0;33m Wait END for HW Task no: " << app << "\033[0m"  << endl;
                cout << "SIGNAL WAIT App : " << app_index << endl;
                while(!shared_memory.release_hw_Read(app_index));
                cout << "\033[1;33m Tile no: " << region_id  << " released !!! \033[0m"  << endl;
            }
}


void compare_data_access_speed(MemoryCoordinator & shared_memory)
{
    clock_t start, ends;
    start = clock();
        read_value_file(PATH_RELEASE_HW,3);
    ends = clock();
    double res = double(ends - start) * 1000 / CLOCKS_PER_SEC;
    std::cout 	<< "READ DATA IN A FILE : " 
                << res << std::endl;

    start = clock();
        int foo = shared_memory.release_hw_Read(2);
    ends = clock();
    res =  double(ends - start) * 1000 / CLOCKS_PER_SEC;
    std::cout 	<< "READ DATA IN A SHARED MEMORY : " 
                << res << std::endl;

    start = clock();
        shared_memory.release_hw_Write(0,2);
    ends = clock();
    res =  double(ends - start) * 1000 / CLOCKS_PER_SEC;
    std::cout 	<< "WRITE DATA IN A SHARED MEMORY : " 
                << res << std::endl;
}

void sh_mem_setup(MemoryCoordinator & shared_memory, vector<Task_in> C3)
{
    std::vector<int> achievable_init;
    std::vector<int> release_hw_init;
    std::vector<int> done_init;
    for(int i = 0; i < APPLICATION_NUMBER; i++)
    {
        achievable_init.push_back(1);   //EM, everything achievable by default
        release_hw_init.push_back(1);   //  everything released by default
        done_init.push_back(0);         //  nothing done by default
    }
    std::vector<int> Sh_C3_init;
    for(int i = 0; i < C3.size(); i++)
    {
        Sh_C3_init.push_back(C3[i].req);
        Sh_C3_init.push_back(C3[i].texec);
        Sh_C3_init.push_back(C3[i].mintexec);
        Sh_C3_init.push_back(C3[i].maxtexec);
        Sh_C3_init.push_back(C3[i].qos);
        Sh_C3_init.push_back(C3[i].minqos);
        Sh_C3_init.push_back(C3[i].maxqos);
        Sh_C3_init.push_back(C3[i].priority);
    }
    shared_memory.Fill_ShMem_C3_table(Sh_C3_init);
    shared_memory.Fill_ShMem_achievable(achievable_init);
    shared_memory.Fill_ShMem_release_hw(release_hw_init);
    shared_memory.Fill_ShMem_done(done_init);
    std::cout << "Fill shared memories, done! " << std::endl;
}

vector<Task_in>	sh_mem_read_C3(MemoryCoordinator & shared_memory)
{
    vector<Task_in> res;
    Task_in tmp;
    vector<int> shared_data;
    for(int i=0; i<APPLICATION_NUMBER; i++)
    {
        shared_data = shared_memory.C3_table_Read(i);
        tmp.req			= shared_data[0];
        tmp.texec		= shared_data[1];
        tmp.mintexec	= shared_data[2];
        tmp.maxtexec	= shared_data[3];
        tmp.qos			= shared_data[4];
        tmp.minqos 		= shared_data[5];
        tmp.maxqos 		= shared_data[6];
        tmp.priority	= shared_data[7];

        res.push_back(tmp);
    }
   
    /*cout << "There are "<< res.size() << " tasks" << endl;
       for (size_t i = 0; i < res.size(); i++)
    {
        cout << "Task " << i << " : " <<  endl;
        res[i].print();
    }*/
    return res;
}



vector<App_timing_qos> 	sh_mem_read_time_qos(MemoryCoordinator & shared_memory)
{
    vector<App_timing_qos> res;
    App_timing_qos tmp;
    for(int i=0; i<APPLICATION_NUMBER; i++)
    {
        tmp.texec		= shared_memory.Read_ExecTime(i);
        tmp.qos			= shared_memory.Read_QoS(i);
        res.push_back(tmp);
    }
   
    /*cout << "There are "<< res.size() << " tasks" << endl;
       for (size_t i = 0; i < res.size(); i++)
    {
        cout << "Task " << i << " : " <<  endl;
        res[i].print();
    }*/
    return res;
}

    /*############################## TEST CODE ##############################*/
    /*std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();



    s = fake_output2();
    if(!first_step)
        prev_app_output_config = app_output_config;
    app_output_config = init_output(s); //conversion sortie step -> table 

    task_mapping(app_output_config, prev_app_output_config, bts_map, shMemAccess);

     
    

    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
    */
    /*############################## TEST CODE ##############################*/
