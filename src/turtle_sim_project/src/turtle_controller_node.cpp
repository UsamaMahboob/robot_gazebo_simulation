#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "turtlesim/srv/kill.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "custom_interfaces/msg/turtle_array.hpp"
#include "custom_interfaces/msg/turtle.hpp"
#include "custom_interfaces/srv/turtle_kill.hpp"
#include <chrono>
#include <math.h>


class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller_node")
    {
        my_pose_.set__x(-10.0); //This means these variables do not have any useful values as -10 is out of bounds so they will serve as a check to check initialization
        turtle_found = false;

        pose_subscriber_ = this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose", 10, 
                            std::bind(&TurtleControllerNode::pose_subscriber_callback,this,std::placeholders::_1));

        alive_tutles_subscriber_ = this->create_subscription<custom_interfaces::msg::TurtleArray>("/turtle_spawn_locations", 10, 
                            std::bind(&TurtleControllerNode::alive_turtles_subscriber_callback, this, std::placeholders::_1));

        turtle_killer_service_client_ = this->create_client<turtlesim::srv::Kill>("/kill");

        while(!turtle_killer_service_client_->wait_for_service(std::chrono::seconds(10)))
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for server to be up..");
        }

        cmd_vel_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel",10);

        timer_ = this->create_wall_timer(std::chrono::milliseconds(10),
                            std::bind(&TurtleControllerNode::control_loop_timer,this));

        RCLCPP_INFO(this->get_logger(), "Turtle Controller node has been started.");
    }
private:

    void alive_turtles_subscriber_callback(custom_interfaces::msg::TurtleArray msg)
    {
        if(msg.turtles.empty()) //No turtle to catch
        {
            turtle_found = false;
            return;
        }
        alive_turtle_ = msg.turtles[0];
        turtle_found = true;
    }
    void pose_subscriber_callback(turtlesim::msg::Pose msg)
    {
        my_pose_ = msg;
    }

    void control_loop_timer()
    {
        if(my_pose_.x == -10.0 || !turtle_found)
            return;
        
        auto target = std::make_shared<turtlesim::msg::Pose>();
        target->set__x(alive_turtle_.x);
        target->set__y(alive_turtle_.y);

        float x_distance = target->x - my_pose_.x;
        float y_distance = target->y - my_pose_.y;

        float total_distance = sqrt(pow(x_distance,2) + pow(y_distance,2));
        auto cmd = geometry_msgs::msg::Twist();

        if(total_distance > 0.5)                                  // Turtle is far from target
        {                         
            
            float goal_theta = atan2(y_distance,x_distance);
        
            float theta_difference = goal_theta - my_pose_.theta;

            if(theta_difference > M_PI)
                theta_difference -= 2*M_PI;

            else if(theta_difference < -M_PI)
                theta_difference += 2*M_PI;

            cmd.linear.x = 2*total_distance;
            
            //angular velocity
            cmd.angular.z = 6*theta_difference;
        }
        else                                                      //Turtle is close enough to the target so we stop moving and kill that turtle
        {
            turtle_found = false;                                 //Reset boolean as current turtle is caught

            send_kill_turtle_request(alive_turtle_.name);

            cmd.linear.x = 0.0;
            cmd.angular.y = 0.0;
        }
        cmd_vel_publisher_->publish(cmd);

    }

    void send_kill_turtle_request(std::string name)
    {
        auto request = std::make_shared<turtlesim::srv::Kill::Request>();
        request->set__name(name);
        turtle_killer_service_client_->async_send_request(request, std::bind(&TurtleControllerNode::recieve_kill_service_response, this, std::placeholders::_1));
    }

    void recieve_kill_service_response(rclcpp::Client<turtlesim::srv::Kill>::SharedFuture future)
    {
        try
        {
            future.get();
            RCLCPP_INFO(this->get_logger(), "Turtle name = %s Killed!", alive_turtle_.name.c_str());
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_subscriber_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
    rclcpp::Client<turtlesim::srv::Kill>::SharedPtr turtle_killer_service_client_;
    rclcpp::Subscription<custom_interfaces::msg::TurtleArray>::SharedPtr alive_tutles_subscriber_; 
    rclcpp::Client<custom_interfaces::srv::TurtleKill>::SharedPtr kill_turtle_client_ //Make client to call kill turtle service
    custom_interfaces::msg::Turtle alive_turtle_;
    bool turtle_found;
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
