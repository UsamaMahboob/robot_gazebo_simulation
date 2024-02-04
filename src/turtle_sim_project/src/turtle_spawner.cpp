#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"

class TurtleSpawnerNode : public rclcpp::Node
{
public:
    TurtleSpawnerNode() : Node("turtle_spawner_node")
    {
        service_client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");
        while (!service_client_->wait_for_service(std::chrono::seconds(1)))
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for server to be up..");
        }
        turtle_counter = 0;
        timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&TurtleSpawnerNode::spawner_callback, this));
    }

private:
    void spawner_callback()
    {
        turtle_counter++;
        auto request = std::make_shared<turtlesim::srv::Spawn::Request>();

        float random_value_x = (float)rand()/(float)(RAND_MAX/9);
        float random_value_y = (float)rand()/(float)(RAND_MAX/9);

        request->set__x(random_value_x);
        request->set__y(random_value_y);

        request->set__theta(rand() % (int)(2 * M_PI));
        request->set__name(name_prefix + std::to_string(turtle_counter));

        auto future = service_client_-> async_send_request(request, std::bind(&TurtleSpawnerNode::response_recieved_callback, this, std::placeholders::_1)) ;
    }

    void response_recieved_callback(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        try
        {
            auto response = future.get();
            RCLCPP_INFO(this->get_logger(), "Spawned turtle %s \n", response->name.c_str());
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "Spawner service call failed!");
            std::cerr << e.what() << '\n';
        }
    }
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr service_client_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::string name_prefix = "my_turtle";
    int turtle_counter;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    TurtleSpawnerNode::SharedPtr node = std::make_shared<TurtleSpawnerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}