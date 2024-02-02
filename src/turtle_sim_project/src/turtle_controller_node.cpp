#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"

class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller_node")
    {
        subscriber_ = this->create_subscription<turtlesim::msg::Pose>("turtle1/pose", 10, 
                            std::bind(&pose_subscriber_callback,this,std::placeholders::_1));

        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("turtle1/cmd_vel",10);

        RCLCPP_INFO(this->get_logger(), "Turtle Controller node has been started.");
    }
    void pose_subscriber_callback(turtlesim::msg::Pose msg)
    {
        my_pose_ = msg;
    }

private:
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    turtlesim::msg::Pose my_pose_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    auto exec = rclcpp::executors::SingleThreadedExecutor();
    exec.add_node(std::make_shared<TurtleControllerNode>());
    exec.spin();

    rclcpp::shutdown();
    return 0;
}
