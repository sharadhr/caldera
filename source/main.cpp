import Caldera;
import std;

using namespace std::literals;

auto main() -> int
{
  auto engine = caldera::Engine{1920, 1080, "caldera"sv};
	engine.run();
}
