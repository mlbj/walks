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

class Grid{
private:
    std::vector<std::vector<bool>> _visited;
    std::vector<std::pair<int, int>> _path;

    std::vector<std::pair<int, int>> _target_end;
    bool _has_target_end = false;

    std::mt19937 rng;
public:
    int height, width;
    std::pair<int, int> bias;

    // Path stored in a matrix for visualization
    // std::vector<std::vector<int>> path_


    // Constructors
    Grid(int h, int w, std::pair<int, int> b) : height(h), width(w), bias(b){
        _visited = std::vector<std::vector<bool>>(h, std::vector<bool>(w, false));
        _path = {};
        rng = std::mt19937(std::random_device{}());
    }

    // From path generation
    Grid(int h, int w, std::pair<int, int> b,
         const std::vector<std::pair<int, int>>& visited_cells) 
         : height(h), width(w), bias(b){
        _visited = std::vector<std::vector<bool>>(h, std::vector<bool>(w, false));
        for (const auto& [vy, vx] : visited_cells){
            int local_y = vy - bias.first;
            int local_x = vx - bias.second;
            if (local_y >= 0 && local_y < height &&
                local_x >= 0 && local_x < width){
                _visited[local_y][local_x] = true;
            }

            // Fill grid
            // grid

        
        }
        // Copy from received path
        _path = visited_cells;

        rng = std::mt19937(std::random_device{}());
    }

    // Check if cell is valid
    bool is_valid(int y, int x){ 
        int local_y = y - bias.first;
        int local_x = x - bias.second;
        
        return local_y >= 0 && local_y < height && 
               local_x >= 0 && local_x < width &&
               !_visited[local_y][local_x];
    }

    // Backtracking function
    bool backtrack(int y, int x){
        int local_y = y - bias.first;
        int local_x = x - bias.second;

        // Mark current cell
        _visited[local_y][local_x] = true;
        _path.push_back({y, x});

        // Check if we found a complete path
        if (_path.size() == static_cast<size_t>(width * height)){
            if (_has_target_end){
                // Check if _path satisfies target end constraint
                for (const auto& [ty, tx] : _target_end){
                    if (y == ty && x == tx){
                        return true;
                    }
                }
                _visited[local_y][local_x] = false;
                _path.pop_back();
                return false;
            }else{
                return true;
            }
        }

        // Get current neighbors
        std::vector<std::pair<int, int>> neighbors;
        for (const auto& [dy, dx] : directions){
            int ny = y + dy;
            int nx = x + dx;
            if (is_valid(ny, nx)){
                neighbors.push_back({ny, nx});
            }
        }

        // Try each neighbor in shuffled order
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        for (const auto& [ny, nx] : neighbors) {
            if (backtrack(ny, nx)){
                return true;
            }
        }

        // Backtrack
        _visited[local_y][local_x] = false;
        _path.pop_back();
        return false;
    }

    // Wrapper to find a random path
    std::vector<std::pair<int, int>> generate_path(int start_y, int start_x, 
                                                   const std::vector<std::pair<int, int>>& end_positions = {}){
        _path.clear();
        _visited = std::vector<std::vector<bool>>(height, std::vector<bool>(width, false));

        // Set target end positions
        _target_end = end_positions;
        if (!end_positions.empty()){
            _has_target_end = true;
        }else{
            _has_target_end = false;
        }
        
        // Start backtracking
        if (backtrack(start_y, start_x)){ 
            return _path;
        }
        return {}; 
    }

    // Alternative wrapper to find a random path using a string direction
    std::vector<std::pair<int, int>> generate_path(int start_y, int start_x,
                                                   const std::string& end_side){
        std::vector<std::pair<int, int>> end_positions;

        // Determine valid end positions depending on chosen side
        if (end_side == "top" || end_side == "up") {
            int y = bias.first;  // topmost row
            for (int x = bias.second; x < bias.second + width; ++x) {
                end_positions.push_back({y, x});
            }
        }
        else if (end_side == "bottom" || end_side == "down") {
            int y = bias.first + height - 1;  // bottom row
            for (int x = bias.second; x < bias.second + width; ++x) {
                end_positions.push_back({y, x});
            }
        }
        else if (end_side == "left") {
            int x = bias.second;  // leftmost column
            for (int y = bias.first; y < bias.first + height; ++y) {
                end_positions.push_back({y, x});
            }
        }
        else if (end_side == "right") {
            int x = bias.second + width - 1;  // rightmost column
            for (int y = bias.first; y < bias.first + height; ++y) {
                end_positions.push_back({y, x});
            }
        }
        else if (end_side == "any") {
            return generate_path(start_y, start_x);
        }

        // Delegate to the existing generate_path
        return generate_path(start_y, start_x, end_positions);
    }


    // Visualization methods
    void print_path_coordinates(const std::vector<std::pair<int, int>>* path_ref = nullptr){
        // If no path provided, use internal _path
        if (path_ref == nullptr){
            path_ref = &_path;
        } 

        for (size_t i=0; i<path_ref->size(); i++){
            const auto& [py, px] = (*path_ref)[i];
            std::cout << "(" << py << ", " << px << ") -> ";
        }   
        std::cout << "\n";
    }

    void print_path(const std::vector<std::pair<int, int>>* path_ref = nullptr,
                    int path_offset = 0){
        // If no path provided, use internal _path
        if (path_ref == nullptr){
            path_ref = &_path;
        } 

        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                bool found = false;

                for (size_t i=0; i<path_ref->size(); i++){
                    const auto& [py, px] = (*path_ref)[i];
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


std::string generate_tikz(
    int height,
    int width,
    const std::vector<std::pair<int, int>>* path_ref = nullptr,
    bool show_numbers = false,
    bool show_points = true,
    bool show_grid = false,
    double scale = 1.0,
    double node_size = 4.0)
{
    std::ostringstream tikz;

    tikz << "\\documentclass{standalone}\n";
    tikz << "\\usepackage{graphicx}\n";
    tikz << "\\usepackage{tikz}\n";
    tikz << "\\usetikzlibrary{decorations.pathmorphing, decorations.markings}\n";

    tikz << "\\title{walks}\n";
    tikz << "\\author{mauro brandão}\n";
    tikz << "\\date{November 2025}\n";

    tikz << "\\begin{document}\n";

    tikz << "\\begin{tikzpicture}[scale=" << scale << "]\n";

    // Define TikZ styles
    tikz << "  \\tikzset{\n";
    tikz << "    startnode/.style={circle, fill=green, draw=black, text=white, minimum size=" << node_size << "pt, inner sep=0pt},\n";
    tikz << "    endnode/.style={circle, fill=red, draw=black, text=white, minimum size=" << node_size << "pt, inner sep=0pt},\n";
    tikz << "    midnode/.style={circle, fill=black, draw=black, text=white, minimum size=" << node_size << "pt, inner sep=0pt},\n";
    tikz << "    emptynode/.style={circle, fill=white, draw=gray, thin, minimum size=" << node_size << "pt, inner sep=0pt},\n";
    // tikz << "    myarrow/.style={thick, blue, postaction={decorate},decoration={markings, mark=at position 0.6 with {\\arrow{stealth}}}},\n";
    tikz << "    myarrow/.style={->, >=stealth, thick, blue}\n";
    tikz << "    gridline/.style={gray, very thin, opacity=0.5}\n";
    tikz << "  }\n\n";

    // Optional: draw grid lines
    if (show_grid) {
        tikz << "  % Draw grid lines\n";
        for (int x = 0; x <= width; ++x)
            tikz << "  \\draw[gridline] (" << x << ",0) -- (" << x << "," << height << ");\n";
        for (int y = 0; y <= height; ++y)
            tikz << "  \\draw[gridline] (0," << y << ") -- (" << width << "," << y << ");\n";
        tikz << "\n";
    }

    // Draw all grid points
    if (show_points) {
        tikz << "  % Draw empty grid points\n";
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                tikz << "  \\node[emptynode] at (" << x << "," << (height - 1 - y) << ") {};\n";
        tikz << "\n";
    }

    if (path_ref && !path_ref->empty()) {
        // Draw arrows between consecutive path points
        tikz << "  % Draw arrows between points\n";
        for (size_t i = 0; i < path_ref->size() - 1; ++i) {
            const auto& [y1, x1] = (*path_ref)[i];
            const auto& [y2, x2] = (*path_ref)[i + 1];
            tikz << "  \\draw[myarrow] (" << x1 << "," << (height - 1 - y1)
                 << ") -- (" << x2 << "," << (height - 1 - y2) << ");\n";
        }

        // Draw colored path nodes
        if (show_points) {
            tikz << "\n  % Draw path points\n";
            for (size_t i = 0; i < path_ref->size(); ++i) {
                const auto& [y, x] = (*path_ref)[i];
                std::string style =
                    (i == 0) ? "startnode" :
                    (i == path_ref->size() - 1) ? "endnode" : "midnode";
                tikz << "  \\node[" << style << "] at (" << x << "," << (height - 1 - y) << ") {};\n";
            }
        }

        // Optional step numbers
        if (show_numbers) {
            tikz << "\n  % Draw step numbers\n";
            for (size_t i = 0; i < path_ref->size(); ++i) {
                const auto& [y, x] = (*path_ref)[i];
                tikz << "  \\node[black] at (" << (x + 0.3)
                     << "," << (height - 1 - y + 0.3)
                     << ") {\\tiny{" << i << "}};\n";
            }
        }
    }

    tikz << "\\end{tikzpicture}\n";
    tikz << "\\end{document}\n";

    return tikz.str();
}