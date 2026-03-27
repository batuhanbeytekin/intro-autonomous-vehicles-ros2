#include <gazebo/gazebo.hh>
#include <iostream>

namespace gazebo
{
  class WorldPluginMyRobot : public WorldPlugin
  {
  public:
    WorldPluginMyRobot()
    {
      std::cout << "Hello World!" << std::endl;
    }

    void Load(physics::WorldPtr /*_world*/, sdf::ElementPtr /*_sdf*/) override
    {
      std::cout << "Hello World plugin loaded successfully." << std::endl;
    }
  };

  GZ_REGISTER_WORLD_PLUGIN(WorldPluginMyRobot)
}