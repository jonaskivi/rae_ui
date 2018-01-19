#include "rae/core/version.hpp"

#ifdef version_catch
#include "rae/core/catch.hpp"

#include "rae/core/Utils.hpp"

#include "rae/core/Log.hpp"

SCENARIO("Utils unittest", "[rae][Utils]")
{
	GIVEN( "parallel_for 8193 array" )
	{
		rae_log("Testing testing...");

		std::vector<int> array;
		int arraySize = 8193;
		for (int i = 0; i < arraySize; ++i)
		{
			array.emplace_back(0);
		}

		int testTimes = 129;
		for (int k = 0; k < testTimes; ++k)
		{
			rae::parallel_for(0, array.size(), [&](int i)
			{
				array[i] = array[i] + 1;
			});
		}

		bool allOnes = true;
		for (int i = 0; i < (int)array.size(); ++i)
		{
			if (array[i] != testTimes)
			{
				allOnes = false;
			}
		}

		REQUIRE(allOnes == true);
	}
}

#endif
