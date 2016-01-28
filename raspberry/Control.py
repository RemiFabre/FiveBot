#!/usr/bin/env python

import serial
import struct
import threading

import rospy
import roslib

import math
import tf

from rospy.rostime import Time

# Messages
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Twist, PoseStamped, Point, Quaternion
from std_msgs.msg import String

import boardcom


class Control(boardcom.BoardCom):


    def __init__(self, tty):
	boardcom.BoardCom.__init__(self,tty)  #Python 2
        self.prev_vx = 0
        self.prev_vy = 0
        self.prev_w  = 0
	self.pid = False

	self.max_speed = 10.
	self.min_speed = -10.
	
	# Current goal
	self.goal_set = False
	self.goal_x = 0.
	self.goal_y = 0.
	self.goal_a = 0.

	# Current position
	self.current_x = 0.
	self.current_y = 0.
	self.current_a = 0.

	# PID parameters
	self.p = 0.1

    def init_node(self):
        self.pub = rospy.Publisher('odom', Odometry, queue_size=10)
        rospy.init_node('Command', anonymous=True)
        rospy.Subscriber('cmd_vel', Twist, self.on_speed)
	rospy.Subscriber('cmd_pos', Odometry, self.on_position)		
 
    def on_odometry(self, x, y, w):
        rospy.loginfo("Encoder data received")
        current_time = rospy.Time.now()

	self.current_x = x
	self.current_y = y
	self.current_a = a
	
        # To compute speed
        #time_delta = (current_time - previous_time).to_sec()
        #(prev_x,prev_y,prev_w,previous_time) = (x,y,w,current_time)

        vx = 0 # (x - prev_x)/time_delta 
        vy = 0 # (y - prev_y)/time_delta
        vw = 0 # (w - prev_w)/time_delta

       # TODO: Understand
        quat = tf.transformations.quaternion_from_euler(0,0,w)

        odom_msg = Odometry()
        odom_msg.header.stamp = rospy.Time.now()
        odom_msg.header.frame_id="odom_combined"
        odom_msg.pose.pose.position.x = x
        odom_msg.pose.pose.position.y = y
        odom_msg.pose.pose.position.z = 0.0

        # TODO: Understand
        odom_msg.pose.pose.orientation.x = quat[0]
        odom_msg.pose.pose.orientation.y = quat[1]
        odom_msg.pose.pose.orientation.z = quat[2]
        odom_msg.pose.pose.orientation.w = quat[3]

        # TODO: Understand
        odom_msg.pose.covariance = [1e-5, 0, 0, 0, 0, 0,
                                0, 1e-5, 0, 0, 0, 0,
                                0, 0, 1e-5, 0, 0, 0,
                                0, 0, 0, 1e-5, 0, 0,
                                0, 0, 0, 0, 1e-5, 0,
                                0, 0, 0, 0, 0, 1e-3]
        odom_msg.twist.twist.linear.x = vx
        odom_msg.twist.twist.linear.y = vy
        odom_msg.twist.twist.angular.z = vw
        odom_msg.twist.covariance = odom_msg.pose.covariance

        self.pub.publish(odom_msg)
        rospy.loginfo("Odometry published")


	if goal_set:
		# PID : TODO: Move code into a new function
		err_x = self.goal_x - self.current_x	
		err_y = self.goal_y - self.current_y
		err_a = self.goal_a - self.current_a
		if (err_x * err_x + err_y * err_y + err_a * err_a < self.epsilon):
			rospy.loginfo("Goal reached !")
			self.goal_set = False
			self.goal_x = 0.
			self.goal_y = 0.
			self.goal_a = 0.
			self.send_speed(0., 0., 0., self.pid)
		else: 	
			cmd_vx = max(min(self.p * err_x, self.max_speed), self.min_speed)
			cmd_vy = max(min(self.p * err_y, self.max_speed), self.min_speed)
			cmd_w  = max(min(self.p * err_a, self.max_speed), self.min_speed)

			self.send_speed(cmd_vx, cmd_vy, cmd_w, self.pid)


    def on_info(self, msg):
        print(msg)
    
    def on_error(self, msg):
        print(msg)
    
    def on_position(self, data):
        rospy.loginfo("Command (position) data received from PC")
	self.goal_set = True
	self.goal_x = data.pose.pose.position.x
	self.goal_y = data.pose.pose.position.y
	self.goal_a = data.pose.pose.orientation.z

    def on_speed(self, data):
        rospy.loginfo("Command (speed) data received from PC")
        current_time = rospy.Time.now()

        vx = data.linear.x
        vy = data.linear.y
        w  = data.angular.z
	
        if (data.linear.z == 1.0):
		self.pid = not self.pid
        if (not self.pid):
                rospy.loginfo("PID disabled : " + str(vx) + " " + str(vy) + " " + str(w))
        else:
                rospy.loginfo("PID enabled : " + str(vx) + " " + str(vy) + " " + str(w))
        self.send_speed(10*vx, 10*vy, 10*w, not self.pid)
        self.prev_vx = vx
        self.prev_vy = vy
        self.prev_w = w
        rospy.loginfo("Command sent to Arduino")



if __name__ == '__main__':
        comm = Control("/dev/ttyUSB0") #TODO: argv
        
	comm.init_node()
        comm.run()


