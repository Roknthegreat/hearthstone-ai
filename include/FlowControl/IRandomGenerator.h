#pragma once

namespace FlowControl
{
	class IRandomGenerator
	{
	public:
		IRandomGenerator() {}
		IRandomGenerator(IRandomGenerator const&) = delete;
		IRandomGenerator & operator=(IRandomGenerator const&) = delete;

		virtual int Get(int exclusive_max) = 0;
		virtual int Get(int min, int max) = 0;
	};
}