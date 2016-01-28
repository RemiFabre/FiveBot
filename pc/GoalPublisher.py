#!/usr/bin/env python
# encoding: utf-8

import roslib
import rospy
from rospy.rostime import Time


# Messages
from nav_msgs.msg import Odometry

import tf
import math

command_pub = None


def publishCommand(x, y, a):

	command_msg = Odometry()

	command_msg.pose.pose.position.x = x
	command_msg.pose.pose.position.y = y
	command_msg.pose.pose.orientation.z = a

	command_pub.publish(command_msg)    
	rospy.loginfo("Command published")	


def askForGoal():
	x = float(input("x ?"))
	y = float(input("y ?"))
	a = float(input("a ?"))


	publishCommand(x, y, a)

def CommandPublisher():
    """Main loop"""
    global command_pub
 
    rospy.init_node('goal_publisher', anonymous=True)        
    command_pub = rospy.Publisher('cmd_pos', Odometry, queue_size=10)
    
#    while(True):
#    rospy.spin()

    while not rospy.is_shutdown():
	askForGoal()

    
if __name__ == '__main__':
    try:
	CommandPublisher()
    except rospy.ROSInterruptException:
        pass
