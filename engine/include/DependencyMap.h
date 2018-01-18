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
namespace AeonGames
{
    /**
    @todo Add const only operator []
    @todo Add iterators and begin and end functions.
    @todo Turn into a template.
    */
    class DependencyVector
    {
    public:
        DependencyVector() {}
        ~DependencyVector() {}
        void Visit ( size_t node, std::unordered_map<size_t, std::tuple<size_t, std::vector<size_t>>>& graph, std::vector<size_t>& sorted )
        {
            if ( graph.find ( node ) == graph.end() || std::get<0> ( graph.at ( node ) ) == 2 )
            {
                return;
            }
            else if ( std::get<0> ( graph.at ( node ) ) == 1 )
            {
                ///@todo Throw an exception.
                std::cout << __func__ << " " << __LINE__ << " ERROR " << std::endl;
                return;
            }
            std::get<0> ( graph.at ( node ) ) = 1;
            for ( auto& i : std::get<1> ( graph.at ( node ) ) )
            {
                ///@todo find a way to remove recursion
                Visit ( i, graph, sorted );
            }
            std::get<0> ( graph.at ( node ) ) = 2;
            sorted.emplace_back ( node );
        }
        void reserve ( size_t count )
        {
            graph.reserve ( count );
            sorted.reserve ( count );
        }
        /**@todo emplace should match std::unordered_map::emplace.*/
        void emplace ( const std::tuple<size_t, std::vector<size_t>>& item )
        {
            for ( auto& i : graph )
            {
                std::get<0> ( i.second ) = 0;
            }
            ///@todo use std::unordered_map::emplace
            graph[std::get<0> ( item )] = {0, std::get<1> ( item ) };
            ///@todo find a way not to clear sorted but just insert the new item instead.
            sorted.clear();
            for ( auto& i : graph )
            {
                if ( std::get<0> ( i.second ) == 0 )
                {
                    Visit ( i.first, graph, sorted );
                }
            }
            for ( auto& i : sorted )
            {
                std::cout << i << ", ";
            }
            std::cout << std::endl;
        }
    private:
        /**
        @todo Remove marks from storage.
        @todo Add User Data field.
        */
        std::unordered_map<size_t, std::tuple<size_t, std::vector<size_t>>> graph;
        std::vector<size_t> sorted;
    };
}
int main ( int argc, char **argv )
{
    AeonGames::DependencyVector dv;
    dv.emplace ( {1, {2, 3}} );
    dv.emplace ( {3, {4, 5}} );
    dv.emplace ( {5, {4}} );
    dv.emplace ( {2, {}} );
    dv.emplace ( {4, {}} );
    return 0;
}
