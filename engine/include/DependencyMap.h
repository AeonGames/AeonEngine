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
#ifndef AEONGAMES_DEPENDENCY_MAP_H
#define AEONGAMES_DEPENDENCY_MAP_H
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <tuple>
#include <algorithm>
#include <exception>
#include <limits>
#include <functional>

namespace AeonGames
{
    /**
    @todo Add iterators and begin and end functions.
    @todo Add initializer list constructor.
    */
    template <
        class Key,
        class T,
        class Hash = std::hash<Key>,
        class KeyEqual = std::equal_to<Key>,
        class MapAllocator = std::allocator< std::pair<const Key, T>>,
        class VectorAllocator = std::allocator<Key>
        >
    class DependencyMap
    {
    public:
        DependencyMap() {}
        ~DependencyMap() {}

        void Reserve ( size_t count )
        {
            graph.reserve ( count );
            sorted.reserve ( count );
        }

        void Insert ( const std::tuple<Key, std::vector<Key>, T>& item )
        {
            // If the map is empty just insert the new node.
            if ( sorted.empty() && graph.empty() )
            {
                graph[std::get<0> ( item )] = {{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.emplace_back ( std::get<0> ( item ) );
                return;
            }
            // We'll move all dependencies and the new node to the start of the sorted vector.
            auto insertion_cursor = sorted.begin();
            bool circular_dependency = false;
            // Iterate the new node's children to bring all dependencies to the front of the sorted vector.
            for ( auto& i : std::get<1> ( item ) )
            {
                // Only process the current child if 1-it is already in the map and 2-is to the right of the insertion cursor
                if ( graph.find ( i ) != graph.end() && std::find ( sorted.begin(), insertion_cursor, i ) == insertion_cursor )
                {
                    // Set initial node to the current child.
                    Key node = i;
                    // Set Parent node
                    std::get<0> ( graph[i] ) = std::get<0> ( item );
                    // Non Recursive Depth First Search.
                    while ( node != std::get<0> ( item ) )
                    {
                        if ( std::get<1> ( graph[node] ) < std::get<2> ( graph[node] ).size() )
                        {
                            if ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] == std::get<0> ( item ) )
                            {
                                /*
                                If we get here, the new node would create a circular dependency,
                                so we must NOT insert it, if we break here however,
                                the instance would be left in an unstable state,
                                so record the circular dependency and let the proccess finish.
                                Later we'll just NOT insert the node but throw an exception.
                                */
                                circular_dependency = true;
                            }
                            // We get here if the current node still further children to process
                            auto next = graph.find ( std::get<2> ( graph[node] ) [std::get<1> ( graph[node] )] );
                            ++std::get<1> ( graph[node] );
                            // Skip not (yet) existing nodes and nodes to the left of the insertion cursor.
                            if ( next != graph.end() && std::find ( sorted.begin(), insertion_cursor, ( *next ).first ) == insertion_cursor )
                            {
                                std::get<0> ( ( *next ).second ) = node;
                                node = ( *next ).first;
                            }
                        }
                        else
                        {
                            // We get here if the current node has no further children to process
                            sorted.erase ( std::remove ( sorted.begin(), sorted.end(), node ), sorted.end() );
                            sorted.insert ( insertion_cursor++, node );
                            std::get<1> ( graph[node] ) = 0; // Reset counter for next traversal.
                            // Go back to the parent
                            node = std::get<0> ( graph[node] );
                        }
                    }
                }
            }
            if ( !circular_dependency )
            {
                // Insert NEW node
                graph[std::get<0> ( item )] = {{}, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.insert ( insertion_cursor++, std::get<0> ( item ) );
            }
            else
            {
                throw std::runtime_error ( "New node would create a circular dependency." );
            }
        }

        void Erase ( const Key& key )
        {
            graph.erase ( key );
            sorted.erase ( std::remove ( sorted.begin(), sorted.end(), key ), sorted.end() );
        }

        const T& operator[] ( const std::size_t index ) const
        {
            if ( index >= sorted.size() )
            {
                throw std::out_of_range ( "Index out of range." );
            }
            return std::get<3> ( graph.at ( sorted[index] ) );
        }

        const std::size_t Size() const
        {
            return sorted.size();
        }

    private:
        std::unordered_map
        <
        Key,
        std::tuple <
        Key,                               //-> Parent Iterator
        size_t,                            //-> Child Iterator
        std::vector<Key, VectorAllocator>, //-> Dependencies
        T >,                               //-> Payload
        Hash,                              //-> Hash function
        KeyEqual,
        MapAllocator
        > graph;
        std::vector<Key, VectorAllocator> sorted;
    };
}

int main ( int argc, char **argv )
{
#if 1
    AeonGames::DependencyMap<size_t, std::function<void() >> dv;
    dv.Reserve ( 10 );
    dv.Insert ( {6, {7}, []()
    {
        std::cout << 6 << std::endl;
    }
                } );
    dv.Insert ( {1, {2, 3}, []()
    {
        std::cout << 1 << std::endl;
    }
                } );
    dv.Insert ( {3, {4, 5}, []()
    {
        std::cout << 3 << std::endl;
    }
                } );
    dv.Insert ( {5, {4}, []()
    {
        std::cout << 5 << std::endl;
    }
                } );
    dv.Insert ( {9, {7}, []()
    {
        std::cout << 9 << std::endl;
    }
                } );
    dv.Insert ( {2, {}, []()
    {
        std::cout << 2 << std::endl;
    }
                } );
    dv.Insert ( {4, {}, []()
    {
        std::cout << 4 << std::endl;
    }
                } );
    dv.Insert ( {7, {1, 2, 3, 4, 5}, []()
    {
        std::cout << 7 << std::endl;
    }
                } );
    dv.Insert ( {8, {9}, []()
    {
        std::cout << 8 << std::endl;
    }
                } );
    dv.Insert ( {10, {13}, []()
    {
        std::cout << 10 << std::endl;
    }
                } );
    dv.Insert ( {11, {}, []()
    {
        std::cout << 11 << std::endl;
    }
                } );
    dv.Insert ( {12, {11}, []()
    {
        std::cout << 12 << std::endl;
    }
                } );

    try
    {
        dv.Insert ( {13, {12}, []()
        {
            std::cout << 13 << std::endl;
        }
                    } );
    }
    catch ( std::runtime_error& e )
    {
        std::cout << e.what() << std::endl;
    }
#else
    AeonGames::DependencyMap<std::string, std::function<void() >> dv;
    dv.Reserve ( 10 );
    dv.Insert ( {"six", {"seven"}, []()
    {
        std::cout << 6 << std::endl;
    }
                } );
    dv.Insert ( {"one", {"two", "three"}, []()
    {
        std::cout << 1 << std::endl;
    }
                } );
    dv.Insert ( {"three", {"four", "five"}, []()
    {
        std::cout << 3 << std::endl;
    }
                } );
    dv.Insert ( {"five", {"four"}, []()
    {
        std::cout << 5 << std::endl;
    }
                } );
    dv.Insert ( {"nine", {"seven"}, []()
    {
        std::cout << 9 << std::endl;
    }
                } );
    dv.Insert ( {"two", {}, []()
    {
        std::cout << 2 << std::endl;
    }
                } );
    dv.Insert ( {"four", {}, []()
    {
        std::cout << 4 << std::endl;
    }
                } );
    dv.Insert ( {"seven", {"one", "two", "three", "four", "five"}, []()
    {
        std::cout << 7 << std::endl;
    }
                } );
    dv.Insert ( {"eight", {"nine"}, []()
    {
        std::cout << 8 << std::endl;
    }
                } );
#endif
    for ( std::size_t i = 0; i < dv.Size(); ++i )
    {
        dv[i]();
    }
    return 0;
}
#endif
