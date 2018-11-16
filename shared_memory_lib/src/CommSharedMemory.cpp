#include "CommSharedMemory.hpp"


int  MemoryCoordinator::achievable_Read(const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(achievable_mutex);
    return achievable_ptr[app_index];
}
void MemoryCoordinator::achievable_Write(const int &data, const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(achievable_mutex);
    achievable_ptr[app_index] = data;
}

int  MemoryCoordinator::release_hw_Read(const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(release_hw_mutex);
    return release_hw_ptr[app_index];
}
void MemoryCoordinator::release_hw_Write(const int &data, const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(release_hw_mutex);
    release_hw_ptr[app_index] = data;
}

int  MemoryCoordinator::done_Read(const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(done_mutex);
    return done_ptr[app_index];
}
void MemoryCoordinator::done_Write(const int &data, const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(done_mutex);
    done_ptr[app_index] = data;
}


void MemoryCoordinator::Update_ExecTime(const int &data, const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(C3_table_mutex);
    C3_table_ptr[app_index+C3_CURRENT_TEXEC] = data;
}
void MemoryCoordinator::Update_QoS(const int &data, const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(C3_table_mutex);
    C3_table_ptr[app_index+C3_CURRENT_QOS] = data;
}
int MemoryCoordinator::Read_ExecTime(const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(C3_table_mutex);
    return C3_table_ptr[app_index+C3_CURRENT_TEXEC];
}
int MemoryCoordinator::Read_QoS(const int &app_index)
{
    bip::scoped_lock<bip::named_mutex> lock(C3_table_mutex);
    return C3_table_ptr[app_index+C3_CURRENT_QOS];
}