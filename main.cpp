#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <cassert>
#include <unordered_map>
#include <fstream>


#include "grid.hpp"

// A backbite function. It takes a path and modifies it in place.
// It randomly selects one of the two ends of the path,
// then randomly selects a neighbor cell of that end which is already in the path.
// It then "bites" the path at that neighbor, reversing the segment of the path
// between the selected end and the neighbor. It takes a prohibited cell that, after bitting,
// cannot be selected as a bite point. The prohibited cell is updated with the chosen bite point.
void backbite(std::vector<std::pair<int,int>>& path,
              int height,
              int width,
              std::mt19937& rng){
    std::uniform_int_distribution<> end_dis(0, 1);
    int end_index = end_dis(rng) == 0 ? 0 : path.size() - 1;
    auto [end_y, end_x] = path[end_index];

    // Collect neighbor cells within bounds
    std::vector<std::pair<int,int>> neighbors;
    for (const auto& [dy, dx] : directions) {
        int ny = end_y + dy;
        int nx = end_x + dx;
        if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
            neighbors.emplace_back(ny, nx);
        }
    } 
    
    if (neighbors.empty()) return;

    std::uniform_int_distribution<> neigh_dis(0, neighbors.size() - 1);
    auto [bite_y, bite_x] = neighbors[neigh_dis(rng)];

    // Find bite position in the path
    auto it = std::find(path.begin(), path.end(), std::make_pair(bite_y, bite_x));
    if (it == path.end()) return;

    // If biting from the front: reverse beginning..before(it)
    if (end_index == 0) {
        // Reverse from start up to *but not including* the bite
        // std::reverse(path.begin(), it);
        if (it != path.begin()){   // Avoid reversing a 1-element range
            std::reverse(path.begin(), it);
        }

    // If biting from the back: reverse after(it)..end
    }else{
        // Reverse from *after* the bite to the end
        // std::reverse(it + 1, path.end());
        if (it + 1 != path.end()) { // Avoid reversing a 1-element range
            std::reverse(it + 1, path.end());
        }
    }
}


// A function that takes a (low resolution) path, its (low resolution) grid dimensions,
// and a subgrid size. It should transverse the low resolution path generating a random
// high resolution path inside each subgrid, connecting to the next subgrid according to the
// direction of the low resolution path. The result is a grid of size (low_res_m * subgrid_m, low_res_n * subgrid_n)
// and a path that visits all its cells exactly once.
std::vector<std::pair<int,int>> generate_path_pyramid(const std::vector<std::pair<int,int>>& lowres_path,
                                                      int lowres_m,
                                                      int lowres_n,
                                                      int subgrid_m,
                                                      int subgrid_n,
                                                      std::mt19937& rng){

    // Full and subgrid paths
    std::vector<std::pair<int, int>> full_path;
    std::vector<std::pair<int, int>> subgrid_path;

    Grid subgrid(subgrid_m, subgrid_n, {0,0});
    
    for (size_t i=0; i<lowres_path.size(); i++){
        int lowres_i = lowres_path[i].first;
        int lowres_j = lowres_path[i].second;
        subgrid.bias = {lowres_i*subgrid_m, lowres_j*subgrid_n};

        // Determine end side for subgrid
        std::string end_side = "any";
        if (i < lowres_path.size() - 1){
            int lowres_i_next = lowres_path[i+1].first;
            int lowres_j_next = lowres_path[i+1].second;
            if (lowres_i_next == lowres_i + 1){
                end_side = "bottom";
            }else if (lowres_i_next == lowres_i - 1){
                end_side = "top";
            }else if (lowres_j_next == lowres_j + 1){
                end_side = "right";
            }else if (lowres_j_next == lowres_j - 1){
                end_side = "left";
            }
        }else{
            end_side = "any";
        }   

        subgrid_path.clear();

        // Generate path in subgrid
        while(subgrid_path.empty()){
            // Find a starting position at the edge of the subgrid
            int start_y, start_x;
            if (i > 0){
                int lowres_i_prev = lowres_path[i-1].first;
                int lowres_j_prev = lowres_path[i-1].second;
                // Get the last random position from the previous subgrid
                std::pair<int, int> last_pos = full_path.back();

                // Compute new start 
                start_y = last_pos.first;
                start_x = last_pos.second;
                
                // Increment according to direction
                if (lowres_i_prev < lowres_i){
                    start_y += 1; // move down
                }else if (lowres_i_prev > lowres_i){
                    start_y -= 1; // move up
                }else if (lowres_j_prev < lowres_j){
                    start_x += 1; // move right
                }else if (lowres_j_prev > lowres_j){
                    start_x -= 1; // move left
                }

            }else{
                // First subgrid, can start anywhere
                std::vector<std::pair<int, int>> possible_starts;
                for (int y = lowres_i*subgrid_m; y < lowres_i*subgrid_m + subgrid_m; y++){
                    for (int x = lowres_j*subgrid_n; x < lowres_j*subgrid_n + subgrid_n; x++){
                        possible_starts.push_back({y, x});
                    }
                }

                std::uniform_int_distribution<> dis(0, possible_starts.size() - 1);
                int rand_index = dis(rng);
                start_y = possible_starts[rand_index].first;
                start_x = possible_starts[rand_index].second;
            }

            // Generate path in subgrid
            subgrid_path = subgrid.generate_path(start_y, start_x, end_side);
        }
        
        full_path.insert(full_path.end(), subgrid_path.begin(), subgrid_path.end());
    }

    return full_path;
}

int main(void){
    // Generate a small example showing how to connect 4 paths on 2x2 grid of 3x3 subgrids without using
    // my pyramid function. i want to do it by hand manually
    std::pair<int, int> start = {0,0};
    std::pair<int, int> end;
    std::vector<std::pair<int,int>> path_swap, path;
     
    Grid subgrid(6,6, start);

    // First subgrid (0,0) to (0,1)
    path = subgrid.generate_path(start.first, start.second, "right");

    // Second subgrid (0,1) to (0,2) 
    subgrid.bias = {0*6, 1*6};
    start = path.back();
    path_swap = subgrid.generate_path(start.first, start.second + 1, "right");
    path.insert(path.end(), path_swap.begin(), path_swap.end());

    // Third subgrid (0,2) to (1,2) 
    subgrid.bias = std::make_pair(0*6, 2*6); 
    start = path.back();
    path_swap = subgrid.generate_path(start.first, start.second + 1, "bottom");
    path.insert(path.end(), path_swap.begin(), path_swap.end());

    // Fourth subgrid (1,2) to (1,1) 
    subgrid.bias = std::make_pair(1*6, 2*6); 
    start = path.back();
    path_swap = subgrid.generate_path(start.first + 1, start.second, "left");
    path.insert(path.end(), path_swap.begin(), path_swap.end());

    // Fifth subgrid (1,1) to (1,0) 
    subgrid.bias = std::make_pair(1*6, 1*6);
    start = path.back(); 
    path_swap = subgrid.generate_path(start.first, start.second - 1, "left");
    path.insert(path.end(), path_swap.begin(), path_swap.end());

    // Sixth subgrid (1,0) to (0,0) 
    subgrid.bias = std::make_pair(1*6, 0*6);
    start = path.back(); 
    path_swap = subgrid.generate_path(start.first, start.second - 1, "any");
    path.insert(path.end(), path_swap.begin(), path_swap.end());

    std::string save_status = save_path_to_file(path, "pyramid_path.txt");
    std::cout << save_status << "\n";

    return 0;

}

// -----------------------------------------------

// int main(void){
//     std::mt19937 rng(std::random_device{}());
//     int lowres_m, lowres_n, subgrid_m, subgrid_n;
//     std::vector<std::pair<int,int>> path_swap, path;  

//     std::vector<std::pair<int,int>> mn = {
//         {2,2},
//         {3,3}
//     };    

//     // Compute overall dimensions
//     int overall_m = 1, overall_n = 1;
//     for (const auto& dim : mn){
//         overall_m *= dim.first;
//         overall_n *= dim.second;
//     }
//     int k = overall_m * overall_n;
//     int backbite_steps = 100*k * std::log(k);


//     Grid grid(mn[0].first, mn[0].second, {0,0});
//     path = grid.generate_path(0,0);

//     for (uint8_t level=1; level<mn.size(); level++){
//         lowres_m = mn[level-1].first;
//         lowres_n = mn[level-1].second;
//         subgrid_m = mn[level].first;
//         subgrid_n = mn[level].second;

//         path_swap = generate_path_pyramid(path,
//                                           lowres_m,
//                                           lowres_n,
//                                           subgrid_m,
//                                           subgrid_n,
//                                           rng);

    
//         // One level more
//         path = path_swap;
//     }

//     grid.print_path_coordinates(path);

//     // // print grid view before backbite, that is a matrix with path steps
//     // Grid overall_grid(overall_m, overall_n, {0,0}, path);
//     // overall_grid.print_path(&path);

//     // // Perform backbite
//     // std::mt19937 rng(std::random_device{}());
//     // // std::pair<int,int> prohibited = {-1,-1};
//     // for (int i=0; i<backbite_steps; i++){
//     //     backbite(path, overall_m, overall_n, rng); //, prohibited);
//     // }

//     // // print grid view after backbite
//     // std::cout << "\nAfter backbite:\n";
//     // overall_grid.print_path(&path);

//     // std::cout << generate_tikz(overall_m, overall_n, &path,
//     //                            false, false, false,
//     //                            1, 3) << "\n";

//     std::string save_status = save_path_to_file(path, "pyramid_path.txt");
//     std::cout << save_status << "\n";

//     return 0;

// }


//------------------------------------------

// struct pair_hash {
//     std::size_t operator()(const std::pair<std::pair<int, int>, std::pair<int, int>>& p) const {
//         auto h1 = std::hash<int>{}(p.first.first);
//         auto h2 = std::hash<int>{}(p.first.second);
//         auto h3 = std::hash<int>{}(p.second.first);
//         auto h4 = std::hash<int>{}(p.second.second);
//         return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
//     }
// };

// int main(void){

//     std::unordered_map<
//             std::pair<std::pair<int,int>, 
//                       std::pair<int,int>>, 
//             int, pair_hash> edge_count;

//     int lowres_m, lowres_n, subgrid_m, subgrid_n;
//     std::vector<std::pair<int,int>> path_swap, path; 
    
//     std::vector<std::pair<int,int>> mn = {
//         {5,5},
//         {2,2}
//     };

//     // Compute overall dimensions
//     int overall_m = 1, overall_n = 1;
//     for (const auto& dim : mn){
//         overall_m *= dim.first;
//         overall_n *= dim.second;
//     }
//     int k = overall_m * overall_n;
//     int backbite_steps = 100*k * std::log(k);


//     // Number of trials
//     int trials = overall_m*overall_n*overall_m*overall_n;


//     for (int t=0; t<trials; t++){

//         // Generate path via pyramid
//         Grid grid(mn[0].first, mn[0].second, {0,0});
//         path = grid.generate_path(0,0);
//         for (uint8_t level=1; level<mn.size(); level++){
//             lowres_m = mn[level-1].first;
//             lowres_n = mn[level-1].second;
//             subgrid_m = mn[level].first;
//             subgrid_n = mn[level].second;

//             path_swap = generate_path_pyramid(path,
//                                               lowres_m,
//                                               lowres_n,
//                                               subgrid_m,
//                                               subgrid_n);
//             // One level more
//             path = path_swap;
//         }

//         // // Generate raster path
//         // path.clear();
//         // for (int i=0; i<mn[0].first; i++){
//         //     for (int j=0; j<mn[0].second; j++){
//         //         if (i % 2 == 0){
//         //             path.push_back({i, j});
//         //         }else{
//         //             path.push_back({i, mn[0].second - 1 - j});;
//         //         }
//         //     }
//         // }

//         // // print grid view before backbite, that is a matrix with path steps
//         // Grid overall_grid(overall_m, overall_n, {0,0}, path);
//         // overall_grid.print_path(&path);

//         // Perform backbite
//         std::mt19937 rng(std::random_device{}());
//         for (int i=0; i<backbite_steps; i++){
//             backbite(path, overall_m, overall_n, rng);
//         }

//         // // print grid view after backbite
//         // std::cout << "\nAfter backbite:\n";
//         // overall_grid.print_path(&path);

//         for (size_t i=0; i<path.size()-1; i++){
//             auto p1 = path[i];
//             auto p2 = path[i+1];
//             edge_count[{p1, p2}] += 1;
//             edge_count[{p2, p1}] += 1;
//         }
//     }

//     // After your edge_count loop, add this:
//     std::ofstream outfile("edge_count.csv");
//     outfile << "y1,x1,y2,x2,count\n";
//     for (const auto& [edge, count] : edge_count) {
//         auto [p1, p2] = edge;
//         auto [y1, x1] = p1;
//         auto [y2, x2] = p2;
//         outfile << y1 << "," << x1 << "," << y2 << "," << x2 << "," << count << "\n";
//     }
//     outfile.close();
//     std::cout << "Edge counts exported to edge_count.csv\n";

// }