import Caldera;
import std;

using namespace std::literals;

auto main() -> int
{
  auto&& engine = caldera::Engine::getInstance(1920U, 1080U, "Caldera");
	engine.run();
}
