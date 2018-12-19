#!/usr/bin/env python

# Import mavutil
from pymavlink import mavutil
import time

# Create the connection
master = mavutil.mavlink_connection('udp:127.0.0.1:14551')
# Wait a heartbeat before sending commands
master.wait_heartbeat()

# Set parameter value
# Set a parameter value TEMPORARILY to RAM. It will be reset to default on system reboot.
# Send the ACTION MAV_ACTION_STORAGE_WRITE to PERMANENTLY write the RAM contents to EEPROM.
# The parameter variable type is described by MAV_PARAM_TYPE in http://mavlink.org/messages/common.
master.mav.param_set_send(
    master.target_system, master.target_component,
    b'SIM_ACC_RND',
    0,
    mavutil.mavlink.MAV_PARAM_TYPE_REAL32
)
# Read ACK
# IMPORTANT: The receiving component should acknowledge the new parameter value by sending a param_value message to all communication partners.
# This will also ensure that multiple GCS all have an up-to-date list of all parameters.
# If the sending GCS did not receive a PARAM_VALUE message within its timeout time, it should re-send the PARAM_SET message.
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))

# Set parameter value
master.mav.param_set_send(
    master.target_system, master.target_component,
    b'SIM_RC_FAIL',
    0,
    mavutil.mavlink.MAV_PARAM_TYPE_REAL32
)
# Read ACK
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))

# Set parameter value
master.mav.param_set_send(
    master.target_system, master.target_component,
    b'SIM_GPS_DISABLE',
    0,
    mavutil.mavlink.MAV_PARAM_TYPE_REAL32
)
# Read ACK
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))

time.sleep(1)

# Request parameter
master.mav.param_request_read_send(
    master.target_system, master.target_component,
    b'SIM_RC_FAIL',
    -1
)
# Print old parameter value
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))


# Request parameter
master.mav.param_request_read_send(
    master.target_system, master.target_component,
    b'SIM_GPS_DISABLE',
    -1
)
# Print old parameter value
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))


# Request parameter
master.mav.param_request_read_send(
    master.target_system, master.target_component,
    b'SIM_ACC_RND',
    -1
)
# Print old parameter value
message = master.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
print('name: %s\tvalue: %d' % (message['param_id'].decode("utf-8"), message['param_value']))