export module Caldera;

export namespace caldera
{
struct Engine
{
	static auto getInstance() -> Engine&;
	auto run() -> void;
};
} // namespace caldera