/*
Copyright (C) 2018 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <iostream>
#include <cstdint>
#include <functional>
#include <string>
#include "DependencyMap.h"
#include "gtest/gtest.h"

using namespace ::testing;
namespace AeonGames
{
    TEST ( DependencyMap, DependencyMapSizeTInsert )
    {
        std::vector<size_t> result{11, 12, 13, 10, 2, 4, 5, 3, 1, 7, 9, 8, 6};
        DependencyMap<size_t, size_t> dependency_map;
        dependency_map.Reserve ( 15 );
        dependency_map.Insert ( {6, {7}, 6} );
        dependency_map.Insert ( {1, {2, 3}, 1} );
        dependency_map.Insert ( {3, {4, 5}, 3} );
        dependency_map.Insert ( {5, {4}, 5} );
        dependency_map.Insert ( {9, {7}, 9} );
        dependency_map.Insert ( {2, {}, 2} );
        dependency_map.Insert ( {4, {}, 4} );
        dependency_map.Insert ( {7, {1, 2, 3, 4, 5}, 7} );
        dependency_map.Insert ( {8, {9}, 8} );
        dependency_map.Insert ( {10, {13}, 10} );
        dependency_map.Insert ( {11, {}, 11} );
        dependency_map.Insert ( {12, {11}, 12} );
        dependency_map.Insert ( {13, {12}, 13} );
        ASSERT_EQ ( result.size(), dependency_map.Size() );
        ///@todo A better test would be to make sure all dependants lie before their dependencies.
        for ( size_t i = 0; i != dependency_map.Size(); ++i )
        {
            EXPECT_EQ ( dependency_map[i], result[i] );
        }
    }

    TEST ( DependencyMap, DependencyMapStringConstruct )
    {
        std::vector<size_t> result{2, 4, 5, 3, 1, 7, 9, 8, 6};
        DependencyMap<std::string, size_t> dependency_map
        {
            {
                "six", {"seven"}, 6
            },
            {
                "one", {"two", "three"}, 1
            },
            {
                "three", {"four", "five"}, 3
            },
            {
                "five", {"four"}, 5
            } ,
            {
                "nine", {"seven"}, 9
            } ,
            {
                "two", {}, 2
            } ,
            {
                "four", {}, 4
            } ,
            {
                "seven", {"one", "two", "three", "four", "five"}, 7
            } ,
            {
                "eight", {"nine"}, 8
            }
        };
        ASSERT_EQ ( result.size(), dependency_map.Size() );
        ///@todo A better test would be to make sure all dependants lie before their dependencies.
        for ( size_t i = 0; i != dependency_map.Size(); ++i )
        {
            EXPECT_EQ ( dependency_map[i], result[i] );
        }
    }
}
