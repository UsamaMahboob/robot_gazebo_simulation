#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <chrono>
#include <math.h>


class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller_node")
    {
        my_pose_.set__x(-10.0);
        pose_subscriber_ = this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10, 
                            std::bind(&TurtleControllerNode::pose_subscriber_callback,this,std::placeholders::_1));

        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel",10);

        timer_ = this->create_wall_timer(std::chrono::milliseconds(10),
                            std::bind(&TurtleControllerNode::control_loop_timer,this));

        RCLCPP_INFO(this->get_logger(), "Turtle Controller node has been started.");
    }
private:
    void pose_subscriber_callback(turtlesim::msg::Pose msg)
    {
        my_pose_ = msg;
    }

    void control_loop_timer()
    {
        if(my_pose_.x == -10.0)
            return;
        
        auto target = turtlesim::msg::Pose();
        target.set__x(4.0);
        target.set__y(8.0);

        float x_distance = target.x - my_pose_.x;
        float y_distance = target.y - my_pose_.y;

        RCLCPP_INFO(this->get_logger(), "y_distance = %f, x_distance = %f \n", y_distance, x_distance);
        float total_distance = sqrt(pow(x_distance,2) + pow(y_distance,2));
        auto cmd = geometry_msgs::msg::Twist();

        float goal_theta = atan2(y_distance,x_distance);
        
        float theta_difference = goal_theta - my_pose_.theta;

        if(total_distance > 0.5)                                  // Turtle is far from target
        {                                  
            cmd.linear.set__x(4*total_distance);

            //angular velocity
            cmd.angular.set__z(8*theta_difference);
        }
        else                                                      //Turtle is close enough to the target so we stop moving
        {
            cmd.linear.set__x(0.0);
            cmd.angular.set__z(0.0);
        }
        cmd_vel_publisher_->publish(cmd);

    }

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    turtlesim::msg::Pose my_pose_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    //auto exec = rclcpp::executors::SingleThreadedExecutor();
    auto node = std::make_shared<TurtleControllerNode>();
    //exec.add_node(std::make_shared<TurtleControllerNode>());
    //exec.spin();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
