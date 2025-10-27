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

#include "grid.hpp"

// A backbite function. It takes a path and modifies it in place.
// It randomly selects one of the two ends of the path,
// then randomly selects a neighbor cell of that end which is already in the path.
// It then "bites" the path at that neighbor, reversing the segment of the path
// between the selected end and the neighbor.
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
            // Exclude the immediate neighbor in path (so we don’t bite our own edge)
            if (end_index == 0) {
                if (path[1] != std::make_pair(ny, nx))
                    neighbors.push_back({ny, nx});
            } else {
                if (path[path.size() - 2] != std::make_pair(ny, nx))
                    neighbors.push_back({ny, nx});
            }
        }
    }
    if (neighbors.empty()) return;

    std::uniform_int_distribution<> neigh_dis(0, neighbors.size() - 1);
    auto [bite_y, bite_x] = neighbors[neigh_dis(rng)];

    // Find bite position in the path
    auto it = std::find(path.begin(), path.end(), std::make_pair(bite_y, bite_x));
    if (it == path.end()) return;

    if (end_index == 0) {
        // Reverse from start up to *but not including* the bite
        std::reverse(path.begin(), it);
    } else {
        // Reverse from *after* the bite to the end
        std::reverse(it + 1, path.end());
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
                                                      int subgrid_n){

    // Full path assembly
    std::vector<std::pair<int, int>> full_path;

    for (size_t i=0; i<lowres_path.size(); i++){
        int lowres_i = lowres_path[i].first;
        int lowres_j = lowres_path[i].second;
        Grid subgrid(subgrid_m, subgrid_n, {lowres_i*subgrid_m, lowres_j*subgrid_n});

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

        // Generate path in subgrid
        std::vector<std::pair<int, int>> subgrid_path;
        while(subgrid_path.empty()){
            // Find a starting position at the edge of the subgrid
            int start_y, start_x;
            if (i > 0){
                int lowres_i_prev = lowres_path[i-1].first;
                int lowres_j_prev = lowres_path[i-1].second;
                // Get the last random position from the previous subgrid
                std::pair<int, int> last_pos = full_path.back();
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
                for (int y = lowres_i*subgrid_m; y < lowres_i*subgrid_m + subgrid_m; ++y){
                    for (int x = lowres_j*subgrid_n; x < lowres_j*subgrid_n + subgrid_n; ++x){
                        possible_starts.push_back({y, x});
                    }
                }
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, possible_starts.size() - 1);
                int rand_index = dis(gen);
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

// int main(void){
//     int lowres_m, lowres_n, subgrid_m, subgrid_n;
//     std::vector<std::pair<int,int>> path_swap, path;  

//     std::vector<std::pair<int,int>> mn = {
//         {2,2},
//         {2,2},
//         {2,2},
//         {2,2},
//         {2,2},
//         {2,2},
//         {2,2}
//     };    

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
//                                           subgrid_n);

    
//         // One level more
//         path = path_swap;
//     }

//     // Compute overall dimensions
//     int overall_m = 1, overall_n = 1;
//     for (const auto& dim : mn){
//         overall_m *= dim.first;
//         overall_n *= dim.second;
//     }

//     // // print grid view before backbite, that is a matrix with path steps
//     // Grid overall_grid(overall_m, overall_n, {0,0}, path);
//     // overall_grid.print_path(&path);

//     // Perform backbite

//     for (int i=0; i<overall_m*overall_n; i++){
//         std::mt19937 rng(std::random_device{}());
//         backbite(path, overall_m, overall_n, rng);
//     }

//     // // print grid view after backbite
//     // std::cout << "\nAfter backbite:\n";
//     // overall_grid.print_path(&path);

//     std::cout << generate_tikz(overall_m, overall_n, &path,
//                                false, false, false,
//                                1, 3) << "\n";
//     return 0;

// }

struct pair_hash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const noexcept {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

int main(){
    std::unordered_map<
        std::pair<
            std::pair<int,int>, std::pair<int,int>
        >, int, pair_hash> edge_count;
    
    int trials = 10000;
    int grid_m = 6;
    int grid_n = 6;

}