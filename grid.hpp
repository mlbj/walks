#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <string>
#include <sstream> 
#include <map>
#include <set>
 
const std::array<std::pair<int, int>, 4> directions = {{
    {0, 1}, {1, 0}, {0, -1}, {-1, 0}
}};

using Point = std::pair<int, int>;
using Points = std::vector<Point>;

class Grid{
private:
    std::mt19937 rng;
public:
    int height, width;
    std::pair<int, int> bias;

	// A constructor
    Grid(int h, int w, Point b) : 
	height(h), width(w), bias(b){
        rng = std::mt19937(std::random_device{}());
    }

	// A helper function that checks if a given coordinate is within bounds 
	bool is_valid(Point p){
		int local_y = p.first - bias.first;
		int local_x = p.second - bias.second;
		return local_y >= 0 && local_y < height &&
			   local_x >= 0 && local_x < width;
	}

    // A Backtracking function
    bool backtrack(Point start,
                   std::vector<std::vector<bool>>& visited,
                   Points& path,
                   const Points& target_end = {}){
        
        int local_y = start.first - bias.first;
        int local_x = start.second - bias.second;

        // Mark current cell
        visited[local_y][local_x] = true;
        path.push_back(std::make_pair(start.first, start.second));

        // Check if we found a complete valid path
        if (path.size() == static_cast<size_t>(width * height)){
            if (!target_end.empty()){
                // Check if path satisfies target end constraint
                for (const auto& [ty, tx] : target_end){
                    if (start.first == ty && start.second == tx){
                        return true;
                    }
                }
                visited[local_y][local_x] = false;
                path.pop_back();
                return false;
            }else{
                return true;
            }
        }

        // Get current neighbors
        Points neighbors;
        for (const auto& [dy, dx] : directions){
            int ny = start.first + dy;
            int nx = start.second + dx;
            Point neighbor = std::make_pair(ny, nx);
            int local_ny = ny - bias.first;
            int local_nx = nx - bias.second;
            if (is_valid(neighbor) && !visited[local_ny][local_nx]){
                neighbors.push_back(neighbor);
            }
        }

        // Try each neighbor in shuffled order
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        for (const auto& [ny, nx] : neighbors){
            Point neighbor = std::make_pair(ny, nx);
            if (backtrack(neighbor, visited, path, target_end)){
                return true;
            }
        }

        // Backtrack
        visited[local_y][local_x] = false;
        path.pop_back();
        return false;
    }

    // A wrapper to the backtracking function
    Points sample_path(Point start, 
                       const Points& target_end = {}){

        std::vector<std::vector<bool>> visited;
        Points path;
        
        path.clear();
        visited = std::vector<std::vector<bool>>(height, std::vector<bool>(width, false));

        // Start backtracking
        if (backtrack(start, visited, path, target_end)){ 
            return path;
        }
        
        return {}; 
    }

    // Alternative wrapper to find a random path using an end_side string
    Points sample_path(Point start,
                       const std::string& end_side){

        Points target_end;

        // Parse end_side to sample target_end positions
        if (end_side == "top"){
            int y = bias.first;  // topmost row
            for (int x=bias.second; x<bias.second+width; x++) {
                target_end.push_back({y, x});
            }
        }
        else if (end_side == "bottom"){
            // bottom row
            int y = bias.first + height - 1;
            for (int x=bias.second; x<bias.second+width; x++){
                target_end.push_back({y, x});
            }
        }
        else if (end_side == "left"){
            // leftmost column
            int x = bias.second;
            for (int y=bias.first; y<bias.first+height; y++){
                target_end.push_back({y, x});
            }
        }
        else if (end_side == "right"){
            // rightmost column
            int x = bias.second + width - 1;
            for (int y=bias.first; y<bias.first+height; y++) {
                target_end.push_back({y, x});
            }
        }
        else if (end_side == "any"){
            return sample_path(start);
        }

        // sample path
        return sample_path(start, target_end);
    }


    // Visualization methods
    void print_path_coordinates(const Points& path){
        for (size_t i=0; i<path.size(); i++){
            const auto& [py, px] = path[i];
            std::cout << "(" << py << ", " << px << ") -> ";
        }   
        std::cout << "\n";
    }

    void print_path(const Points& path,
                    int path_offset = 0){

        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                bool found = false;

                for (size_t i=0; i<path.size(); i++){
                    const auto& [py, px] = path[i];
                    if (py == y + bias.first && px == x + bias.second){
                        std::cout << (i + path_offset < 10 ? " " : "") << i + path_offset << " ";
                        found = true;
                        break;
                    }
                }
                if (!found){
                    std::cout << " .  ";
                }
            }
            std::cout << "\n";
        }   
    }
};

std::string save_path_to_file(const Points& path,
                              const std::string& filename){
    std::ofstream file(filename);
    if (!file.is_open()){
        return "Error: Unable to open file " + filename;
    }
    for (const auto& [y, x] : path){
        file << y << " " << x << "\n";
    }
    file.close();
    return "Path saved to " + filename;;
}