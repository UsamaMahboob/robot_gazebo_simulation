#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "custom_interfaces/msg/turtle.hpp"
#include "custom_interfaces/msg/turtle_array.hpp"
#include "custom_interfaces/srv/turtle_kill.hpp"

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

        turtle_kill_server_ = this->create_service<custom_interfaces::srv::TurtleKill>("/remove_turtle",
                             std::bind(&TurtleSpawnerNode::kill_turtle_service_response_callback, this, std::placeholders::_1, std::placeholders::_2));

        spawn_publisher_ = this->create_publisher<custom_interfaces::msg::TurtleArray>("/turtle_spawn_locations",10);
        timer_ = this->create_wall_timer(std::chrono::seconds(2), std::bind(&TurtleSpawnerNode::spawner_callback, this));
    }

private:

    void kill_turtle_service_response_callback(custom_interfaces::srv::TurtleKill::Request::SharedPtr request,
                             custom_interfaces::srv::TurtleKill::Response::SharedPtr response)
    {
        if(map_of_turtles_.find(request->name) == map_of_turtles_.end())
        {
            response->set__success(false);
            RCLCPP_WARN(this->get_logger(), "Unable to find turtle to remove from list!");
        }
        else
        {
            alive_turtles_.erase(map_of_turtles_[request->name]);
            publish_alive_turtles();
            response->set__success(true);
            RCLCPP_INFO(this->get_logger(), "Removed turtle from list.");
        }
    }
    void spawner_callback()
    {
        turtle_counter++;
        auto request = std::make_shared<turtlesim::srv::Spawn::Request>();

        float random_value_x = (float)rand() / (float)(RAND_MAX / 9);
        float random_value_y = (float)rand() / (float)(RAND_MAX / 9);

        request->set__x(random_value_x);
        request->set__y(random_value_y);

        float theta = rand() % (int)(2 * M_PI);
        request->set__theta(theta);

        std::string name = name_prefix + std::to_string(turtle_counter);

        request->set__name(name);

        auto new_turtle = custom_interfaces::msg::Turtle();
        new_turtle.set__name(name);
        new_turtle.set__theta(theta);
        new_turtle.set__x(random_value_x);
        new_turtle.set__y(random_value_y);

        alive_turtles_.emplace_back(new_turtle);
        map_of_turtles_[new_turtle.name] = alive_turtles_.end() - 1;

        auto future = service_client_-> async_send_request(request, std::bind(&TurtleSpawnerNode::response_recieved_callback, this, std::placeholders::_1)) ;
    }

    void response_recieved_callback(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        try
        {
            auto response = future.get();
            RCLCPP_INFO(this->get_logger(), "Spawned turtle %s \n", response->name.c_str());

            publish_alive_turtles();
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR(this->get_logger(), "Spawner service call failed!");
            std::cerr << e.what() << '\n';
        }
    }

    void publish_alive_turtles()
    {
        auto msg = custom_interfaces::msg::TurtleArray();
        msg.set__turtles(alive_turtles_);
        spawn_publisher_->publish(msg);
    }
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr service_client_;
    rclcpp::Publisher<custom_interfaces::msg::TurtleArray>::SharedPtr spawn_publisher_;
    std::vector<custom_interfaces::msg::Turtle> alive_turtles_;
    rclcpp::Service<custom_interfaces::srv::TurtleKill>::SharedPtr turtle_kill_server_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::unordered_map<std::string, std::vector<custom_interfaces::msg::Turtle>::iterator> map_of_turtles_;
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