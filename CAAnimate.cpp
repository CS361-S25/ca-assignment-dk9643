#include "emp/web/Animate.hpp"      // For animation frame updates
#include "emp/web/web.hpp"          // For rendering and UI in the browser

// Create a Document object to hook into the HTML element with ID "target"
emp::web::Document doc{"target"};

// Animator class that inherits from emp's Animate system
class CAAnimator : public emp::web::Animate {

// === Grid configuration ===
const int num_h_boxes = 10;         // Number of cells vertically
const int num_w_boxes = 15;         // Number of cells horizontally
const double RECT_SIDE = 25;        // Size of each cell (square)
const double width{num_w_boxes * RECT_SIDE};   // Canvas width in pixels
const double height{num_h_boxes * RECT_SIDE};  // Canvas height in pixels

// === Cellular automaton state ===
// 2D grid storing float values in [0,1] for continuous CA
std::vector<std::vector<float> > cells;        // Current state
std::vector<std::vector<float> > nextCells;    // Next state after update

// === Drawing surface ===
// Canvas for rendering the CA to the browser
emp::web::Canvas canvas{width, height, "canvas"};

public:

    // Constructor: sets up canvas, buttons, and initial CA state
    CAAnimator() {
        // Attach canvas and control buttons to the web document
        doc << canvas;
        doc << GetToggleButton("Toggle");       // Start/stop animation
        doc << GetStepButton("Step");           // Advance by one frame

        // Initialize grid to all zero values
        cells.resize(num_w_boxes, std::vector<float>(num_h_boxes, 0.0f));
        nextCells = cells;

        // Seed initial cells with full intensity (1.0)
        for (int i = 0; i < 5; i++) {
            int cx = rand() % num_w_boxes;
            int cy = rand() % num_h_boxes;
            cells[cx][cy] = 1.0f;               // Set random cells to alive
        }
    }

    // This method is called on every animation frame
    void DoFrame() override {
        // Clear the canvas before redrawing
        canvas.Clear();

        // === Render current CA state ===
        for (int x = 0; x < num_w_boxes; x++){
            for (int y = 0; y < num_h_boxes; y++) {

                // Convert cell value to grayscale
                std::stringstream color;
                float val = cells[x][y];
                int gray = static_cast<int>((1.0 - val) * 255);
                color << "rgb(" << gray << "," << gray << "," << gray << ")";

                // Draw rectangle at (x,y) with grayscale fill
                canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE, color.str(), "black");
            }
        }

        // === Update CA state ===
        for (int x = 0; x < num_w_boxes; x++) {
            for (int y = 0; y < num_h_boxes; y++) {

                // Compute average of all 3x3 neighbors, including self
                float sum = 0.0;
                int count = 0;

                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        // Toroidal wrap-around on edges
                        int nx = (x + dx + num_w_boxes) % num_w_boxes;
                        int ny = (y + dy + num_h_boxes) % num_h_boxes;
                        sum += cells[nx][ny];
                        count++;
                    }
                }

                float average = sum / count;    // Neighborhood mean value

                // Apply SmoothLife-style sigmoid rule
                float current = cells[x][y];
                float birth = 1.0 / (1.0 + exp(-20 * (average - 0.3)));
                float death = 1.0 / (1.0 + exp(-20 * (average - 0.4)));
                float s = 1.0 / (1.0 + exp(-20 * (current - birth) / (death - birth + 1e-6)));

                // Update cell value with time step (dt = 0.1)
                float next = current + 0.1f * (2.0f * s - 1.0f);

                // Clamp values to [0, 1] range
                if (next < 0.0f) next = 0.0f;
                if (next > 1.0f) next = 1.0f;

                // Store in next state buffer
                nextCells[x][y] = next;
            }
        }

        // === Swap buffers: next becomes current ===
        std::swap(cells, nextCells);
    }

};

// Create animator instance globally
CAAnimator animator;

int main() {
    // Start with a single animation step
    animator.Step();
}
