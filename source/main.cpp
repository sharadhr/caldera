import std;
import vulkan_hpp;
import Caldera;

auto main() -> int
{
  auto engine = caldera::Engine{1920, 1080, "engine"};
	engine.run();
}