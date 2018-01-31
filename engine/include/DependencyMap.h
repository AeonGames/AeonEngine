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
        /*
        Node Data Contents:
        std::tuple<
            Key,                 //-> Parent Iterator
            size_t,              //-> Child Iterator
            size_t,              //-> Visit Mark
            std::vector<size_t>, //-> Dependencies
            T                    //-> Payload
            >
        */
    public:
        DependencyMap() {}
        ~DependencyMap() {}

        void reserve ( size_t count )
        {
            graph.reserve ( count );
            sorted.reserve ( count );
        }

        void insert ( const std::tuple<Key, std::vector<Key>, T>& item )
        {
            if ( sorted.empty() && graph.empty() )
            {
                graph[std::get<0> ( item )] = {{}, 0, 0, std::get<1> ( item ), std::get<2> ( item ) };
                sorted.emplace_back ( std::get<0> ( item ) );
                return;
            }
            ///@todo find a way not to reset marks.
            for ( auto& i : graph )
            {
                //std::get<0> ( i.second ) = {};
                //std::get<1> ( i.second ) = 0;
                std::get<2> ( i.second ) = 0;
            }
            sorted.clear();
            ///@todo find a way not to insert the item until it is found to be valid.
            graph[std::get<0> ( item )] = {{}, 0, 0, std::get<1> ( item ), std::get<2> ( item ) };
            for ( auto& i : graph )
            {
                if ( std::get<2> ( i.second ) == 0 )
                {
                    Key node = i.first;
                    while ( true )
                    {
                        if ( std::get<1> ( graph[node] ) < std::get<3> ( graph[node] ).size() )
                        {
                            auto next = graph.find ( std::get<3> ( graph[node] ) [std::get<1> ( graph[node] )] );
                            ++std::get<1> ( graph[node] );
                            if ( next != graph.end() && std::get<2> ( ( *next ).second ) != 2 )
                            {
                                std::get<2> ( graph.at ( node ) ) = 1;
                                std::get<0> ( ( *next ).second ) = node;
                                node = ( *next ).first;
                                if ( std::get<2> ( graph.at ( node ) ) == 1 )
                                {
                                    ///@todo If there is no way to insert the new node, leave the map in a usable state.
                                    throw std::runtime_error ( "New node would create a circular dependency." );
                                }
                            }
                        }
                        else
                        {
                            std::cout << node << ", ";
                            std::get<2> ( graph.at ( node ) ) = 2;
                            sorted.emplace_back ( node );
                            std::get<1> ( graph[node] ) = 0; // Reset counter for next traversal.
                            if ( node == i.first )
                            {
                                break;
                            }
                            // Go back to the parent
                            node = std::get<0> ( graph[node] );
                        }
                    }
                }
            }
            std::cout << std::endl;
        }
        void erase ( const Key& key )
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
            return std::get<4> ( graph.at ( sorted[index] ) );
        }
        const std::size_t size() const
        {
            return sorted.size();
        }
    private:
        /**
        @todo Find out if it is posible to remove marks from storage.
        @todo Add User Data field.
        */
        std::unordered_map
        <
        Key,
        std::tuple <
        Key,                              //-> Parent Iterator
        size_t,                           //-> Child Iterator
        size_t,                           //-> Visit Mark
        std::vector<Key, VectorAllocator>, //-> Dependencies
        T >,                              //-> Payload
        Hash,                             //-> Hash function
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
    dv.reserve ( 10 );
    dv.insert ( {6, {7}, []()
    {
        std::cout << 6 << std::endl;
    }
                } );
    dv.insert ( {1, {2, 3}, []()
    {
        std::cout << 1 << std::endl;
    }
                } );
    dv.insert ( {3, {4, 5}, []()
    {
        std::cout << 3 << std::endl;
    }
                } );
    dv.insert ( {5, {4}, []()
    {
        std::cout << 5 << std::endl;
    }
                } );
    dv.insert ( {9, {7}, []()
    {
        std::cout << 9 << std::endl;
    }
                } );
    dv.insert ( {2, {}, []()
    {
        std::cout << 2 << std::endl;
    }
                } );
    dv.insert ( {4, {}, []()
    {
        std::cout << 4 << std::endl;
    }
                } );
    dv.insert ( {7, {1, 2, 3, 4, 5}, []()
    {
        std::cout << 7 << std::endl;
    }
                } );
    dv.insert ( {8, {9}, []()
    {
        std::cout << 8 << std::endl;
    }
                } );
    dv.insert ( {10, {13}, []()
    {
        std::cout << 10 << std::endl;
    }
                } );
    dv.insert ( {11, {}, []()
    {
        std::cout << 11 << std::endl;
    }
                } );
    dv.insert ( {12, {11}, []()
    {
        std::cout << 12 << std::endl;
    }
                } );

    dv.insert ( {13, {12}, []()
    {
        std::cout << 13 << std::endl;
    }
                } );

#else
    AeonGames::DependencyMap<std::string, std::function<void() >> dv;
    dv.reserve ( 10 );
    dv.insert ( {"six", {"seven"}, []()
    {
        std::cout << 6 << std::endl;
    }
                } );
    dv.insert ( {"one", {"two", "three"}, []()
    {
        std::cout << 1 << std::endl;
    }
                } );
    dv.insert ( {"three", {"four", "five"}, []()
    {
        std::cout << 3 << std::endl;
    }
                } );
    dv.insert ( {"five", {"four"}, []()
    {
        std::cout << 5 << std::endl;
    }
                } );
    dv.insert ( {"nine", {"seven"}, []()
    {
        std::cout << 9 << std::endl;
    }
                } );
    dv.insert ( {"two", {}, []()
    {
        std::cout << 2 << std::endl;
    }
                } );
    dv.insert ( {"four", {}, []()
    {
        std::cout << 4 << std::endl;
    }
                } );
    dv.insert ( {"seven", {"one", "two", "three", "four", "five"}, []()
    {
        std::cout << 7 << std::endl;
    }
                } );
    dv.insert ( {"eight", {"nine"}, []()
    {
        std::cout << 8 << std::endl;
    }
                } );
#endif
    for ( std::size_t i = 0; i < dv.size(); ++i )
    {
        dv[i]();
    }
    return 0;
}
