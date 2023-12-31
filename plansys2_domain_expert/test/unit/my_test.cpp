// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <memory>

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "gtest/gtest.h"
#include "plansys2_domain_expert/DomainExpertNode.hpp"
#include "plansys2_domain_expert/DomainExpertClient.hpp"
#include "plansys2_msgs/msg/action.hpp"

#include "lifecycle_msgs/msg/state.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

TEST(domain_expert, lifecycle)
{
  auto test_node = rclcpp::Node::make_shared("get_action_from_string");
  auto domain_node = std::make_shared<plansys2::DomainExpertNode>();
  auto domain_client = std::make_shared<plansys2::DomainExpertClient>();

  std::string pkgpath = ament_index_cpp::get_package_share_directory("plansys2_domain_expert");

  domain_node->set_parameter({"model_file", pkgpath + "/pddl/domain_simple.pddl"});
  rclcpp::executors::MultiThreadedExecutor exe(rclcpp::ExecutorOptions(), 8);

  exe.add_node(domain_node->get_node_base_interface());

  bool finish = false;
  std::thread t([&]() {
      while (!finish) {exe.spin_some();}
    });

  domain_node->trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_CONFIGURE);

  {
    rclcpp::Rate rate(10);
    auto start = test_node->now();
    while ((test_node->now() - start).seconds() < 0.5) {
      rate.sleep();
    }
  }

  ASSERT_EQ(
    domain_node->get_current_state().id(),
    lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE);

  domain_node->trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_ACTIVATE);

  {
    rclcpp::Rate rate(10);
    auto start = test_node->now();
    while ((test_node->now() - start).seconds() < 0.5) {
      rate.sleep();
    }
  }

  ASSERT_EQ(
    domain_node->get_current_state().id(),
    lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE);

  std::vector<std::string> actions = domain_client->getActions();
  for (std::string action : actions)
  {
    std::cout << action << "\n";
  }

  std::string action = "move_person"; 
  std::vector<std::string> params = {"p1", "r1"};
  plansys2_msgs::msg::Action::SharedPtr action_msg = domain_client->getAction(action, params);
  std::cout << action_msg->name << "\n";
  for(auto p : action_msg->parameters){
    std::cout << p.name << "\n";
    std::cout << p.type << "\n";
    for(auto subt : p.sub_types)
      std::cout << subt << "\n";

  }
  

  // {
  //   rclcpp::Rate rate(10);
  //   auto start = test_node->now();
  //   while ((test_node->now() - start).seconds() < 0.5) {
  //     rate.sleep();
  //   }
  // }

  // std::ifstream domain_ifs_p(pkgpath + "/pddl/domain_simple_processed.pddl");
  // std::string domain_str_p((
  //     std::istreambuf_iterator<char>(domain_ifs_p)),
  //   std::istreambuf_iterator<char>());

  // ASSERT_EQ(domain_str, domain_str_p);

  finish = true;
  t.join();
}


int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);

  return RUN_ALL_TESTS();
}
