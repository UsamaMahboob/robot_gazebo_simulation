// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_interfaces:srv/TurtleKill.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_INTERFACES__SRV__DETAIL__TURTLE_KILL__BUILDER_HPP_
#define CUSTOM_INTERFACES__SRV__DETAIL__TURTLE_KILL__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_interfaces/srv/detail/turtle_kill__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_TurtleKill_Request_name
{
public:
  Init_TurtleKill_Request_name()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::srv::TurtleKill_Request name(::custom_interfaces::srv::TurtleKill_Request::_name_type arg)
  {
    msg_.name = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::TurtleKill_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::TurtleKill_Request>()
{
  return custom_interfaces::srv::builder::Init_TurtleKill_Request_name();
}

}  // namespace custom_interfaces


namespace custom_interfaces
{

namespace srv
{

namespace builder
{

class Init_TurtleKill_Response_success
{
public:
  Init_TurtleKill_Response_success()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_interfaces::srv::TurtleKill_Response success(::custom_interfaces::srv::TurtleKill_Response::_success_type arg)
  {
    msg_.success = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_interfaces::srv::TurtleKill_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_interfaces::srv::TurtleKill_Response>()
{
  return custom_interfaces::srv::builder::Init_TurtleKill_Response_success();
}

}  // namespace custom_interfaces

#endif  // CUSTOM_INTERFACES__SRV__DETAIL__TURTLE_KILL__BUILDER_HPP_
