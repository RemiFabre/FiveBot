#!/usr/bin/env python
# encoding: utf-8

import roslib
import rospy
from rospy.rostime import Time

from geometry_msgs.msg import Twist, PoseStamped,Point
from sensor_msgs.msg import Joy

import tf
import math

command_pub = None


prev_vx = 0
prev_vy = 0
prev_w = 0

def publishCommand(vx, vy, w, pid):

	command_msg = Twist()

	command_msg.linear.x = vx
	command_msg.linear.y = vy
	command_msg.angular.z = w
	command_msg.linear.z = pid

	command_pub.publish(command_msg)    
	rospy.loginfo("Command published")	


def joystickDataReceived(data):
	global prev_vx, prev_vy, prev_w
	vx = data.axes[1]
	vy = data.axes[0]
	w = data.axes[2]
	pid = data.buttons[14]
	if (abs(prev_vx-vx)+abs(prev_vy-vy)+abs(prev_w-w)+pid > 0.1):
		publishCommand(vx, vy, w, pid)
		prev_vx = vx
		prev_vy = vy
		prev_w = w

def CommandPublisher():
    """Main loop"""
    global command_pub
    global current_time,previous_time
 
    rospy.init_node('command_publisher', anonymous=True)
    rospy.loginfo("Node created")	

    rospy.Subscriber('joy', Joy, joystickDataReceived)
    command_pub = rospy.Publisher('cmd_vel', Twist, queue_size=1)
    
    rospy.spin()

#    while not rospy.is_shutdown():
#	current_time=rospy.Time.now()
#	previous_time = current_time

    
if __name__ == '__main__':
    try:
	CommandPublisher()
    except rospy.ROSInterruptException:
        pass
