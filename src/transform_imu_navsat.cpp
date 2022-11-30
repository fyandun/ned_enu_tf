#include "ros/ros.h"
#include "sensor_msgs/Imu.h"
#include <tf2/LinearMath/Transform.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf/LinearMath/Quaternion.h>
#include <tf/transform_datatypes.h>

bool debug = true;
sensor_msgs::Imu new_imu_msg;
ros::Publisher new_imu_publisher; 

void imu_Callback(const sensor_msgs::Imu::ConstPtr& imu_msg)
{
    double raw_imuRoll, raw_imuPitch, raw_imuYaw;
    double conv_imuRoll, conv_imuPitch, conv_imuYaw;
    // std::cout << "HERE" << std::endl;
    geometry_msgs::Quaternion quat_msg_in, quat_msg_out;    
    quat_msg_in = imu_msg->orientation;

    tf2::Quaternion tf2_quat(quat_msg_in.x,quat_msg_in.y,quat_msg_in.z,quat_msg_in.w); 
    tf2::Matrix3x3(tf2_quat).getRPY(raw_imuRoll, raw_imuPitch, raw_imuYaw);
    tf2::Quaternion q_modified;
    
    q_modified.setRPY (raw_imuRoll, raw_imuPitch, raw_imuYaw);
    quat_msg_out = tf2::toMsg(q_modified);

    // Create a rotation from NED -> ENU
    /*tf2::Quaternion q_rotate;
    q_rotate.setRPY (M_PI, 0.0, M_PI/2);
    // Apply the NED to ENU rotation such that the coordinate frame matches
    tf2_quat = q_rotate*tf2_quat;
    quat_msg_out = tf2::toMsg(tf2_quat);*/


    new_imu_msg.header = imu_msg->header;
    new_imu_msg.angular_velocity = imu_msg->angular_velocity;
    new_imu_msg.linear_acceleration.x = imu_msg->linear_acceleration.y;
    new_imu_msg.linear_acceleration.y = imu_msg->linear_acceleration.x;
    new_imu_msg.linear_acceleration.z = imu_msg->linear_acceleration.z;
    new_imu_msg.orientation = quat_msg_out;
    
    if (sqrt(quat_msg_out.x*quat_msg_out.x + quat_msg_out.y*quat_msg_out.y + quat_msg_out.z*quat_msg_out.z + quat_msg_out.w*quat_msg_out.w) < 0.1)
    {
        ROS_ERROR("Invalid quaternion, please use a 9-axis IMU!");
        ros::shutdown();
    }    

    // new_imu_msg.header = imu_msg->header;
    // new_imu_msg.angular_velocity.x = imu_msg->angular_velocity.y;
    // new_imu_msg.angular_velocity.y = imu_msg->angular_velocity.x;
    // new_imu_msg.angular_velocity.z = -imu_msg->angular_velocity.z;

    // new_imu_msg.linear_acceleration.x = imu_msg->linear_acceleration.y;
    // new_imu_msg.linear_acceleration.y = imu_msg->linear_acceleration.x;
    // new_imu_msg.linear_acceleration.z = -imu_msg->linear_acceleration.z;

    // // put into ENU - swap X/Y, invert Z
    // new_imu_msg.orientation.x = imu_msg->orientation.y;
    // new_imu_msg.orientation.y = imu_msg->orientation.x;
    // new_imu_msg.orientation.z = -imu_msg->orientation.z;
    // new_imu_msg.orientation.w = imu_msg->orientation.w;

    if (debug){
    tf::Quaternion raw_orientation, conv_orientation;
    tf::quaternionMsgToTF(imu_msg->orientation, raw_orientation);
    tf::Matrix3x3(raw_orientation).getRPY(raw_imuRoll, raw_imuPitch, raw_imuYaw);
    std::cout << "raw_roll: " << raw_imuRoll << ", raw_pitch: " << raw_imuPitch << ", raw_yaw: " << raw_imuYaw << std::endl << std::endl;

    tf::quaternionMsgToTF(new_imu_msg.orientation, conv_orientation);
    tf::Matrix3x3(conv_orientation).getRPY(conv_imuRoll, conv_imuPitch, conv_imuYaw);
    std::cout << "conv_roll: " << conv_imuRoll << ", conv_pitch: " << conv_imuPitch << ", conv_yaw: " << conv_imuYaw << std::endl << std::endl;
    }

    new_imu_publisher.publish(new_imu_msg);

}

int main(int argc, char **argv)
{

  ros::init(argc, argv, "imu_navsat_conv");


  ros::NodeHandle n;

  new_imu_publisher = n.advertise<sensor_msgs::Imu>("/vectornav/imu_navsat", 1000);
  ros::Subscriber sub = n.subscribe("/vectornav/IMU", 1000, imu_Callback);

  /**
   * ros::spin() will enter a loop, pumping callbacks.  With this version, all
   * callbacks will be called from within this thread (the main one).  ros::spin()
   * will exit when Ctrl-C is pressed, or the node is shutdown by the master.
   */
  ros::spin();

  return 0;
}
