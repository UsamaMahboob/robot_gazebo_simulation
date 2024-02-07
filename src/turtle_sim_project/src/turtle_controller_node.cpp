#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "custom_interfaces/msg/turtle_array.hpp"
#include "custom_interfaces/msg/turtle.hpp"
#include "custom_interfaces/srv/turtle_kill.hpp"
#include "std_srvs/srv/empty.hpp"
#include <chrono>
#include <limits.h>
#include <math.h>

class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller_node")
    {
        my_pose_.set__x(-10.0); // This means these variables do not have any useful values as -10 is out of bounds so they will serve as a check to check initialization
        turtle_found = false;
        kill_closest_turtle = true; // It will kill the turtle closest to the main turtle first

        pose_subscriber_ = this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10,
                                                                           std::bind(&TurtleControllerNode::pose_subscriber_callback, this, std::placeholders::_1));

        alive_tutles_subscriber_ = this->create_subscription<custom_interfaces::msg::TurtleArray>("/turtle_spawn_locations", 10,
                                                                                                  std::bind(&TurtleControllerNode::alive_turtles_subscriber_callback, this, std::placeholders::_1));

        remove_turtle_from_list_client_ = this->create_client<custom_interfaces::srv::TurtleKill>("/remove_turtle");
        while (remove_turtle_from_list_client_->wait_for_service(std::chrono::seconds(1)))
        {
            RCLCPP_WARN(this->get_logger(), "waiting for /remove_turtle server to be up..");
        }

        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

        timer_ = this->create_wall_timer(std::chrono::milliseconds(10),
                                         std::bind(&TurtleControllerNode::control_loop_timer, this));

        RCLCPP_INFO(this->get_logger(), "Turtle Controller node has been started.");
    }

private:
    void alive_turtles_subscriber_callback(custom_interfaces::msg::TurtleArray msg)
    {
        if (msg.turtles.empty() || turtle_found == true) // No turtle to catch
        {
            // turtle_found = false;
            return;
        }
        if (kill_closest_turtle)
        {
            std::pair<int, long unsigned int> shortest_distance(INT_MAX, -1);

            for (long unsigned int i = 0; i < msg.turtles.size(); i++) // Find turtle closest to us
            {
                float total_distance = sqrt(pow((my_pose_.x - msg.turtles[i].x), 2) + pow((my_pose_.y - msg.turtles[i].y), 2));
                if (total_distance < shortest_distance.first)
                {
                    shortest_distance.first = total_distance;
                    shortest_distance.second = i;
                }
            }
            alive_turtle_ = msg.turtles[shortest_distance.second];
        }
        else
        {
            for (long unsigned int i = 0; i < msg.turtles.size(); i++)
            {
                RCLCPP_INFO(this->get_logger(), "Choosing turtle %s ", msg.turtles[i].name.c_str());
            }
            RCLCPP_INFO(this->get_logger(), "Done choosing");
            alive_turtle_ = msg.turtles[0];
        }
        turtle_found = true;
    }
    void pose_subscriber_callback(turtlesim::msg::Pose msg)
    {
        my_pose_ = msg;
    }

    void control_loop_timer()
    {
        if (my_pose_.x == -10.0 || !turtle_found)
            return;

        auto target = std::make_shared<turtlesim::msg::Pose>();
        target->set__x(alive_turtle_.x);
        target->set__y(alive_turtle_.y);

        float x_distance = target->x - my_pose_.x;
        float y_distance = target->y - my_pose_.y;

        float total_distance = sqrt(pow(x_distance, 2) + pow(y_distance, 2));
        auto cmd = geometry_msgs::msg::Twist();

        if (total_distance > 0.2) // Turtle is far from target
        {

            float goal_theta = atan2(y_distance, x_distance);

            float theta_difference = goal_theta - my_pose_.theta;

            if (theta_difference > M_PI)
                theta_difference -= 2 * M_PI;

            else if (theta_difference < -M_PI)
                theta_difference += 2 * M_PI;

            cmd.linear.x = 4 * total_distance;

            // angular velocity
            cmd.angular.z = 8 * theta_difference;
        }
        else // Turtle is close enough to the target so we stop moving and kill that turtle
        {

            helper_send_kill_turtle_request(alive_turtle_.name);

            cmd.linear.x = 0.0;
            cmd.angular.y = 0.0;
            // Reset boolean as current turtle is caught
        }
        cmd_vel_publisher_->publish(cmd);
    }

    void helper_send_kill_turtle_request(std::string name)
    {

        auto request_2 = std::make_shared<custom_interfaces::srv::TurtleKill::Request>();
        request_2->set__name(name);
        remove_turtle_from_list_client_->async_send_request(request_2, std::bind(&TurtleControllerNode::helper_remove_turtle_callback, this, std::placeholders::_1)); // Remove turtle from record

        auto request_3 = std::make_shared<std_srvs::srv::Empty::Request>();
    }

    void helper_remove_turtle_callback(rclcpp::Client<custom_interfaces::srv::TurtleKill>::SharedFuture future)
    {
        try
        {
            auto response = future.get();
            turtle_found = false;
            if (response->success == true)
                RCLCPP_INFO(this->get_logger(), "Removed turtle %s from list!", alive_turtle_.name.c_str());
            else
                RCLCPP_WARN(this->get_logger(), "Unable to remove turtle %s from list!", alive_turtle_.name.c_str());
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
    rclcpp::Subscription<custom_interfaces::msg::TurtleArray>::SharedPtr alive_tutles_subscriber_;
    rclcpp::Client<custom_interfaces::srv::TurtleKill>::SharedPtr remove_turtle_from_list_client_;
    custom_interfaces::msg::Turtle alive_turtle_;
    bool turtle_found;
    bool kill_closest_turtle;
    rclcpp::TimerBase::SharedPtr timer_;
    turtlesim::msg::Pose my_pose_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    // auto exec = rclcpp::executors::SingleThreadedExecutor();
    auto node = std::make_shared<TurtleControllerNode>();
    // exec.add_node(std::make_shared<TurtleControllerNode>());
    // exec.spin();
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
